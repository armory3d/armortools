![](https://armorpaint.org/img/git.jpg)

armorpaint
==============

[ArmorPaint](https://armorpaint.org) is a software for 3D PBR texture painting - check out the [manual](https://armorpaint.org/manual/).

*Note 1: This repository is aimed at developers and may not be stable. Distributed binaries are currently [paid](https://armorpaint.org/download) to help with the project funding. All of the development is happening here in order to make it accessible to everyone. Thank you for support!*

*Note 2: If you are compiling git version of ArmorPaint, then you need to have a compiler ([Visual Studio](https://visualstudio.microsoft.com/downloads/) - Windows, [clang](https://clang.llvm.org/get_started.html) + [deps](https://github.com/Kode/Kha/wiki/Linux) - Linux, [Xcode](https://developer.apple.com/xcode/resources/) - macOS), [nodejs](https://nodejs.org/en/download/) and [git](https://git-scm.com/downloads) installed. Learn more about [Kha](https://github.com/Kode/Kha/wiki), [Kinc](https://github.com/Kode/Kinc/wiki) and [Krom](https://github.com/Kode/Krom/blob/master/readme.md).*
```bash
git clone --recursive https://github.com/armory3d/armorpaint
cd armorpaint
```
```bash
# Windows
node Kromx/make -g direct3d11
cd Kromx
# Unpack `V8\Libraries\win32\release\v8_monolith.7z` using 7-Zip (exceeds 100MB)
node Kinc/make -g direct3d11
# Open generated Visual Studio project
# Set command-line arguments to `..\..\build\krom`
# Build for x64 & release
```
```bash
# Linux
node Kromx/make -g opengl
cd Kromx
node Kinc/make -g opengl --compiler clang --compile
cd Deployment
strip Krom
./Krom ../../build/krom
```
```bash
# macOS
node Kromx/make -g opengl
cd Kromx
node Kinc/make -g opengl
# Open generated Xcode project
# Set command-line arguments to `armorpaint_repo/build/krom`
# Add `path/to/Kromx/V8/Libraries/macos/release` into Library Search Paths
# Build
```
```bash
# Updating cloned repository
git pull origin master
git submodule update --init --recursive
```
