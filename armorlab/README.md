![](https://armorlab.org/img/git.jpg)

armorlab
==============

[ArmorLab](https://armorlab.org) is a software for PBR texture authoring - check out the [manual](https://armorlab.org/manual).

*Note 1: This repository is aimed at developers and may not be stable. Distributed binaries are [paid](https://armorlab.org/download) to help with the project funding. All of the development is happening here in order to make it accessible to everyone. Thank you for support!*

*Note 2: If you are compiling git version of ArmorLab, then you need to have a compiler ([Visual Studio](https://visualstudio.microsoft.com/downloads/) - Windows, [deps](https://github.com/armory3d/armortools/wiki/Linux-Dependencies) - Linux, [Xcode](https://developer.apple.com/xcode/resources/) - macOS / iOS, [Android Studio](https://developer.android.com/studio) - Android) and [git](https://git-scm.com/downloads) installed.*

```bash
git clone --recursive https://github.com/armory3d/armortools
cd armortools/armorlab
git clone https://github.com/armory3d/onnx_bin onnx
# Unpack `models.zip` from https://github.com/armory3d/armorai/releases into `assets/models` using 7-Zip - Extract Here
```

**Windows**
```bash
..\armorcore\make --graphics direct3d11
# Open generated Visual Studio project at `build\ArmorLab.sln`
# Build and run for x64 & release
```

**Linux** *wip - cpu only*
```bash
../armorcore/make --graphics opengl --run
```

**macOS** *wip - apple silicon only*
```bash
../armorcore/make --graphics metal
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
