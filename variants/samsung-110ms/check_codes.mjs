import { readFileSync } from "node:fs";
import { dirname, join } from "node:path";
import { fileURLToPath } from "node:url";

const HERE = dirname(fileURLToPath(import.meta.url));
const src = readFileSync(join(HERE, "src", "main.c"), "utf8");
const defs = {};
for (const line of src.split(/\r?\n/)) {
  const m = line.match(/^#define\s+(\w+)\s+(0x[0-9a-fA-F]+|\d+)u?/);
  if (m) defs[m[1]] = Number(m[2]);
}

const addr = defs.SAM_ADDR;
const expect = {
  SAM_CMD_POWER: 0xE0E040BF,
  SAM_CMD_ENTER: 0xE0E016E9,
  SAM_CMD_7: 0xE0E030CF,
  SAM_CMD_1: 0xE0E020DF,
  SAM_CMD_5: 0xE0E0906F,
  SAM_CMD_0: 0xE0E08877,
};

let fail = 0;
for (const [name, exp] of Object.entries(expect)) {
  const cmd = defs[name];
  const bytes = [addr, addr, cmd, (~cmd) & 0xff];
  const bits = [];
  for (const b of bytes) for (let i = 0; i < 8; i++) bits.push((b >> i) & 1);
  let v = 0;
  for (const bit of bits) v = ((v * 2) + bit) >>> 0;
  const ok = v === exp;
  if (!ok) fail++;
  const hex = (x) => "0x" + x.toString(16).toUpperCase().padStart(8, "0");
  console.log(`${ok ? "PASS" : "FAIL"}  ${name.padEnd(15)} addr=${addr} cmd=0x${cmd.toString(16).padStart(2, "0")} -> ${hex(v)}  (published ${hex(exp)})`);
}
console.log(fail ? `${fail} MISMATCH` : "all frames match published Samsung codes");
process.exit(fail ? 1 : 0);
