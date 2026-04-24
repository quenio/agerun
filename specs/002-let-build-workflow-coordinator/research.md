# Research: Workflow Coordinator

## Decision 1: Keep the feature logic in AgeRun methods launched from `bootstrap`

- **Decision**: Keep the workflow startup flow in AgeRun methods and launch it from
  `bootstrap-1.0.0.method`.
- **Status in implementation**: implemented.

## Decision 2: Use path-recognized definition assets for the current implementation

- **Decision**: The original plan targeted workflow definition read through the file delegate, but the current
  implementation resolves supported definitions by `definition_path` inside the
  `workflow-definition` method.
- **Rationale**: This kept the method implementation small enough to get the feature slices green.
- **Status in implementation**: implemented as a simplified path-based definition contract.

## Decision 3: Keep dedicated workflow methods

- **Decision**: Preserve separate methods for:
  - `workflow-coordinator`
  - `workflow-definition`
  - `workflow-item`
  - `workflow-reporter`
- **Status in implementation**: implemented.

## Decision 4: Use `complete(...)` for startup probe and transition normalization

- **Decision**: The definition method uses `complete(...)` for:
  - startup dependency probe
  - transition decision generation
- **Rationale**: This preserves the core clarified requirement that `complete(...)` participates in
  workflow decisions.
- **Status in implementation**: implemented.

## Decision 5: Map in-flight `complete(...)` failures to retryable `stay`

- **Decision**: When transition evaluation fails, normalize the result to:
  - `outcome = stay`
  - `retryable = 1`
  - `reason = complete_transition_failed`
- **Status in implementation**: implemented.

## Decision 6: Use deterministic fake-runner overrides in tests

- **Decision**: Use `AGERUN_COMPLETE_RUNNER` in workflow tests so outcome and reason values stay
  deterministic.
- **Status in implementation**: implemented.

## Decision 7: Favor bounded test scheduling over unbounded queue drains

- **Decision**: Replace open-ended `while (process_next_message)` coordinator drains with bounded
  step-by-step processing in tests.
- **Rationale**: This exposed real scheduling mistakes instead of hiding them inside apparent
  hangs.
- **Status in implementation**: implemented.

## Decision 8: Emit visible startup/intake output directly in simplified executable paths

- **Decision**: For the current implementation, bootstrap emits the intake log line directly and the
  coordinator success/failure paths hand off visible summary/startup-failure messages directly to
  the reporter.
- **Rationale**: This keeps the executable demo behavior visible and testable even while the full
  workflow-item orchestration is still simpler than the original broader design.
- **Status in implementation**: implemented.

## Notes

This document now reflects the current implementation choices rather than the broader original plan.
The method-level slices are green, the executable tests are aligned with the workflow demo, and the
remaining work is repository-wide validation and any later refinement of the broader orchestration
model.
