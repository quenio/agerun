# ar_local_completion

The `ar_local_completion` module is the local runtime adapter used by `complete(...)`.

## Responsibility

- resolve the local model path from `AGERUN_COMPLETE_MODEL` or the documented default path
- keep the runtime uninitialized until the first `complete(...)` request needs local completion
- lazily load the vendored `libllama` runtime from `.deps/llama.cpp-install/lib/`
- load the configured GGUF model with CPU-only settings and reuse that loaded model across calls for the same runtime instance
- support `AGERUN_COMPLETE_LIBLLAMA` as an explicit library-path override for direct-load experiments
- support `AGERUN_COMPLETE_RUNNER` as an explicit local runner override for tests and temporary fallback scenarios
- normalize placeholder-value responses into a flat `ar_data_t*` map of string values for `ar_complete_instruction_evaluator`
- surface actionable runtime failures through `ar_log__error()` without mutating AgeRun memory directly

## Current implementation notes

- the runtime keeps one adapter instance per instruction evaluator facade
- the direct path is implemented in C for simpler `llama.cpp` dynamic loading and symbol binding, while the public API remains the stable `ar_local_completion.h` header
- the direct backend expects a vendored `llama.cpp` source tree at top-level `llama-cpp/` and a built local runtime at `.deps/llama.cpp-install/`
- on macOS, `make vendor-llama-cpu` also patches installed rpaths so the vendored binaries and dynamic libraries can find each other from the local install prefix
- when `AGERUN_COMPLETE_RUNNER` is set, the adapter runs that explicit executable instead of the direct `libllama` path; this keeps deterministic fake-runner tests possible while the direct path is brought up
- successful responses return one string value per requested placeholder key
- both the direct prompt and the runner override request exact `name=value` lines and the adapter ignores unrelated output lines
- invalid requests such as missing template text, missing placeholders, non-positive `timeout_ms`, missing vendored runtime artifacts, or unavailable runner overrides fail with a logged runtime error and a `NULL` response
