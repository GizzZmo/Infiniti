const path = require('node:path');
const express = require('express');
const bcrypt = require('bcryptjs');
const rateLimit = require('express-rate-limit');
const multer = require('multer');
const { db } = require('./db');
const {
  sentEmails,
  issueAuthToken,
  verifyAuthToken,
  issueChallengeToken,
  verifyChallengeToken,
  createAuditLog,
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
  randomToken,
} = require('./services');

const app = express();
const upload = multer({ storage: multer.memoryStorage(), limits: { fileSize: 6 * 1024 * 1024 } });

const authLimiter = rateLimit({ windowMs: 60_000, max: 20 });
const loginLimiter = rateLimit({ windowMs: 60_000, max: 8 });
const resetLimiter = rateLimit({ windowMs: 60_000, max: 5 });
const otpLimiter = rateLimit({ windowMs: 60_000, max: 8 });

app.use(express.json());
app.use('/api/auth/register', authLimiter);
app.use('/api/auth/login', loginLimiter);
app.use('/api/auth/forgot-password', resetLimiter);
app.use('/api/auth/reset-password', resetLimiter);
app.use('/api/auth/login/2fa', otpLimiter);

function sanitizeUser(user) {
  return {
    id: user.id,
    email: user.email,
    emailVerified: Boolean(user.email_verified),
    twoFactorEnabled: Boolean(user.two_factor_enabled),
    isAdmin: Boolean(user.is_admin),
  };
}

function requireAuth(req, res, next) {
  const authHeader = req.headers.authorization || '';
  let token = null;
  if (authHeader.startsWith('Bearer ')) token = authHeader.slice(7);
  if (authHeader.startsWith('Token ')) token = authHeader.slice(6);
  if (!token) return res.status(401).json({ error: 'Missing bearer token.' });

  try {
    const payload = verifyAuthToken(token);
    const user = db
      .prepare('SELECT * FROM users WHERE id = ? AND token_version = ?')
      .get(payload.sub, payload.tokenVersion);
    if (!user) return res.status(401).json({ error: 'Session invalid.' });
    req.user = user;
    next();
  } catch {
    return res.status(401).json({ error: 'Invalid token.' });
  }
}

function requireEmailVerified(req, res, next) {
  if (!req.user.email_verified) {
    return res.status(403).json({ error: 'Email verification required.' });
  }
  next();
}

function requireAdmin(req, res, next) {
  if (!req.user.is_admin) {
    return res.status(403).json({ error: 'Admin access required.' });
  }
  next();
}

app.post('/api/auth/register', (req, res) => {
  const { email, password } = req.body;
  if (!email || !validatePasswordStrength(password)) {
    return res.status(400).json({ error: 'Invalid email or weak password.' });
  }

  const passwordHash = bcrypt.hashSync(password, 12);
  const userCount = db.prepare('SELECT COUNT(*) AS count FROM users').get().count;
  const isAdmin = userCount === 0 ? 1 : 0;
  try {
    const result = db
      .prepare('INSERT INTO users(email, password_hash, is_admin) VALUES (?, ?, ?)')
      .run(email.toLowerCase(), passwordHash, isAdmin);

    const token = createVerificationToken(result.lastInsertRowid);
    sendVerificationEmail(email, token);

    const user = db.prepare('SELECT * FROM users WHERE id = ?').get(result.lastInsertRowid);
    return res.status(201).json({ user: sanitizeUser(user) });
  } catch {
    return res.status(409).json({ error: 'Email already exists.' });
  }
});

app.post('/api/auth/login', (req, res) => {
  const { email, password } = req.body;
  const user = db.prepare('SELECT * FROM users WHERE email = ?').get((email || '').toLowerCase());
  if (!user || !bcrypt.compareSync(password || '', user.password_hash)) {
    return res.status(401).json({ error: 'Invalid credentials.' });
  }

  if (user.two_factor_enabled) {
    return res.json({
      requires2fa: true,
      challengeToken: issueChallengeToken(user.id),
    });
  }

  return res.json({
    token: issueAuthToken(user),
    user: sanitizeUser(user),
  });
});

