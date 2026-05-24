const crypto = require('node:crypto');
const jwt = require('jsonwebtoken');
const bcrypt = require('bcryptjs');
const { authenticator } = require('otplib');
const QRCode = require('qrcode');
const { S3Client, PutObjectCommand } = require('@aws-sdk/client-s3');
const {
  JWT_SECRET,
  LOGIN_CHALLENGE_SECRET,
  EMAIL_VERIFY_TTL_MINUTES,
  PASSWORD_RESET_TTL_MINUTES,
  APP_BASE_URL,
  S3_BUCKET,
  AWS_REGION,
} = require('./config');
const { db } = require('./db');

const sentEmails = [];
const s3Client = S3_BUCKET ? new S3Client({ region: AWS_REGION }) : null;

function nowIso() {
  return new Date().toISOString();
}

function hashToken(token) {
  return crypto.createHash('sha256').update(token).digest('hex');
}

function randomToken(size = 32) {
  return crypto.randomBytes(size).toString('hex');
}

function addMinutes(date, minutes) {
  return new Date(date.getTime() + minutes * 60_000).toISOString();
}

function issueAuthToken(user) {
  return jwt.sign(
    { sub: user.id, email: user.email, tokenVersion: user.token_version },
    JWT_SECRET,
    { expiresIn: '24h' },
  );
}

function issueChallengeToken(userId) {
  return jwt.sign({ sub: userId, stage: '2fa' }, LOGIN_CHALLENGE_SECRET, { expiresIn: '5m' });
}

function verifyChallengeToken(token) {
  return jwt.verify(token, LOGIN_CHALLENGE_SECRET);
}

function verifyAuthToken(token) {
  return jwt.verify(token, JWT_SECRET);
}

function createAuditLog(userId, action, metadata = {}) {
  db.prepare('INSERT INTO audit_logs(user_id, action, metadata) VALUES (?, ?, ?)').run(
    userId || null,
    action,
    JSON.stringify(metadata),
  );
}

function sendEmail(to, subject, text) {
  const message = { to, subject, text, sentAt: nowIso() };
  sentEmails.push(message);
  return message;
}

function createVerificationToken(userId) {
  const token = randomToken();
  const tokenHash = hashToken(token);
  const expiresAt = addMinutes(new Date(), EMAIL_VERIFY_TTL_MINUTES);
  db.prepare(
    'INSERT INTO email_verification_tokens(user_id, token_hash, expires_at) VALUES (?, ?, ?)',
  ).run(userId, tokenHash, expiresAt);
  return token;
}

function consumeVerificationToken(token) {
  const tokenHash = hashToken(token);
  const record = db.prepare(
    `SELECT * FROM email_verification_tokens
     WHERE token_hash = ? AND used_at IS NULL AND expires_at > datetime('now')`,
  ).get(tokenHash);

  if (!record) return null;

  const tx = db.transaction(() => {
    db.prepare('UPDATE email_verification_tokens SET used_at = ? WHERE id = ?').run(nowIso(), record.id);
    db.prepare('UPDATE users SET email_verified = 1 WHERE id = ?').run(record.user_id);
  });
  tx();

  return record.user_id;
}

function createPasswordResetToken(userId) {
  const token = randomToken();
  const tokenHash = hashToken(token);
  const expiresAt = addMinutes(new Date(), PASSWORD_RESET_TTL_MINUTES);
  db.prepare(
    'INSERT INTO password_reset_tokens(user_id, token_hash, expires_at) VALUES (?, ?, ?)',
  ).run(userId, tokenHash, expiresAt);
  return token;
}

