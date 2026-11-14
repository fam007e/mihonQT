# Building MihonQT

MihonQT builds with **Qt 6** (6.4+ supported, newer recommended), CMake 3.16+ and a C++17 compiler, plus two pkg-config dependencies: **gumbo** (HTML5 parser) and **libwebp**. QuaZip 1.5 is vendored in `libs/quazip` — no need to install it.

## Linux (Debian/Ubuntu)

```bash
sudo apt install build-essential cmake pkg-config \
    qt6-base-dev qt6-webengine-dev qt6-declarative-dev qt6-5compat-dev \
    libgumbo-dev libwebp-dev
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
./build/MihonQT
```

## Arch Linux

```bash
sudo pacman -S --needed base-devel cmake pkgconf qt6-base qt6-webengine qt6-declarative qt6-5compat gumbo libwebp quazip
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
```

## macOS (Homebrew)

```bash
brew install cmake pkg-config qt gumbo-parser webp
export PKG_CONFIG_PATH="$(brew --prefix)/lib/pkgconfig"
cmake -B build -S . -DCMAKE_PREFIX_PATH="$(brew --prefix qt)"
cmake --build build -j$(sysctl -n hw.ncpu)
```

## Windows (vcpkg + Qt online installer)

Manifest mode installs `zlib`, `gumbo`, `libwebp` and `pkgconf` automatically from `vcpkg.json`:

```powershell
vcpkg integrate install
cmake -B build -S . `
  -DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_INSTALLATION_ROOT/scripts/buildsystems/vcpkg.cmake" `
  -DVCPKG_TARGET_TRIPLET=x64-windows `
  -DCMAKE_PREFIX_PATH="$env:Qt6_DIR"
cmake --build build --config Release
```

## Optional build options

| Option | Default | Purpose |
|---|---|---|
| `-DBUILD_TESTS=ON` | ON | Qt Test suites (`ctest`) |
| `-DBUILD_FUZZERS=ON` | OFF | libFuzzer targets for the parser/image/sort code |
| `-DENABLE_SANITIZERS=ON` | OFF | ASan + UBSan on the main binary |

## Tests

```bash
cmake --build build -j
ctest --test-dir build
```

## GitHub Actions

Every push to `main` and every PR runs the three-platform build matrix, the sanitizer + test job, and CodeQL/Flawfinder scans. The Windows job uses a vcpkg binary cache keyed on `vcpkg.json` — it only rebuilds when a dependency changes.