app.post('/api/auth/login/2fa', async (req, res) => {
  const { challengeToken, otp, recoveryCode } = req.body;
  if (!challengeToken) return res.status(400).json({ error: 'Missing challenge token.' });

  let payload;
  try {
    payload = verifyChallengeToken(challengeToken);
  } catch {
    return res.status(401).json({ error: 'Invalid or expired challenge.' });
  }

  const user = db.prepare('SELECT * FROM users WHERE id = ?').get(payload.sub);
  if (!user || !user.two_factor_enabled) {
    return res.status(400).json({ error: '2FA is not enabled.' });
  }

  const otpOk = otp ? await verifyTotp(user.two_factor_secret, otp) : false;
  const recoveryOk = !otpOk && recoveryCode ? consumeRecoveryCode(user.id, recoveryCode) : false;

  if (!otpOk && !recoveryOk) {
    return res.status(401).json({ error: 'Invalid OTP or recovery code.' });
  }

  return res.json({ token: issueAuthToken(user), user: sanitizeUser(user) });
});

app.get('/api/auth/verify-email/:token', (req, res) => {
  const userId = consumeVerificationToken(req.params.token);
  if (!userId) return res.status(400).json({ error: 'Invalid or expired verification token.' });

  createAuditLog(userId, 'email_verified');
  return res.json({ success: true });
});

app.post('/api/auth/verify-email/resend', requireAuth, (req, res) => {
  if (req.user.email_verified) return res.json({ success: true, message: 'Already verified.' });

  const token = createVerificationToken(req.user.id);
  sendVerificationEmail(req.user.email, token);
  return res.json({ success: true });
});

app.post('/api/auth/forgot-password', (req, res) => {
  const { email } = req.body;
  const user = db.prepare('SELECT * FROM users WHERE email = ?').get((email || '').toLowerCase());

  if (user) {
    const token = createPasswordResetToken(user.id);
    sendPasswordResetEmail(user.email, token);
    createAuditLog(user.id, 'password_reset_requested');
  }

  return res.json({ success: true });
});

app.post('/api/auth/reset-password/:token', (req, res) => {
  const { password } = req.body;
  if (!validatePasswordStrength(password)) {
    return res.status(400).json({ error: 'Password does not meet complexity rules.' });
  }

  const passwordHash = bcrypt.hashSync(password, 12);
  const ok = resetPasswordByToken(req.params.token, passwordHash);
  if (!ok) return res.status(400).json({ error: 'Invalid or expired reset token.' });

  return res.json({ success: true });
});

app.post('/api/auth/2fa/setup', requireAuth, async (req, res) => {
  const setup = await createTotpSetup(req.user);
  return res.json(setup);
});

app.post('/api/auth/2fa/enable', requireAuth, async (req, res) => {
  const { otp } = req.body;
  const current = db.prepare('SELECT * FROM users WHERE id = ?').get(req.user.id);
  if (!current.two_factor_secret_pending) {
    return res.status(400).json({ error: '2FA setup not initialized.' });
  }

  if (!(await verifyTotp(current.two_factor_secret_pending, otp || ''))) {
    return res.status(400).json({ error: 'Invalid OTP.' });
  }

  db.prepare(
    'UPDATE users SET two_factor_enabled = 1, two_factor_secret = ?, two_factor_secret_pending = NULL WHERE id = ?',
  ).run(current.two_factor_secret_pending, req.user.id);

  const recoveryCodes = generateRecoveryCodes(req.user.id);
  createAuditLog(req.user.id, '2fa_enabled');
  return res.json({ success: true, recoveryCodes });
});

app.post('/api/auth/2fa/disable', requireAuth, async (req, res) => {
  const { otp } = req.body;
  const current = db.prepare('SELECT * FROM users WHERE id = ?').get(req.user.id);
  if (!current.two_factor_enabled) return res.json({ success: true });
  if (!(await verifyTotp(current.two_factor_secret, otp || ''))) {
    return res.status(400).json({ error: 'Invalid OTP.' });
  }

  db.prepare('UPDATE users SET two_factor_enabled = 0, two_factor_secret = NULL WHERE id = ?').run(req.user.id);
  db.prepare('DELETE FROM two_factor_recovery_codes WHERE user_id = ?').run(req.user.id);
  createAuditLog(req.user.id, '2fa_disabled');
  return res.json({ success: true });
});

