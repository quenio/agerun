# Research: Executable Persistence Disable Option

## Decision 1: Use one explicit executable flag named `--no-persistence`

- **Decision**: The main executable will accept `--no-persistence` as the operator-facing flag for a
  run that must not load or save persisted methodology or agency state.
- **Rationale**: The feature disables persistence behavior for the current executable session, so an
  explicit negative flag communicates the effect clearly without implying file deletion or a broader
  runtime mode change.
- **Alternatives considered**:
  - `--fresh`: rejected because it describes startup shape but not shutdown behavior, and it could
    be misread as only affecting boot-agent creation.
  - `--no-persist`: rejected because it is shorter but less clear in project documentation.
  - Environment-variable-only control: rejected because the spec calls for an executable startup
    option.

## Decision 2: The option disables both methodology and agency persistence together

- **Decision**: `--no-persistence` suppresses persisted methodology loading, persisted agency
  loading, persisted methodology saving, and persisted agency saving for the current run.
- **Rationale**: The user asked to disable loading and saving of both methodology and agency, and a
  single combined control avoids half-persistent states that would be harder for operators to reason
  about.
- **Alternatives considered**:
  - Separate method and agent flags: rejected because the feature scope calls for one startup option
    and the runtime behavior is easier to explain when both persistence stores are treated together.
  - Disable loading only: rejected because shutdown would otherwise recreate state and undermine the
    clean-run use case.
  - Disable saving only: rejected because startup would still restore stale state and fail the main
    user scenario.

## Decision 3: Persistence-disabled runs use the fresh-start startup path

- **Decision**: When `--no-persistence` is enabled, the executable skips persisted restores and then
  follows the same fresh-start initialization path used when no agents are restored.
- **Rationale**: This preserves the current startup model instead of inventing a third execution
  path, and it aligns with the operator expectation that disabling persistence means ignoring prior
  saved state for the run.
- **Alternatives considered**:
  - Create a separate startup mode with custom boot handling: rejected because it would increase
    branching without adding user value.
  - Start with no boot flow at all: rejected because the executable should still behave as a valid
    fresh runtime session.

## Decision 4: Existing persisted files remain untouched

- **Decision**: The feature does not delete, truncate, or rewrite pre-existing `agerun.methodology`
  or `agerun.agency` files when persistence is disabled.
- **Rationale**: The requested behavior is to skip loading and saving, not to mutate or clean up
  persisted state. Leaving files untouched makes the option safe for temporary experimentation.
- **Alternatives considered**:
  - Delete persisted files at startup: rejected because it would turn a temporary runtime choice
    into destructive state mutation.
  - Rename or back up persisted files automatically: rejected because it adds file-management
    behavior outside the requested scope.

## Decision 5: Expose the mode through `make run-exec NO_PERSISTENCE=...`

- **Decision**: `make run-exec` accepts a non-empty `NO_PERSISTENCE` value and translates that into
  the executable's `--no-persistence` CLI flag. `NO_PERSISTENCE=1` remains the documented example
  invocation.
- **Rationale**: Repository workflows should remain Make-driven, and the variable name clearly
  mirrors the executable feature without introducing a duplicate target.
- **Alternatives considered**:
  - A dedicated target such as `run-exec-no-persistence`: rejected because it duplicates the launch
    path and conflicts with the project preference for parameterized Make targets.
  - `PERSIST=0`: rejected because it is shorter but less explicit and easier to misuse.
  - No Make exposure: rejected because project workflow guidance prefers Make entry points over
    direct binary execution.

## Decision 6: The option must coexist cleanly with existing boot override behavior

- **Decision**: If `--no-persistence` is used together with `--boot-method`, the executable treats
  the run as a fresh session and applies the requested boot override because no persisted agents are
  restored.
- **Rationale**: Skipping persistence removes the restored-state branch entirely, so the existing
  fresh-start boot override semantics should remain intact.
- **Alternatives considered**:
  - Reject combined use with boot override: rejected because the behaviors are compatible and the
    combination is useful for clean startup testing.
  - Ignore boot override when persistence is disabled: rejected because that would be surprising and
    inconsistent with the fresh-start model.

## Decision 7: Add coverage in executable integration tests and fixture passthroughs

- **Decision**: Extend the executable fixture to support persistence-disabled runs through the
  existing `make run-exec` path, and add executable tests for skipped loads, skipped saves, default
  behavior preservation, and interaction with boot overrides.
- **Rationale**: This preserves Make-only test discipline and validates the real startup/shutdown
  flow instead of isolated helper behavior.
- **Alternatives considered**:
  - Run the executable binary directly in tests: rejected by project build discipline.
  - Add shell wrappers just for this feature: rejected because the Make target and fixture already
    provide the required launch surface.

## Decision 8: Operator-visible output must state when persistence is disabled

- **Decision**: Startup output will explicitly report that persistence loading and saving are
  disabled for the current run.
- **Rationale**: The operator needs an observable distinction between a default restored run and an
  intentional fresh non-persistent run, especially when persisted files already exist.
- **Alternatives considered**:
  - Silent behavior change with no message: rejected because users would have no trustworthy way to
    confirm the option was applied.
  - Only mention skipped load/save opportunistically when files exist: rejected because the current
    mode should be explicit regardless of which files happen to be present.

## Notes

The design stays intentionally small in scope: one executable flag, one Make passthrough, no file
format changes, no runtime API expansion, and targeted executable/documentation updates only.
