const path = require('node:path');

const ROOT_DIR = path.resolve(__dirname, '..');

module.exports = {
  ROOT_DIR,
  PORT: Number(process.env.PORT || 3000),
  JWT_SECRET: process.env.JWT_SECRET || 'dev-jwt-secret-change-me',
  LOGIN_CHALLENGE_SECRET: process.env.LOGIN_CHALLENGE_SECRET || 'dev-login-secret-change-me',
  APP_BASE_URL: process.env.APP_BASE_URL || 'http://localhost:3000',
  DB_PATH: process.env.DB_PATH || path.join(ROOT_DIR, 'data', 'app.db'),
  EMAIL_VERIFY_TTL_MINUTES: Number(process.env.EMAIL_VERIFY_TTL_MINUTES || 60),
  PASSWORD_RESET_TTL_MINUTES: Number(process.env.PASSWORD_RESET_TTL_MINUTES || 20),
  S3_BUCKET: process.env.S3_BUCKET || '',
  AWS_REGION: process.env.AWS_REGION || 'us-east-1',
};