app.get('/api/auth/me', requireAuth, (req, res) => {
  const user = db.prepare('SELECT * FROM users WHERE id = ?').get(req.user.id);
  res.json({ user: sanitizeUser(user) });
});

app.post('/api/kyc/upload', requireAuth, upload.single('document'), async (req, res) => {
  const documentType = req.body.documentType;
  if (!req.file || !['government_id', 'proof_of_address'].includes(documentType)) {
    return res.status(400).json({ error: 'Valid document type and file are required.' });
  }

  const key = `kyc/${req.user.id}/${Date.now()}-${randomToken(8)}`;
  const storageKey = await uploadToObjectStorage(req.file.buffer, key, req.file.mimetype);

  const result = db
    .prepare('INSERT INTO kyc_submissions(user_id, document_type, storage_key) VALUES (?, ?, ?)')
    .run(req.user.id, documentType, storageKey);

  createAuditLog(req.user.id, 'kyc_uploaded', { submissionId: result.lastInsertRowid, documentType });
  return res.status(201).json({ id: result.lastInsertRowid, status: 'pending' });
});

app.get('/api/kyc/me', requireAuth, (req, res) => {
  const rows = db
    .prepare('SELECT id, document_type, status, review_comment, created_at, updated_at FROM kyc_submissions WHERE user_id = ? ORDER BY id DESC')
    .all(req.user.id);
  res.json({ submissions: rows });
});

app.get('/api/admin/kyc', requireAuth, requireAdmin, (req, res) => {
  const rows = db.prepare(
    `SELECT k.id, k.user_id, u.email, k.document_type, k.storage_key, k.status, k.review_comment, k.created_at
     FROM kyc_submissions k
     JOIN users u ON u.id = k.user_id
     ORDER BY k.id DESC`,
  ).all();

  res.json({ submissions: rows });
});

app.post('/api/admin/kyc/:id/review', requireAuth, requireAdmin, (req, res) => {
  const { status, comment } = req.body;
  if (!['approved', 'rejected'].includes(status)) {
    return res.status(400).json({ error: 'Status must be approved or rejected.' });
  }

  const existing = db.prepare('SELECT * FROM kyc_submissions WHERE id = ?').get(req.params.id);
  if (!existing) return res.status(404).json({ error: 'Submission not found.' });

  db.prepare(
    `UPDATE kyc_submissions
     SET status = ?, review_comment = ?, reviewed_by = ?, updated_at = datetime('now')
     WHERE id = ?`,
  ).run(status, comment || null, req.user.id, req.params.id);

  createAuditLog(existing.user_id, 'kyc_reviewed', { submissionId: existing.id, status, reviewerId: req.user.id });
  return res.json({ success: true });
});

function buildTransactionWhere(userId, query) {
  const clauses = ['user_id = ?'];
  const params = [userId];

  if (query.type) {
    clauses.push('type = ?');
    params.push(query.type);
  }

  if (query.status) {
    clauses.push('status = ?');
    params.push(query.status);
  }

  if (query.startDate) {
    clauses.push('created_at >= ?');
    params.push(query.startDate);
  }

  if (query.endDate) {
    clauses.push('created_at <= ?');
    params.push(query.endDate);
  }

  if (query.minAmount) {
    clauses.push('amount >= ?');
    params.push(Number(query.minAmount));
  }

  if (query.maxAmount) {
    clauses.push('amount <= ?');
    params.push(Number(query.maxAmount));
  }

  return { whereSql: clauses.join(' AND '), params };
}

function ensureSeedTransactions(userId) {
  const count = db.prepare('SELECT COUNT(*) AS count FROM transactions WHERE user_id = ?').get(userId).count;
  if (count > 0) return;

  const insert = db.prepare(
    'INSERT INTO transactions(user_id, type, amount, currency, status, metadata) VALUES (?, ?, ?, ?, ?, ?)',
  );
  insert.run(userId, 'deposit', 100, 'USD', 'completed', JSON.stringify({ method: 'card' }));
  insert.run(userId, 'withdrawal', 25, 'USD', 'pending', JSON.stringify({ method: 'bank' }));
  insert.run(userId, 'entry_fee', 5, 'USD', 'completed', JSON.stringify({ tournament: 'weekly' }));
}

