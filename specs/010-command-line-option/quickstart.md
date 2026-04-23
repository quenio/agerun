# Quickstart: Executable Persistence Disable Option

This document describes how the implemented persistence-disable option behaves for operators and for
repository validation.

## Prerequisites

- The repository builds successfully with `make clean build 2>&1`
- You understand whether you want the default persistence-enabled behavior or a temporary
  non-persistent run
- Persisted files may already exist in the executable run directory; the option is intended to
  ignore them rather than remove them
- Repository validation should prefer Make entry points instead of launching the binary directly

## 1. Run with normal persistence behavior

```bash
make run-exec 2>&1
```

Expected result:
- the executable loads persisted methodology when available
- the executable loads persisted agents when available
- a fresh run saves methodology and agents at shutdown
- current startup and shutdown behavior remains unchanged

## 2. Run with persistence disabled through Make

```bash
make run-exec NO_PERSISTENCE=1 2>&1
```

Expected result:
- `run-exec` forwards the persistence-disable request to the executable
- the documented invocation uses `NO_PERSISTENCE=1`, and any non-empty `NO_PERSISTENCE` value
  triggers the same passthrough
- the executable skips persisted methodology loading
- the executable skips persisted agency loading
- the executable follows the fresh-start path for the current run
- the executable skips persisted methodology saving and agency saving at shutdown
- startup output clearly states that persistence is disabled for the current run

## 3. Combine clean startup with a boot override

```bash
make run-exec NO_PERSISTENCE=1 BOOT_METHOD=boot-echo-1.0.0 2>&1
```

Expected result:
- the executable still skips all persisted loading and saving
- because no persisted agents are restored, the run behaves like a fresh startup
- the requested boot override is applied using the existing fresh-start boot-selection rules

## 4. Direct executable contract reference

The underlying executable contract is:

```bash
./agerun --no-persistence
```

And when combined with an existing boot override:

```bash
./agerun --no-persistence --boot-method boot-echo-1.0.0
```

Repository validation should still prefer `make run-exec ...` to stay within project workflow rules.

## 5. Validation commands

Targeted validation for this feature should include:

```bash
make ar_executable_tests 2>&1
make sanitize-tests 2>&1
make check-docs
make check-naming 2>&1
make clean build 2>&1
make check-logs
```

## 6. Documentation expectations

The repository documentation now clearly states:

- the default executable behavior still loads and saves persisted methodology/agency state
- operators can disable both load and save paths for one run with `--no-persistence`
- `make run-exec` exposes the same behavior through `NO_PERSISTENCE=1` as the documented example
  invocation
- persistence-disabled runs ignore existing persisted files without deleting them
- persistence-disabled runs behave like fresh startup sessions for boot-selection purposes