function resetPasswordByToken(token, newPasswordHash) {
  const tokenHash = hashToken(token);
  const record = db.prepare(
    `SELECT * FROM password_reset_tokens
     WHERE token_hash = ? AND used_at IS NULL AND expires_at > datetime('now')`,
  ).get(tokenHash);

  if (!record) return false;

  const tx = db.transaction(() => {
    db.prepare('UPDATE password_reset_tokens SET used_at = ? WHERE id = ?').run(nowIso(), record.id);
    db.prepare(
      `UPDATE users
       SET password_hash = ?, token_version = token_version + 1
       WHERE id = ?`,
    ).run(newPasswordHash, record.user_id);
    createAuditLog(record.user_id, 'password_reset_completed', { method: 'token' });
  });

  tx();
  return true;
}

function validatePasswordStrength(password) {
  return (
    typeof password === 'string' &&
    password.length >= 8 &&
    /[a-z]/.test(password) &&
    /[A-Z]/.test(password) &&
    /\d/.test(password)
  );
}

function generateRecoveryCodes(userId) {
  const codes = Array.from({ length: 8 }, () => randomToken(4));
  const insert = db.prepare('INSERT INTO two_factor_recovery_codes(user_id, code_hash) VALUES (?, ?)');
  const tx = db.transaction(() => {
    db.prepare('DELETE FROM two_factor_recovery_codes WHERE user_id = ?').run(userId);
    for (const code of codes) {
      insert.run(userId, bcrypt.hashSync(code, 10));
    }
  });
  tx();
  return codes;
}

function consumeRecoveryCode(userId, code) {
  const records = db
    .prepare('SELECT * FROM two_factor_recovery_codes WHERE user_id = ? AND used_at IS NULL')
    .all(userId);

  for (const record of records) {
    if (bcrypt.compareSync(code, record.code_hash)) {
      db.prepare('UPDATE two_factor_recovery_codes SET used_at = ? WHERE id = ?').run(nowIso(), record.id);
      return true;
    }
  }

  return false;
}

async function createTotpSetup(user) {
  const secret = authenticator.generateSecret();
  const label = encodeURIComponent(`Infiniti:${user.email}`);
  const issuer = encodeURIComponent('Infiniti Chess');
  const otpauth = `otpauth://totp/${label}?secret=${secret}&issuer=${issuer}`;
  const qrCodeDataUrl = await QRCode.toDataURL(otpauth);
  db.prepare('UPDATE users SET two_factor_secret_pending = ? WHERE id = ?').run(secret, user.id);
  return { secret, otpauth, qrCodeDataUrl };
}

function verifyTotp(secret, otp) {
  return authenticator.verify({ token: otp, secret });
}

async function uploadToObjectStorage(buffer, key, contentType) {
  if (!s3Client || !S3_BUCKET) {
    return `blackhole://${key}`;
  }

  await s3Client.send(
    new PutObjectCommand({
      Bucket: S3_BUCKET,
      Key: key,
      Body: buffer,
      ContentType: contentType,
    }),
  );

  return `s3://${S3_BUCKET}/${key}`;
}

function sendVerificationEmail(email, token) {
  const link = `${APP_BASE_URL}/verify-email.html?token=${encodeURIComponent(token)}`;
  sendEmail(email, 'Verify your Infiniti account', `Verify your account: ${link}`);
}

function sendPasswordResetEmail(email, token) {
  const link = `${APP_BASE_URL}/reset-password.html?token=${encodeURIComponent(token)}`;
  sendEmail(email, 'Reset your Infiniti password', `Reset your password: ${link}`);
}

module.exports = {
  sentEmails,
  hashToken,
  randomToken,
  issueAuthToken,
  verifyAuthToken,
  issueChallengeToken,
  verifyChallengeToken,
  createAuditLog,
  sendEmail,
  createVerificationToken,
  consumeVerificationToken,
  createPasswordResetToken,
  resetPasswordByToken,
  validatePasswordStrength,
  generateRecoveryCodes,
  consumeRecoveryCode,
  createTotpSetup,
  verifyTotp,
  uploadToObjectStorage,
  sendVerificationEmail,
  sendPasswordResetEmail,
};
