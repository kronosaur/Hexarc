# Assimp Dependency

This directory contains the static Assimp dependency used by LuminousArc.

- Assimp source revision: `af2c02c5794d5ad5e7f8f91f3c3144a4229cb7b5`,
  plus the COB Y-up and Local Axes pivot-frame changes. The fork includes
  GridWhale's COB `ShBx V0.04` texture-import
  and material-property changes, a COB absolute-to-relative hierarchy transform
  fix, a glTF2 mesh-merge reference-stability fix, geometric glTF2 buffer
  growth, shared-path texture-embedding reuse, and glTF2
  `KHR_texture_transform` export support. The COB changes preserve
  authored image color layers, their mix/mask metadata, separate source-image
  and mask transforms, displacement amplitude, trueSpace
  faceted/autofacet/smooth settings, and a recursive, typed reflectance-painter
  tree used by LuminousArc's focused glTF material translators. Source-painter
  export is opt-in through `AI_CONFIG_IMPORT_COB_SOURCE_MATERIALS`; legacy
  Assimp callers retain the existing projected PBR properties.
- zlib version: 1.2.13, built from Assimp's bundled dependency.
- Platform/toolset: x64, Visual Studio 2026 `v145`.
- Debug runtime: static multithreaded debug (`/MTd`).
- Release runtime: static multithreaded (`/MT`).
- Assimp is built static with the COB importer and glTF importer/exporter.

`include/assimp` combines Assimp's public headers with the generated
`config.h` and `revision.h` from the same build. `lib/Debug` and `lib/Release`
contain Assimp and its required zlib static library. The binary archives are
stored through Git LFS.

See `LICENSE-assimp.txt` and `LICENSE-zlib.txt` for their respective licenses.