app.get('/api/transactions', requireAuth, requireEmailVerified, (req, res) => {
  ensureSeedTransactions(req.user.id);
  const page = Math.max(1, Number(req.query.page || 1));
  const pageSize = Math.min(100, Math.max(1, Number(req.query.pageSize || 20)));
  const sortBy = ['created_at', 'amount', 'type', 'status'].includes(req.query.sortBy)
    ? req.query.sortBy
    : 'created_at';
  const sortDir = req.query.sortDir === 'asc' ? 'ASC' : 'DESC';

  const { whereSql, params } = buildTransactionWhere(req.user.id, req.query);
  const total = db.prepare(`SELECT COUNT(*) AS count FROM transactions WHERE ${whereSql}`).get(...params).count;
  const rows = db
    .prepare(
      `SELECT id, type, amount, currency, status, metadata, created_at
       FROM transactions WHERE ${whereSql}
       ORDER BY ${sortBy} ${sortDir}
       LIMIT ? OFFSET ?`,
    )
    .all(...params, pageSize, (page - 1) * pageSize)
    .map((row) => ({ ...row, metadata: JSON.parse(row.metadata) }));

  return res.json({
    total,
    page,
    pageSize,
    items: rows,
  });
});

app.get('/api/transactions/export.csv', requireAuth, requireEmailVerified, (req, res) => {
  ensureSeedTransactions(req.user.id);
  const { whereSql, params } = buildTransactionWhere(req.user.id, req.query);
  const rows = db
    .prepare(
      `SELECT id, type, amount, currency, status, metadata, created_at
       FROM transactions WHERE ${whereSql}
       ORDER BY created_at DESC`,
    )
    .all(...params);

  const header = 'id,type,amount,currency,status,metadata,created_at';
  const lines = rows.map((r) => {
    const meta = JSON.stringify(JSON.parse(r.metadata)).replaceAll('"', '""');
    return `${r.id},${r.type},${r.amount},${r.currency},${r.status},"${meta}",${r.created_at}`;
  });

  res.setHeader('Content-Type', 'text/csv');
  res.setHeader('Content-Disposition', 'attachment; filename="transactions.csv"');
  res.send([header, ...lines].join('\n'));
});

app.get('/api/games', requireAuth, (req, res) => {
  const games = db.prepare('SELECT id, name, fen, pgn, created_at FROM games ORDER BY id DESC').all();
  res.json({ games });
});

app.post('/api/games/:id/favorite', requireAuth, (req, res) => {
  const game = db.prepare('SELECT id FROM games WHERE id = ?').get(req.params.id);
  if (!game) return res.status(404).json({ error: 'Game not found.' });

  const { tags, notes } = req.body;
  db.prepare(
    `INSERT INTO favorite_games(user_id, game_id, tags, notes)
     VALUES (?, ?, ?, ?)
     ON CONFLICT(user_id, game_id)
     DO UPDATE SET tags = excluded.tags, notes = excluded.notes`,
  ).run(req.user.id, game.id, tags || null, notes || null);

  return res.json({ success: true });
});

app.delete('/api/games/:id/favorite', requireAuth, (req, res) => {
  db.prepare('DELETE FROM favorite_games WHERE user_id = ? AND game_id = ?').run(req.user.id, req.params.id);
  return res.json({ success: true });
});

app.get('/api/users/me/favorites', requireAuth, (req, res) => {
  const rows = db.prepare(
    `SELECT g.id, g.name, g.fen, g.pgn, f.tags, f.notes, f.created_at AS favorited_at
     FROM favorite_games f
     JOIN games g ON g.id = f.game_id
     WHERE f.user_id = ?
     ORDER BY f.created_at DESC`,
  ).all(req.user.id);

  res.json({ favorites: rows });
});

app.get('/api/audit-logs/me', requireAuth, (req, res) => {
  const rows = db
    .prepare('SELECT id, action, metadata, created_at FROM audit_logs WHERE user_id = ? ORDER BY id DESC LIMIT 100')
    .all(req.user.id)
    .map((row) => ({ ...row, metadata: JSON.parse(row.metadata) }));
  res.json({ logs: rows });
});

app.get('/api/dev/sent-emails', (req, res) => {
  if (process.env.NODE_ENV === 'production') {
    return res.status(404).end();
  }
  return res.json({ sentEmails });
});

app.use(express.static(path.join(__dirname, '..', 'public')));

module.exports = { app };
