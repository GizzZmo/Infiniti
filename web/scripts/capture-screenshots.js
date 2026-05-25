const fs = require('node:fs');
const path = require('node:path');
const { chromium } = require('playwright');

const webRoot = path.join(__dirname, '..');
const outputDir = process.env.SCREENSHOT_DIR || path.join(webRoot, 'dist', 'screenshots');
const baseUrl = (process.env.SCREENSHOT_BASE_URL || 'http://127.0.0.1:3000').replace(/\/$/, '');

const pages = [
  { route: '/index.html', file: 'index.png', waitFor: '#registerBtn' },
  { route: '/kyc.html', file: 'kyc.png', waitFor: '#uploadBtn' },
  { route: '/transactions.html', file: 'transactions.png', waitFor: '#searchBtn' },
  { route: '/favorites.html', file: 'favorites.png', waitFor: '#loadBtn' },
  { route: '/admin-kyc.html', file: 'admin-kyc.png', waitFor: '#loadBtn' },
  { route: '/verify-email.html?token=demo-token', file: 'verify-email.png', waitFor: '#status' },
  { route: '/reset-password.html?token=demo-token', file: 'reset-password.png', waitFor: '#submit' },
];

async function captureScreenshots() {
  fs.mkdirSync(outputDir, { recursive: true });

  const browser = await chromium.launch({ headless: true });
  try {
    const captured = [];
    for (const pageConfig of pages) {
      const page = await browser.newPage({ viewport: { width: 1440, height: 1080 } });
      const targetUrl = `${baseUrl}${pageConfig.route}`;
      await page.goto(targetUrl, { waitUntil: 'domcontentloaded' });
      await page.waitForSelector(pageConfig.waitFor);
      await page.screenshot({ path: path.join(outputDir, pageConfig.file), fullPage: true });
      await page.close();
      captured.push({ file: pageConfig.file, url: targetUrl });
    }

    fs.writeFileSync(
      path.join(outputDir, 'manifest.json'),
      `${JSON.stringify({ generatedAt: new Date().toISOString(), screenshots: captured }, null, 2)}\n`,
    );
    console.log(`Captured ${captured.length} screenshots in ${path.relative(webRoot, outputDir)}.`);
  } finally {
    await browser.close();
  }
}

captureScreenshots().catch((error) => {
  console.error(error);
  process.exitCode = 1;
});
