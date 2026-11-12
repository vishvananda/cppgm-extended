#!/usr/bin/env node
// Optional maintainer driver; dependencies and browser binaries stay external.
const fs = require('fs');
const path = require('path');
const http = require('http');
const [directory, playwright, chromium, firefox, selected = 'all'] = process.argv.slice(2);
if (!firefox) throw Error('usage: wasi_browser_test.cjs PACKAGE PLAYWRIGHT_MODULE CHROMIUM FIREFOX');
if (!['all', 'chromium', 'firefox'].includes(selected)) throw Error('unknown browser selection');
const root = path.resolve(directory);
const pw = require(path.resolve(playwright));
const server = http.createServer((req, res) => {
  const requested = req.url === '/' ? '/wasi_browser.html' : req.url.split('?')[0];
  const file = path.resolve(root, '.' + decodeURIComponent(requested));
  if (!file.startsWith(root + '/')) {res.writeHead(403); res.end(); return;}
  const extension = path.extname(file);
  res.setHeader('Content-Type', extension === '.wasm' ? 'application/wasm'
    : ['.mjs', '.js'].includes(extension) ? 'text/javascript'
    : extension === '.html' ? 'text/html' : 'application/octet-stream');
  const stream = fs.createReadStream(file);
  stream.on('error', () => {res.writeHead(404); res.end();});
  stream.pipe(res);
});
(async () => {
  await new Promise(resolve => server.listen(0, '127.0.0.1', resolve));
  try {
    for (const [name, engine, executablePath] of [
      ['chromium', pw.chromium, chromium], ['firefox', pw.firefox, firefox]]) {
      if (selected !== 'all' && selected !== name) continue;
      const browser = await engine.launch({headless: true, executablePath,
        args: name === 'chromium' ? ['--no-sandbox'] : []});
      try {
        const context = await browser.newContext({viewport: null, acceptDownloads: true});
        const page = await context.newPage();
        await page.goto(`http://127.0.0.1:${server.address().port}/`);
        await page.locator('#start').click();
        await page.waitForFunction(() => ['progress', 'error'].includes(window.lastInceptionEvent?.kind),
                                   null, {timeout: 180000});
        if (await page.evaluate(() => window.lastInceptionEvent.kind) === 'error')
          throw Error(await page.evaluate(() => window.lastInceptionEvent.error));
        await page.locator('#cancel').click();
        if (await page.locator('#downloads a').count()) throw Error('cancelled run published artifacts');
        console.log(`${name}: cancellation passed; starting fresh full run`);
        await page.locator('#start').click();
        const progressTimer = setInterval(async () => {
          try {console.log(`${name}: ${await page.locator('#status').innerText()}`);} catch (_) {}
        }, 30000);
        try {
          await page.waitForFunction(() => ['complete', 'error'].includes(window.lastInceptionEvent?.kind),
                                     null, {timeout: 3600000});
        } finally {clearInterval(progressTimer);}
        const result = await page.evaluate(() => window.lastInceptionEvent);
        result.browser = name; result.version = browser.version(); result.cancellationPassed = true;
        fs.writeFileSync(path.join(root, `${name}-results.json`), JSON.stringify(result, null, 2) + '\n');
        if (result.kind !== 'complete') throw Error(result.error);
        const downloadPromise = page.waitForEvent('download');
        await page.getByText('cppgm-w3.wasm', {exact: true}).click();
        await (await downloadPromise).saveAs(path.join(root, `${name}-w3.wasm`));
        if (!fs.readFileSync(path.join(root, `${name}-w3.wasm`)).equals(
            fs.readFileSync(path.join(root, 'inputs/w1/cppgm.wasm')))) throw Error('downloaded module differs');
        console.log(`${name}: W2/W3 byte-identical, ${result.seconds.toFixed(1)}s, peak ${result.peakMemoryBytes} bytes`);
      } finally {await browser.close();}
    }
  } finally {server.close();}
})().catch(error => {console.error(error); process.exitCode = 1; server.close();});
