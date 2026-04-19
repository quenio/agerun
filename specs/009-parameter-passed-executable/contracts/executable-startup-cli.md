# Contract: Executable Startup CLI

## Purpose

Define the operator-facing startup contract for selecting an alternate boot method when launching the
main `agerun` executable.

## Invocation

```text
agerun [--boot-method <method-name-version>]
```

## Inputs

### `--boot-method <method-name-version>`

- Optional
- Selects the method name and version to use for fresh-start boot agent creation instead of the
  default `bootstrap-1.0.0`
- The value matches the filename stem used in `methods/` (for example, `echo-1.0.0`)

## Behavioral Contract

1. If no boot override is provided and no persisted agents are restored, the executable creates the
   default `bootstrap` boot agent.
2. If `--boot-method` is provided and no persisted agents are restored, the executable parses the
   combined identifier and attempts to create the initial boot agent from that method/version.
3. When the override is applied, the executable queues the same standard `"__boot__"` startup
   message used by the default boot flow.
4. If persisted agents are restored successfully, the executable does not create a fresh boot agent
   and reports that the requested override was skipped.
5. If the requested identifier cannot be parsed into a method name/version, or if the resulting
   method/version cannot be instantiated, startup fails clearly and does not fall back to the
   default boot method.
6. Unknown flags or missing option values are usage errors and stop startup before runtime
   initialization.

## Observability Contract

Startup output must make these outcomes distinguishable to the operator:

- default boot method selected
- override boot method selected
- override skipped because persisted agents were restored
- override rejected because the requested method identifier was malformed or the selected method
  could not be created
- usage error due to malformed CLI input

## Non-Goals

- No semantic pre-validation of whether a method is a “good” boot method beyond successful creation
  and standard boot-message delivery
- No changes to method-language syntax or persisted agent/method file formats
- No changes to the `arsh` executable CLI
