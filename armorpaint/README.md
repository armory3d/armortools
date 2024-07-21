![](https://armorpaint.org/img/git.jpg)

armorpaint
==============

[ArmorPaint](https://armorpaint.org) is a software for 3D PBR texture painting - check out the [manual](https://armorpaint.org/manual).

*Note 1: This repository is aimed at developers and may not be stable. Distributed binaries are [paid](https://armorpaint.org/download) to help with the project funding. All of the development is happening here in order to make it accessible to everyone. Thank you for support!*

*Note 2: If you are compiling git version of ArmorPaint, then you need to have a compiler ([Visual Studio](https://visualstudio.microsoft.com/downloads/) - Windows, [deps](https://github.com/armory3d/armortools/wiki/Linux-Dependencies) - Linux, [Xcode](https://developer.apple.com/xcode/resources/) - macOS / iOS, [Android Studio](https://developer.android.com/studio) - Android) and [git](https://git-scm.com/downloads) installed.*

```bash
git clone --recursive https://github.com/armory3d/armortools
cd armortools/armorpaint
```

**Windows**
```bash
..\armorcore\make --graphics direct3d11
# Open generated Visual Studio project at `build\ArmorPaint.sln`
# Build and run for x64 & release
```

**Linux**
```bash
../armorcore/make --graphics opengl --run
```

**macOS**
```bash
../armorcore/make --graphics metal
# Open generated Xcode project at `build/ArmorPaint.xcodeproj`
# Set macOS Deployment Target to 13.0
# Build and run
```

**Android** *wip*
```bash
../armorcore/make --graphics opengl --target android
cp -r build/krom/* build/ArmorPaint/app/src/main/assets/
# Manual tweaking is required for now:
# https://github.com/armory3d/armorcore/blob/master/kfile.js#L136
# Open generated Android Studio project at `build/ArmorPaint`
# Build for device
```

**iOS** *wip*
```bash
../armorcore/make --graphics metal --target ios
cp -a build/krom/ ../armorcore/Deployment
# Open generated Xcode project `build/ArmorPaint.xcodeproj`
# Set iOS Deployment Target to 16.0
# Build for device in release mode
```

**Windows DXR** *wip*
```bash
..\armorcore\make --graphics direct3d12
# Open generated Visual Studio project at `build\ArmorPaint.sln`
# Build and run for x64 & release
```

**Linux VKRT** *wip*
```bash
git clone --recursive https://github.com/armory3d/glsl_to_spirv
../armorcore/make --graphics vulkan --run
```
