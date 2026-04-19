# Research: Executable Boot Method Override

## Decision 1: Use one named executable flag with the combined methods-folder identifier

- **Decision**: The main executable will accept `--boot-method <method-name-version>`, where the
  value matches the filename stem used in `methods/` (for example, `bootstrap-1.0.0` or
  `echo-1.0.0`).
- **Rationale**: This directly follows the repository's existing method naming convention and keeps
  the external interface to one override parameter instead of two coordinated parameters.
- **Alternatives considered**:
  - Separate `--boot-method` and `--boot-version` flags: rejected after user feedback because the
    repository already expresses both pieces together in method filenames.
  - Single positional argument for the override method: rejected because a named flag is clearer and
    consistent with the repository's existing CLI style in `arsh` (`--verbose`).
  - Environment-variable-only override: rejected because the specification calls for a parameter
    passed to the executable.

## Decision 2: Split the combined identifier inside the executable using the existing naming rule

- **Decision**: The executable will parse the override value by splitting at the final hyphen to
  recover the method name and version components.
- **Rationale**: That matches how method files are already named and avoids inventing a second
  external encoding such as `name@version`.
- **Alternatives considered**:
  - Require a different separator such as `@` or `:`: rejected because it diverges from the methods
    directory naming convention.
  - Add a second explicit version flag: rejected as unnecessary complexity.

## Decision 3: Expose the override through one `make run-exec` variable

- **Decision**: `make run-exec` will accept `BOOT_METHOD=<method-name-version>` and translate that
  value into the executable's `--boot-method` CLI argument.
- **Rationale**: This satisfies the user's planning input without creating a parallel target and
  keeps the canonical runtime contract at the executable boundary rather than moving it into
  environment-only behavior.
- **Alternatives considered**:
  - Add a separate target such as `run-exec-with-boot`: rejected because it duplicates the launch
    path and conflicts with the repository's preference for generic parameterized Make targets.
  - Add `BOOT_METHOD_VERSION` as a second Make variable: rejected because the version is already
    represented in the combined identifier.
  - Pass the override through environment variables directly to the executable: rejected because it
    would diverge from the specified executable-parameter behavior.

## Decision 4: Restored persisted agents take precedence over any override request

- **Decision**: If persisted agents are restored successfully, the executable will not create a new
  boot agent even when `--boot-method` or `BOOT_METHOD` is supplied, and startup output will make it
  explicit that the override was skipped.
- **Rationale**: This preserves the existing runtime contract that restored state wins over fresh
  bootstrap creation and satisfies the feature requirement that overrides apply only to fresh
  startup.
- **Alternatives considered**:
  - Force a new override-selected boot agent even when agents were restored: rejected because it
    would corrupt the current persistence model by mixing restored and fresh startup flows.
  - Silently ignore the override on restored startup: rejected because operators need observable
    confirmation of what happened.

## Decision 5: Invalid or unavailable overrides fail clearly without fallback

- **Decision**: If the requested combined identifier cannot be parsed into a valid method name and
  version, or if the resulting method/version cannot be instantiated as the initial agent, startup
  fails with an explicit error and does not fall back to the default bootstrap method.
- **Rationale**: Silent fallback would make it impossible for operators and tests to trust that the
  requested override actually ran.
- **Alternatives considered**:
  - Fall back to `bootstrap` after logging a warning: rejected because it violates the feature spec.
  - Treat invalid override input as non-fatal and continue with no boot agent: rejected because the
  executable would then start in an undefined partial state.

## Decision 6: Boot-capability is defined by successful creation plus standard boot-message delivery

- **Decision**: The executable will not try to semantically pre-validate whether a method is a
  “good boot method.” A method is considered boot-capable for this feature if its combined
  identifier resolves, the agent can be created, and the standard `"__boot__"` startup message can
  be queued.
- **Rationale**: The current runtime contract already centers on agent creation and message delivery.
  Adding semantic boot-method screening would require a new capability-discovery contract that is
  outside this feature's scope.
- **Alternatives considered**:
  - Maintain a hard-coded allow-list of approved boot methods: rejected because it would create a
    second registry outside the methodology.
  - Add method metadata declaring boot support: rejected because it would expand the method system
    beyond the current feature.

## Decision 7: Use executable integration tests and fixture passthrough rather than new helper scripts

- **Decision**: Extend the existing executable fixture so tests can request a combined boot-method
  override while still invoking `make run-exec`, and add executable tests for the new startup cases.
- **Rationale**: This preserves Make-only test discipline, keeps the launch path realistic, and
  avoids one-off shell helpers.
- **Alternatives considered**:
  - Run the binary directly in tests: rejected by project build discipline.
  - Create a dedicated script wrapper for override testing: rejected because the Makefile target and
    fixture already cover the needed path.

## Decision 8: Observe override behavior through startup logs and persisted agent metadata

- **Decision**: Tests and docs will rely on explicit startup messages that include the selected boot
  method identifier and on the existing persisted-agency behavior where useful, instead of adding a
  new test-only method asset.
- **Rationale**: The repository already includes boot-related observability in executable output, and
  existing methods such as `echo-1.0.0.method` are sufficient for an alternate-boot integration
  case.
- **Alternatives considered**:
  - Add a dedicated test-only boot-override method to `methods/`: rejected because it would add
    asset and documentation overhead to a feature that only changes startup selection.

## Notes

The design is intentionally small in scope: one executable CLI contract, one Makefile passthrough,
no method-language changes, and targeted executable/documentation updates.

Final implementation stayed within this scope: the repository now exposes the combined boot-method
identifier through both `agerun --boot-method ...` and `make run-exec BOOT_METHOD=...`, while
keeping restored-agent precedence and explicit no-fallback failure reporting.
