![](https://armorpaint.org/img/git_root.jpg)

armortools
==============

3D content creation tools.

[armorforge/](https://github.com/armory3d/armortools/tree/main/forge)

**Generating a locale file**
```bash
./base/make --js base/tools/extract_locales.js <locale code>
# Generates a `base/assets/locale/<locale code>.json` file
```

**Embedding data files**
```bash
# Requires compiler with c23 #embed support (clang 19 or newer)
../base/make --embed
```
