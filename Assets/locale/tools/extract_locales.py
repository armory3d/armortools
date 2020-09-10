#!/usr/bin/env python3
# Extracts localizable strings from a set of source files and writes them to JSON files.
# This script can create new translations or update existing ones.
# Usage: ./extract_locales.py <locale code>

import fnmatch
import json
import os
import sys
from typing import Any, Dict, IO, List
from typing_extensions import Final

unique_str: List[str] = []


def process_file(f: IO[Any], fname: str, template_data: Dict[str, str]) -> None:
    line = f.readline()
    while line:
        patterns = ['tr("']
        idx = 0
        pos = 0
        while pos >= 0:
            pos = line.find(patterns[idx], pos)
            if pos == -1:
                if idx < len(patterns) - 1:
                    idx += 1
                    pos = 0
                continue
            pos += len(patterns[idx])

            msg = ""
            while pos < len(line) and (line[pos] != '"' or line[pos - 1] == "\\"):
                msg += line[pos]
                pos += 1

            # Only add each unique string once.
            if msg not in unique_str:
                # Empty keys are considered untranslated by the i18n library.
                # Fix newlines so they're not escaped anymore. Otherwise,
                # they won't match the source strings.
                template_data[msg.replace("\\n", "\n")] = ""
                unique_str.append(msg)

        line = f.readline()


def main() -> None:
    if len(sys.argv) != 2:
        sys.exit(f"Usage: {sys.argv[0]} <locale code>")

    # Change to the directory where the script is located,
    # so that the script can be run from any location.
    os.chdir(os.path.dirname(os.path.realpath(__file__)) + "/../../..")

    output_path: Final = f"Assets/locale/{sys.argv[1]}.json"

    if not os.path.exists("Sources"):
        sys.exit(
            "ERROR: Couldn't find the Sources folder in the folder where this script is located."
        )

    matches: List[str] = []
    for folder in ["Sources", "Libraries"]:
        for root, dirnames, filenames in os.walk(folder):
            dirnames[:] = [d for d in dirnames]
            for filename in fnmatch.filter(filenames, "*.hx"):
                matches.append(os.path.join(root, filename))
    matches.sort()

    template_data: Dict[str, str] = {}
    for filename in matches:
        with open(filename, "r", encoding="utf8") as f:
            # Read source files for localizable strings.
            process_file(f, filename, template_data)

    if os.path.exists(output_path):
        print(f'Updating the translation at "{output_path}"...')
        with open(output_path, "r", encoding="utf8") as f:
            existing_data = json.loads(f.read())
            # Remove obsolete translations (i.e. translations that are no longer
            # present in the generated data).
            existing_data_no_obsolete = {
                key: value
                for key, value in existing_data.items()
                if key in template_data
            }
            # Merge existing data with the generated template data (so we keep
            # existing translations).
            template_data = {**template_data, **existing_data_no_obsolete}

        with open(output_path, "w", encoding="utf8") as f:
            json.dump(template_data, f, ensure_ascii=False, indent=4)
    else:
        print(f'Creating new translation template at "{output_path}"...')
        with open(output_path, "w", encoding="utf8") as f:
            json.dump(template_data, f, ensure_ascii=False, indent=4)


if __name__ == "__main__":
    main()
