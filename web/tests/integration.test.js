const fs = require('node:fs');
const path = require('node:path');
const test = require('node:test');
const assert = require('node:assert/strict');
const request = require('supertest');
const otplib = require('otplib');

const testDbPath = path.join(__dirname, 'tmp-test.db');
if (fs.existsSync(testDbPath)) fs.unlinkSync(testDbPath);
process.env.DB_PATH = testDbPath;
process.env.NODE_ENV = 'test';

const { app } = require('../src/app');

function extractTokenFromEmail(text) {
  const tokenMatch = text.match(/token=([^\s]+)/);
  return tokenMatch ? decodeURIComponent(tokenMatch[1]) : null;
}

test('end-to-end auth, transactions, 2fa, kyc and favorites flow', async () => {
  const register = await request(app)
    .post('/api/auth/register')
    .send({ email: 'user@example.com', password: 'StrongPass1' })
    .expect(201);
  assert.equal(register.body.user.emailVerified, false);

  const emails1 = await request(app).get('/api/dev/sent-emails').expect(200);
  const verifyToken = extractTokenFromEmail(emails1.body.sentEmails[0].text);
  assert.ok(verifyToken);

  await request(app).get('/api/auth/verify-email/' + verifyToken).expect(200);

  const login = await request(app)
    .post('/api/auth/login')
    .send({ email: 'user@example.com', password: 'StrongPass1' })
    .expect(200);
  const token1 = login.body.token;
  assert.ok(token1);

  await request(app)
    .get('/api/transactions')
    .set('Authorization', 'Token ' + token1)
    .expect(200);

  const setup2fa = await request(app)
    .post('/api/auth/2fa/setup')
    .set('Authorization', 'Token ' + token1)
    .expect(200);
  const otp = await otplib.generate({ secret: setup2fa.body.secret });

  const enable2fa = await request(app)
    .post('/api/auth/2fa/enable')
    .set('Authorization', 'Token ' + token1)
    .send({ otp })
    .expect(200);
  assert.equal(enable2fa.body.recoveryCodes.length, 8);

  const login2 = await request(app)
    .post('/api/auth/login')
    .send({ email: 'user@example.com', password: 'StrongPass1' })
    .expect(200);
  assert.equal(login2.body.requires2fa, true);

  const otp2 = await otplib.generate({ secret: setup2fa.body.secret });
  const login2fa = await request(app)
    .post('/api/auth/login/2fa')
    .send({ challengeToken: login2.body.challengeToken, otp: otp2 })
    .expect(200);
  const token2 = login2fa.body.token;

  await request(app)
    .get('/api/games')
    .set('Authorization', 'Token ' + token2)
    .expect(200)
    .then(async (res) => {
      const gameId = res.body.games[0].id;
      await request(app)
        .post(`/api/games/${gameId}/favorite`)
        .set('Authorization', 'Token ' + token2)
        .send({ tags: 'opening', notes: 'test note' })
        .expect(200);
      await request(app)
        .get('/api/users/me/favorites')
        .set('Authorization', 'Token ' + token2)
        .expect(200)
        .then((favRes) => assert.equal(favRes.body.favorites.length, 1));
    });

  await request(app)
    .post('/api/auth/forgot-password')
    .send({ email: 'user@example.com' })
    .expect(200);

  const emails2 = await request(app).get('/api/dev/sent-emails').expect(200);
  const resetEmail = emails2.body.sentEmails.find((m) => m.subject.includes('Reset'));
  const resetToken = extractTokenFromEmail(resetEmail.text);
  assert.ok(resetToken);

  await request(app)
    .post('/api/auth/reset-password/' + resetToken)
    .send({ password: 'AnotherPass2' })
    .expect(200);

  await request(app)
    .get('/api/auth/me')
    .set('Authorization', 'Token ' + token2)
    .expect(401);

  await request(app)
    .post('/api/auth/login')
    .send({ email: 'user@example.com', password: 'AnotherPass2' })
    .expect(200);
});
