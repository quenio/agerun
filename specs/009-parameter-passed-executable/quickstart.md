# Quickstart: Executable Boot Method Override

This document describes how the planned boot-method override should behave for operators and for
repository validation.

## Prerequisites

- The repository builds successfully with `make clean build 2>&1`
- The requested boot method is available in the methods directory loaded by the executable
- The override value uses the methods-folder naming convention `<method-name>-<version>` (for
  example, `echo-1.0.0`)
- You understand whether you are testing a fresh startup or a restored-agent startup
- For a fresh-start override demonstration, remove or isolate any existing persisted runtime files
  first so the executable will create a new boot agent

## 1. Run with the default boot method

```bash
make run-exec 2>&1
```

Expected result on a fresh startup:
- the executable loads methods
- the executable creates the default `bootstrap` boot agent
- the executable queues the standard `"__boot__"` startup message
- the existing bundled startup flow runs exactly as it does today

## 2. Run with a boot-method override through Make

```bash
make run-exec BOOT_METHOD=echo-1.0.0 2>&1
```

Expected result on a fresh startup:
- `run-exec` forwards the override to the executable as a CLI parameter
- the executable creates the requested boot agent instead of `bootstrap`
- the executable still queues the standard `"__boot__"` startup message
- startup output makes the selected method identifier visible

## 3. Observe restored-agent precedence

If persisted agents are loaded at startup, the executable does not create a new boot agent. When an
override is also requested:

- the restored agents remain authoritative
- the override is skipped
- startup output explains that the override was not applied because agents were restored

## 4. Observe invalid override behavior

Example operator mistake:

```bash
make run-exec BOOT_METHOD=does-not-exist 2>&1
```

Expected result on a fresh startup:
- the executable reports that the requested boot method identifier is malformed or that the
  selected method could not be created
- the process exits with failure
- the executable does not silently fall back to `bootstrap`

## 5. Direct executable contract reference

The underlying executable contract is expected to be:

```bash
./agerun --boot-method echo-1.0.0
```

Repository validation should still prefer `make run-exec ...` so the normal build/run path remains
consistent with project workflow rules.

## 6. Validation commands

Targeted validation for this feature should include:

```bash
make ar_executable_tests 2>&1
make sanitize-tests 2>&1
make check-docs
make clean build 2>&1
make check-logs
```

## 7. Documentation expectations

When implemented, the repository documentation should clearly state:

- the default fresh-start boot method remains `bootstrap`
- operators can request an alternate boot method with the combined identifier used by `methods/`
- `make run-exec` exposes the same override through `BOOT_METHOD`
- restored persisted agents take precedence over the override
- invalid overrides fail clearly without default-method fallback
