![](https://armorlab.org/img/git.jpg)

armorlab
==============

[ArmorLab](https://armorlab.org) is a software for AI-powered texture authoring - check out the [manual](https://armorlab.org/manual).

*Note 1: This repository is aimed at developers and may not be stable. Distributed binaries are [paid](https://armorlab.org/download) to help with the project funding. All of the development is happening here in order to make it accessible to everyone. Thank you for support!*

*Note 2: If you are compiling git version of ArmorLab, then you need to have a compiler ([Visual Studio](https://visualstudio.microsoft.com/downloads/) - Windows, [clang](https://clang.llvm.org/get_started.html) + [deps](https://github.com/armory3d/armortools/wiki/Linux-Dependencies) - Linux, [Xcode](https://developer.apple.com/xcode/resources/) - macOS / iOS, [Android Studio](https://developer.android.com/studio) - Android) and [git](https://git-scm.com/downloads) installed.*

```bash
git clone --recursive https://github.com/armory3d/armortools
cd armortools/armorlab
# Unpack `models.zip` from https://github.com/armory3d/armorai/releases into `Assets/models` using 7-Zip - Extract Here
```

**Windows**
```bash
# Unpack `..\armorcore\v8\libraries\win32\release\v8_monolith.7z` using 7-Zip - Extract Here (exceeds 100MB)
..\armorcore\Kinc\make --from ..\armorcore -g direct3d11
# Open generated Visual Studio project at `build\ArmorLab.sln`
# Build and run for x64 & release
# Copy build\x64\Release\ArmorLab.exe to build\krom to run ArmorLab.exe directly
```

**Linux** *wip - cpu only*
```bash
../armorcore/Kinc/make --from ../armorcore -g opengl --compiler clang --compile
cd ../armorcore/Deployment
strip ArmorLab
./ArmorLab ../../armorlab/build/krom
```

**macOS** *wip - apple silicon only*
```bash
cd ../armorcore
git apply patch/metal_raytrace.diff --directory=Kinc
cd ../armorpaint
../armorcore/Kinc/make --from ../armorcore -g metal
cp -a build/krom/ ../armorcore/Deployment
# Open generated Xcode project at `build/ArmorLab.xcodeproj`
# Set macOS Deployment Target to 13.0
# Build and run
```

**Android** *wip*
```bash
```

**iOS** *wip*
```bash
```
