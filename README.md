![](https://armorpaint.org/img/git_root.jpg)

armortools
==============

3D content creation tools.

[armorpaint/](https://github.com/armory3d/armortools/tree/main/armorpaint)<br>
[armorlab/](https://github.com/armory3d/armortools/tree/main/armorlab)

**Updating cloned repository**
```bash
git pull origin main
git submodule update --init --recursive
# Delete `armorpaint/build` directory if present
```

**Generating a locale file**
```bash
pip install typing_extensions -t base/tools
python ./base/tools/extract_locales.py <locale code>
# Generates a `base/Assets/locale/<locale code>.json` file
```

**Release builds** *Optional, used for best performance*
```bash
# Compile krom.js using the closure compiler
https://developers.google.com/closure/compiler
# Generate a v8 snapshot file
export ARM_SNAPSHOT=1
../armorcore/Kinc/make --from ../armorcore -g api
./ArmorPaint . --snapshot
# Generates a `krom.bin` file from `krom.js` file
```
