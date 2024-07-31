![](https://armorpaint.org/img/git_root.jpg)

armortools
==============

3D content creation tools.

[armorpaint/](https://github.com/armory3d/armortools/tree/main/armorpaint)<br>
[armorlab/](https://github.com/armory3d/armortools/tree/main/armorlab)

**Generating a locale file**
```bash
export ARM_LOCALE=<locale code>
./armorcore/make --js base/tools/extract_locales.js
# Generates a `base/assets/locale/<locale code>.json` file
```
