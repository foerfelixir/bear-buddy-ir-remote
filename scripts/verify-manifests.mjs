import { createHash } from "node:crypto";
import { readFileSync } from "node:fs";
import { dirname, join, resolve } from "node:path";
import { fileURLToPath } from "node:url";

const root = resolve(dirname(fileURLToPath(import.meta.url)), "..");
const manifest = readFileSync(join(root, "manifests", "SHA256SUMS"), "utf8");
let failed = 0;
let checked = 0;

for (const line of manifest.split(/\r?\n/)) {
  if (!line.trim() || line.startsWith("#")) continue;
  const match = line.match(/^([0-9a-f]{64})\s+\*?(.+)$/);
  if (!match) throw new Error(`bad manifest line: ${line}`);
  const [, expected, relativePath] = match;
  const actual = createHash("sha256").update(readFileSync(join(root, relativePath))).digest("hex");
  const ok = actual === expected;
  console.log(`${ok ? "PASS" : "FAIL"}  ${relativePath}`);
  if (!ok) {
    console.log(`      expected ${expected}\n      actual   ${actual}`);
    failed++;
  }
  checked++;
}

if (!checked) throw new Error("manifest contained no files");
console.log(failed ? `\n${failed} of ${checked} artifacts failed` : `\nall ${checked} artifacts match`);
process.exit(failed ? 1 : 0);
