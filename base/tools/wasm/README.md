
**Wasm (Linux, macOS or WSL)**
```bash
../../make --target wasm --compile
# Copy resulting iron.wasm file to build/out
# Copy index.html to build/out
# Copy https://github.com/armory3d/armortools/tree/main/sources/backends/data/wasm/JS-Sources to build/out
# Todo:
# start.js has hard-coded .wasm file name: https://github.com/armory3d/armortools/tree/main/sources/backends/data/wasm/JS-Sources/start.js#L48
# memory.c has hard-coded size: https://github.com/armory3d/armortools/tree/main/sources/libs/miniclib/memory.c#L6
```
