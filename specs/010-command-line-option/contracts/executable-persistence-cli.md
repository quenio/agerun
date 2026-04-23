# Contract: Executable Persistence CLI

## Purpose

Define the operator-facing startup contract for running the main `agerun` executable without loading
or saving persisted methodology and agency state.

## Invocation

```text
agerun [--boot-method <method-name-version>] [--no-persistence]
```

## Inputs

### `--no-persistence`

- Optional
- Disables persisted methodology loading, persisted agency loading, persisted methodology saving,
  and persisted agency saving for the current run
- Treats the executable session as a fresh-start run for startup-flow purposes

### `--boot-method <method-name-version>`

- Existing optional fresh-start boot override
- Remains compatible with `--no-persistence` because no persisted agents are restored in that mode

## Behavioral Contract

1. Without `--no-persistence`, the executable preserves its current persistence-enabled behavior.
2. With `--no-persistence`, the executable does not restore persisted methodology state during
   startup.
3. With `--no-persistence`, the executable does not restore persisted agency state during startup.
4. With `--no-persistence`, the executable continues through the normal fresh-start path rather than
   the restored-state path.
5. With `--no-persistence`, the executable does not save persisted methodology or agency state at
   shutdown.
6. Existing persisted files remain untouched; the option skips participation in persistence rather
   than deleting files.
7. If `--no-persistence` is combined with `--boot-method`, the boot override is evaluated using the
   existing fresh-start rules.
8. Unknown flags remain usage errors and stop startup before runtime initialization.

## Observability Contract

Startup and shutdown output must make these outcomes distinguishable to the operator:

- default persistence-enabled behavior in use
- persistence loading disabled for the current run
- persistence saving disabled for the current run
- fresh-start boot behavior occurring because persistence was intentionally disabled
- existing boot override behavior, if also requested

## Non-Goals

- No deletion or migration of existing persisted files
- No separate method-only or agent-only persistence controls
- No changes to persisted file formats
- No changes to the `arsh` executable CLI
