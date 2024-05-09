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
export ARM_LOCALE=<locale code>
./armorcore/Kinc/make --from base/Tools --kfile extract_locales.js
# Generates a `base/Assets/locale/<locale code>.json` file
```
