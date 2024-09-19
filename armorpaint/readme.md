![](https://armorpaint.org/img/git.jpg)

armorpaint
==============

[ArmorPaint](https://armorpaint.org) is a software for 3D PBR texture painting - check out the [manual](https://armorpaint.org/manual).

*Note 1: This repository is aimed at developers and may not be stable. Distributed binaries are [paid](https://armorpaint.org/download) to help with the project funding. All of the development is happening here in order to make it accessible to everyone. Thank you for support!*

*Note 2: If you are compiling git version of ArmorPaint, then you need to have a compiler ([Visual Studio with clang tools](https://visualstudio.microsoft.com/downloads/) - Windows, [clang + dependencies](https://github.com/armory3d/armortools/wiki/Linux-Dependencies) - Linux, [Xcode](https://developer.apple.com/xcode/resources/) - macOS / iOS, [Android Studio](https://developer.android.com/studio) - Android) and [git](https://git-scm.com/downloads) installed.*

```bash
git clone --recursive https://github.com/armory3d/armortools
cd armortools/armorpaint
```

**Windows (x64)**
```bash
..\armorcore\make --graphics direct3d11
# Open generated Visual Studio project at `build\ArmorPaint.sln`
# Build and run
```

**Linux (x64)**
```bash
../armorcore/make --graphics opengl --run
```

**macOS (arm64)**
```bash
../armorcore/make --graphics metal
# Open generated Xcode project at `build/ArmorPaint.xcodeproj`
# Build and run
```

**Android (arm64)** *wip*
```bash
../armorcore/make --graphics opengl --target android
# Open generated Android Studio project at `build/ArmorPaint`
# Build for device
```

**iOS (arm64)** *wip*
```bash
../armorcore/make --graphics metal --target ios
# Open generated Xcode project `build/ArmorPaint.xcodeproj`
# Build for device
```

**Windows DXR (x64)** *wip*
```bash
..\armorcore\make --graphics direct3d12
# Open generated Visual Studio project at `build\ArmorPaint.sln`
# Build and run
```

**Linux VKRT (x64)** *wip*
```bash
../armorcore/make --graphics vulkan --run
```
