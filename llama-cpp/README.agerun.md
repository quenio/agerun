# AgeRun vendoring notes for `llama.cpp`

This top-level `llama-cpp/` directory is the intended home for the pinned upstream `llama.cpp`
source tree used by AgeRun's direct `complete(...)` integration.

Pinned upstream snapshot:

- Tag: `b8826`
- Commit: `30dce2cf29d16d194f46a9a633ccbc3d36ce24ba`
- Origin: `https://github.com/ggml-org/llama.cpp.git`
- Pin record: `llama-cpp/PINNED-VERSION.agerun`

Planned layout:

- Vendored source root: `llama-cpp/`
- Local build directory: `.deps/llama.cpp-build`
- Local install prefix: `.deps/llama.cpp-install`
- AgeRun-specific notes: `llama-cpp/README.agerun.md`

Build the project-controlled CPU-only dependency with:

```bash
make vendor-llama-cpu
```

Show the configured source/install paths with:

```bash
make print-llama-config
```

## License compliance

Upstream `llama.cpp` is MIT-licensed. The key redistribution requirement is that the copyright
notice and permission notice stay included in all copies or substantial portions of the software.

For AgeRun packaging, that means:

- keep the upstream `llama-cpp/LICENSE` file in the vendored source tree
- do not strip upstream copyright/license notices from vendored files
- if AgeRun distributes vendored `llama.cpp` source or binaries built from it, include the
  upstream MIT license text in the distributed materials
- keep AgeRun-specific notes separate from upstream license text

The `make vendor-llama-cpu` target now fails if `llama-cpp/LICENSE` is missing.

## Notes

- The AgeRun integration targets CPU-only operation as the baseline contract.
- On macOS, `make vendor-llama-cpu` disables Metal with `-DGGML_METAL=OFF`.
- On Linux, the same target uses the default CPU build path.
- Future `ar_local_completion` work should include `llama.h` from the project-controlled install
  prefix rather than assuming a globally installed header or library.
- The GGUF model file is a separate artifact with its own license/distribution terms and must be
  handled separately from `llama.cpp` itself.
