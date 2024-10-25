// Extracts localizable strings from a set of source files and writes them to JSON files.
// This script can create new translations or update existing ones.
// Usage:
// `../../armorcore/make --js extract_locales.js <locale code>`
// Generates a `base/assets/locale/<locale code>.json` file

let locale = scriptArgs[4];

if (!locale) {
    console.log("Locale code not set!");
    std.exit();
}

let locale_path = "./base/assets/locale/" + locale + ".json";

let out = {};
let old = {};
if (fs_exists(locale_path)) {
    old = JSON.parse(fs_readfile(locale_path).toString());
}

let source_paths = [
    "base/sources", "base/sources/nodes",
    "armorpaint/sources", "armorpaint/sources/nodes",
    "armorlab/sources", "armorlab/sources/nodes",
    "armorsculpt/sources", "armorsculpt/sources/nodes",
    "armorforge/sources", "armorforge/sources/nodes"
];

for (let path of source_paths) {
    if (!fs_exists(path)) {
        continue;
    }

    let files = fs_readdir(path);
    for (let file of files) {
        if (!file.endsWith(".ts")) {
            continue;
        }

        let data = fs_readfile(path + "/" + file).toString();
        let start = 0;
        while (true) {
            start = data.indexOf('tr("', start);
            if (start == -1) {
                break;
            }
            start += 4; // tr("

            let end_a = data.indexOf('")', start);
            let end_b = data.indexOf('",', start);
            if (end_a == -1) {
                end_a = end_b;
            }
            if (end_b == -1) {
                end_b = end_a;
            }
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

fs_writefile(locale_path, JSON.stringify(out, Object.keys(out).sort(), 4));
