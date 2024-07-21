// Extracts localizable strings from a set of source files and writes them to JSON files.
// This script can create new translations or update existing ones.
// Usage:
// `export ARM_LOCALE=<locale code>`
// `../../armorcore/Kinc/make --kfile extract_locales.js`
// Generates a `base/Assets/locale/<locale code>.json` file

const fs = require('fs');

if (!process.env.ARM_LOCALE) {
    console.log("ARM_LOCALE env variable not set!");
}

let locale = process.env.ARM_LOCALE;
let locale_path = "./base/Assets/locale/" + locale + ".json";

let out = {};
let old = {};
if (fs.existsSync(locale_path)) {
    old = JSON.parse(fs.readFileSync(locale_path).toString());
}

let source_paths = [
    "base/Sources", "base/Sources/nodes",
    "armorpaint/Sources", "armorpaint/Sources/nodes",
    "armorlab/Sources", "armorlab/Sources/nodes",
    "armorsculpt/Sources", "armorsculpt/Sources/nodes",
    "armorforge/Sources", "armorforge/Sources/nodes"
];

for (let path of source_paths) {
    if (!fs.existsSync(path)) continue;

    let files = fs.readdirSync(path);
    for (let file of files) {
        if (!file.endsWith(".ts")) continue;

        let data = fs.readFileSync(path + "/" + file).toString();
        let start = 0;
        while (true) {
            start = data.indexOf('tr("', start);
            if (start == -1) break;
            start += 4; // tr("

            let end_a = data.indexOf('")', start);
            let end_b = data.indexOf('",', start);
            if (end_a == -1) end_a = end_b;
            if (end_b == -1) end_b = end_a;
            let end = end_a < end_b ? end_a : end_b;

            let val = data.substring(start, end);
            val = val.replaceAll("\\n", "\n");
            if (old.hasOwnProperty(val)) {
                out[val] = old[val];
            }
            else {
                out[val] = "";
            }
            start = end;
        }
    }
}

fs.writeFileSync(locale_path, JSON.stringify(out, Object.keys(out).sort(), 4));
