# WAMR

This directory contains a static x64 build of the GridWhale WAMR fork for embedding in GridWhale.

Source checkout:

- `https://github.com/gridwhale/wasm-micro-runtime/tree/gridwhale/yield-hooks`
- Build source for this package was expanded under `.wamr-src\wasm-micro-runtime-gridwhale-yield-hooks`

Build directory:

- `.wamr-build-staticcrt-x64`

Configuration:

- Visual Studio 18 2026, x64
- Static MSVC runtime: `/MT` for Release, `/MTd` for Debug
- `WAMR_BUILD_PLATFORM=windows`
- `WAMR_BUILD_TARGET=X86_64`
- `WAMR_BUILD_INTERP=1`
- `WAMR_BUILD_FAST_INTERP=1`
- `WAMR_BUILD_AOT=0`
- `WAMR_BUILD_JIT=0`
- `WAMR_BUILD_FAST_JIT=0`
- `WAMR_BUILD_LIBC_BUILTIN=1`
- `WAMR_BUILD_LIBC_WASI=0`
- `WAMR_BUILD_MULTI_MODULE=0`
- `WAMR_BUILD_LIB_PTHREAD=0`
- `WAMR_BUILD_LIB_WASI_THREADS=0`
- `WAMR_BUILD_SIMD=1`
- `WAMR_BUILD_REF_TYPES=1`
- `WAMR_BUILD_GRIDWHALE_YIELD=1`

GridWhale yield hook API:

- `wasm_runtime_set_yield_callback`
- `wasm_runtime_set_yield_check_interval`

Packaged files:

- `include\*.h`: public WAMR embedding headers from `core\iwasm\include`
- `lib\Debug\iwasm.lib`: Debug static library
- `lib\Release\iwasm.lib`: Release static library
- `gridwhale-yield-hooks.patch`: local patch for the GridWhale WAMR fork
- `LICENSE`: WAMR license
