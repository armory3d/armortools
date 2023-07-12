![](https://armorpaint.org/img/git.jpg)

armorpaint
==============

[ArmorPaint](https://armorpaint.org) is a software for 3D PBR texture painting - check out the [manual](https://armorpaint.org/manual).

*Note 1: This repository is aimed at developers and may not be stable. Distributed binaries are [paid](https://armorpaint.org/download) to help with the project funding. All of the development is happening here in order to make it accessible to everyone. Thank you for support!*

*Note 2: If you are compiling git version of ArmorPaint, then you need to have a compiler ([Visual Studio](https://visualstudio.microsoft.com/downloads/) - Windows, [clang](https://clang.llvm.org/get_started.html) + [deps](https://github.com/armory3d/armortools/wiki/Linux-Dependencies) - Linux, [Xcode](https://developer.apple.com/xcode/resources/) - macOS / iOS, [Android Studio](https://developer.android.com/studio) - Android) and [git](https://git-scm.com/downloads) installed.*

```bash
git clone --recursive https://github.com/armory3d/armortools
cd armortools/armorpaint
```

**Windows**
```bash
# Unpack `..\armorcore\v8\libraries\win32\release\v8_monolith.7z` using 7-Zip - Extract Here (exceeds 100MB)
..\armorcore\Kinc\make --from ..\armorcore -g direct3d11
# Open generated Visual Studio project at `build\ArmorPaint.sln`
# Build and run for x64 & release
# Copy build\x64\Release\ArmorPaint.exe to build\krom to run ArmorPaint.exe directly
```

**Linux**
```bash
../armorcore/Kinc/make --from ../armorcore -g opengl --compiler clang --compile
cd ../armorcore/Deployment
strip ArmorPaint
./ArmorPaint ../../armorpaint/build/krom
```

**macOS**
```bash
cd ../armorcore
git apply patch/metal_raytrace.diff --directory=Kinc
cd ../armorpaint
../armorcore/Kinc/make --from ../armorcore -g metal
cp -a build/krom/ ../armorcore/Deployment
# Open generated Xcode project at `build/ArmorPaint.xcodeproj`
# Set macOS Deployment Target to 13.0
# Build and run
```

**Android** *wip*
```bash
cd ../armorcore
git apply patch/android_document_picker.diff --directory=Kinc
cd ../armorpaint
../armorcore/Kinc/make --from ../armorcore -g opengl android
cp -r build/krom/* build/ArmorPaint/app/src/main/assets/
# Manual tweaking is required for now:
# https://github.com/armory3d/armorcore/blob/master/kfile.js#L136
# Open generated Android Studio project at `build/ArmorPaint`
# Build for device
```

**iOS** *wip*
```bash
cd ../armorcore
git apply patch/metal_raytrace.diff --directory=Kinc
git apply patch/ios_document_picker.diff --directory=Kinc
git apply patch/ios_pencil_hover.diff --directory=Kinc
cd ../armorpaint
../armorcore/Kinc/make --from ../armorcore -g metal ios
cp -a build/krom/ ../armorcore/Deployment
# Open generated Xcode project `build/ArmorPaint.xcodeproj`
# Set iOS Deployment Target to 16.0
# Build for device in release mode
```

**Windows DXR** *wip*
```bash
# Unpack `..\armorcore\v8\libraries\win32\release\v8_monolith.7z` using 7-Zip - Extract Here (exceeds 100MB)
cd ..\armorcore
git apply patch/d3d12_raytrace.diff --directory=Kinc
cd ..\armorpaint
..\armorcore\Kinc\make --from ..\armorcore -g direct3d12
# Open generated Visual Studio project at `build\ArmorPaint.sln`
# Build and run for x64 & release
# Copy build\x64\Release\ArmorPaint.exe to build\krom to run ArmorPaint.exe directly
```

**Linux VKRT** *wip*
```bash
git clone --recursive https://github.com/armory3d/glsl_to_spirv
cd ../armorcore
git apply patch/vulkan_raytrace.diff --directory=Kinc
cd ../armorpaint
../armorcore/Kinc/make --from ../armorcore -g vulkan --compiler clang --compile
cd ../armorcore/Deployment
strip ArmorPaint
./ArmorPaint ../../armorpaint/build/krom
```
