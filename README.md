![](https://armorpaint.org/img/git.jpg)

armorpaint
==============

[![Build Status](https://travis-ci.org/armory3d/armorpaint.svg?branch=master)](https://travis-ci.org/armory3d/armorpaint)

[ArmorPaint](https://armorpaint.org) is a software for 3D PBR texture painting - check out the [manual](https://armorpaint.org/manual/).

*Note 1: This repository is aimed at developers and may not be stable. Distributed binaries are currently [paid](https://armorpaint.org/download) to help with the project funding. All of the development is happening here in order to make it accessible to everyone. Thank you for support!*

*Note 2: If you are compiling git version of ArmorPaint, then you need to have a compiler ([Visual Studio](https://visualstudio.microsoft.com/downloads/) - Windows, [clang](https://clang.llvm.org/get_started.html) - Linux, [Xcode](https://developer.apple.com/xcode/resources/) - macOS), [nodejs](https://nodejs.org/en/download/) and [git](https://git-scm.com/downloads) installed. Learn more about [Kha](https://github.com/Kode/Kha/wiki), [Kore](https://github.com/Kode/Kore/wiki) and [Krom](https://github.com/Kode/Krom/blob/master/readme.md).*
```bash
git clone --recursive https://github.com/armory3d/armorpaint
cd armorpaint
```
```bash
# Windows
node Kha/make krom -g direct3d11
cd Krom
node Kore/make -g direct3d11
# Open generated Visual Studio project
# Set command-line arguments to `..\..\build\krom`
# Build for x64 & release
```
```bash
# Linux
node Kha/make krom -g opengl
cd Krom
node Kore/make -g opengl --compiler clang --compile
cp build/Release/Krom Deployment/Krom
cd Deployment
strip Krom
./Krom ../../build/krom
```
```bash
# macOS
node Kha/make krom -g opengl
cd Krom
node Kore/make -g opengl
# Open generated Xcode project
# Set command-line arguments to `armorpaint_repo/build/krom`
# Build
```
```bash
# Optional: Parse krom.js bytecode into krom.bin for startup efficiency
./Krom_binary armorpaint_repo/build/krom armorpaint_repo/build/krom-resources --writebin
```
