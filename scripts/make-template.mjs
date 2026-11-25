import { readFileSync, writeFileSync } from "node:fs";

const [input, output, codeSizeText = "2048", configSizeText = "64"] = process.argv.slice(2);
const codeSize = Number(codeSizeText);
const configSize = Number(configSizeText);

if (!input || !output || !Number.isInteger(codeSize) || !Number.isInteger(configSize) || codeSize <= 0 || configSize < 0) {
  throw new Error("usage: node make-template.mjs <firmware.bin> <template.bin> [code-size] [config-size]");
}

const firmware = readFileSync(input);
if (firmware.length > codeSize) {
  throw new Error(`${input} is ${firmware.length} bytes and does not fit the ${codeSize}-byte code region`);
}

const template = Buffer.alloc(codeSize + configSize, 0xff);
firmware.copy(template);
writeFileSync(output, template);
console.log(`template: ${template.length} bytes (${firmware.length} bytes firmware + ${codeSize - firmware.length} bytes fill + ${configSize} bytes config)`);
