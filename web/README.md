# Infiniti Web Platform

This folder contains a Node.js web platform implementing:

- Email verification
- Password reset with session invalidation
- KYC upload + admin review workflow
- TOTP 2FA + recovery codes
- Transaction filtering/search + CSV export
- Favorite games management

## Run locally

```bash
npm install
npm run start
```

Open http://localhost:3000.

## Environment variables

- `PORT` (default `3000`)
- `DB_PATH` (default `web/data/app.db`)
- `JWT_SECRET`
- `LOGIN_CHALLENGE_SECRET`
- `APP_BASE_URL`
- `EMAIL_VERIFY_TTL_MINUTES`
- `PASSWORD_RESET_TTL_MINUTES`
- `S3_BUCKET` and `AWS_REGION` (optional; when omitted, uploads use non-persistent blackhole storage URI)

## Scripts

- `npm run lint`
- `npm run build` (generates `web/dist/assets` plus `web/dist/asset-manifest.json`)
- `npm run screenshots` (captures UI screenshots into `web/dist/screenshots`)
- `npm test`

## Notes

- The first registered user is auto-promoted to admin for KYC review access.
- In non-production mode, sent emails are exposed at `GET /api/dev/sent-emails` for local testing.

## CI artifacts

- GitHub Actions workflow `.github/workflows/web-packaging.yml` installs dependencies, validates the app, builds static assets, captures screenshots, and uploads them together as a packaging artifact.
