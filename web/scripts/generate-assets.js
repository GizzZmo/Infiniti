const fs = require('node:fs');
const path = require('node:path');
const crypto = require('node:crypto');

const webRoot = path.join(__dirname, '..');
const publicDir = path.join(webRoot, 'public');
const distDir = path.join(webRoot, 'dist');
const assetsDir = path.join(distDir, 'assets');

function toPosixPath(filePath) {
  return filePath.split(path.sep).join('/');
}

function walkDirectory(rootDir, currentDir = rootDir) {
  const entries = fs.readdirSync(currentDir, { withFileTypes: true });
  return entries.flatMap((entry) => {
    const absolutePath = path.join(currentDir, entry.name);
    if (entry.isDirectory()) {
      return walkDirectory(rootDir, absolutePath);
    }
    return [absolutePath];
  });
}

fs.rmSync(distDir, { recursive: true, force: true });
fs.mkdirSync(assetsDir, { recursive: true });

const manifest = walkDirectory(publicDir)
  .sort()
  .map((sourcePath) => {
    const relativePath = path.relative(publicDir, sourcePath);
    const targetPath = path.join(assetsDir, relativePath);
    fs.mkdirSync(path.dirname(targetPath), { recursive: true });
    fs.copyFileSync(sourcePath, targetPath);

    const contents = fs.readFileSync(sourcePath);
    return {
      path: toPosixPath(relativePath),
      size: contents.length,
      sha256: crypto.createHash('sha256').update(contents).digest('hex'),
    };
  });

const assetManifest = {
  generatedAt: new Date().toISOString(),
  assetCount: manifest.length,
  assets: manifest,
};

fs.writeFileSync(path.join(distDir, 'asset-manifest.json'), `${JSON.stringify(assetManifest, null, 2)}\n`);
console.log(`Generated ${manifest.length} static assets in ${path.relative(webRoot, distDir)}.`);
