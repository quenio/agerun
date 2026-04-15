# Research: Workflow Coordinator

## Decision 1: Keep the feature logic entirely in AgeRun methods and launch it from `bootstrap`

- **Decision**: Implement the workflow coordinator as new AgeRun method assets launched by the
  existing executable boot flow. Do not add new C feature modules for coordinator, item,
  definition, or reporter behavior.
- **Rationale**: The user explicitly constrained this feature to run from the executable's boot
  method and to be implemented as AgeRun methods rather than as shell-driven or C-driven logic.
  The runtime already loads `.method` assets and queues the `"__boot__"` message automatically, so
  the feature can reuse that startup path without new executable behavior.
- **Alternatives considered**:
  - Add new C modules for workflow orchestration: rejected because it violates the methods-only
    requirement.
  - Drive the feature through `arsh`: rejected because the user explicitly wants boot-driven
    executable startup, not shell usage.

## Decision 2: Represent the workflow definition as a YAML file read through the file delegate

- **Decision**: Store the workflow definition in a YAML file and have the workflow methods read that
  file through the existing file delegate instead of treating the definition as a `.method` asset.
  Use one bundled default YAML file for the executable demo and at least one alternate YAML file for
  validation tests.
- **Rationale**: The user explicitly corrected the earlier assumption and stated that agents can read
  files through the file delegate. Using YAML keeps the definition separate from the reusable method
  logic while still honoring the methods-only requirement for the coordinator behavior itself.
- **Alternatives considered**:
  - Represent the definition as a `.method` asset: rejected by user feedback.
  - Parse YAML in new C feature code: rejected because the feature logic must remain in AgeRun
    methods.
  - Hard-code the workflow directly into `bootstrap-1.0.0.method`: rejected because the spec and
    user feedback require a reusable definition artifact that tests can swap out.

## Decision 3: Use a four-method runtime pattern: coordinator, item, definition reader/evaluator, and reporter

- **Decision**: Introduce these method roles:
  - `workflow-coordinator`: boot-time orchestrator that spawns the other workflow agents and seeds
    one demo work item
  - `workflow-item`: stateful per-item agent that owns item memory and requests transition decisions
  - `workflow-definition`: generic definition agent that reads YAML through the file delegate and
    describes/evaluates the active workflow
  - `workflow-reporter`: logging/reporting agent that emits progress and final summaries through the
    existing runtime log delegate
- **Rationale**: This keeps the reusable workflow state (`workflow-item`) separate from the
  reusable YAML-backed definition logic (`workflow-definition`) and isolates user-visible output in a
  dedicated reporter. The resulting pattern is generic enough for future workflow YAML files while
  still being small enough for a bundled demo.
- **Alternatives considered**:
  - Put all behavior into one method: rejected because it couples reusable state handling, YAML
    definition loading, workflow rules, and reporting into one opaque method.
  - Keep a coordinator plus definition only: rejected because one reusable work item per agent is a
    clearer AgeRun pattern and aligns better with future extension.

## Decision 4: Let the workflow definition method evaluate transitions and return `advance` / `stay` / `reject`

- **Decision**: The workflow item will send a transition-evaluation request to the workflow
  definition agent for its current stage and current item fields. The definition agent will return a
  single decision describing the validation outcome, next stage (if any), terminal outcome (if any),
  and progress text.
- **Rationale**: The clarified spec requires declarative stages, transitions, validation clauses,
  transition-attached rules, and outcomes limited to `advance`, `stay`, or `reject`. Returning one
  explicit decision per transition lets the reusable item method stay generic while the definition
  method owns workflow-specific validation.
- **Alternatives considered**:
  - Make the item method interpret arbitrary declarative clauses itself: rejected because the method
    language has no generic iteration or dynamic field-resolution features suitable for a robust
    clause interpreter.
  - Allow custom outcome names: rejected because the clarified spec restricts outcomes to the fixed
    set `advance`, `stay`, and `reject`.

## Decision 5: Model the workflow definition schema as YAML plus a documented runtime message protocol

- **Decision**: Document both:
  - the YAML schema contract that names the required logical elements (metadata, item field schema,
    stages, transitions, validation clauses, terminal outcomes)
  - the runtime message contract describing file-read requests plus `describe` and
    `evaluate_transition` exchanges between the workflow item/coordinator and the generic
    workflow-definition method
- **Rationale**: The definition now lives in YAML, but the feature still needs a precise runtime
  contract describing how methods obtain and use that YAML content through the file delegate.
- **Alternatives considered**:
  - Document only the default YAML file shape: rejected because tests must be able to substitute a
    different workflow definition and still satisfy the same runtime contract.
  - Document only a human-readable narrative: rejected because planning and testing need precise,
    parseable message expectations.

## Decision 6: Keep the default boot demo short and deterministic while reserving `stay` for tests

- **Decision**: The default workflow definition will use a monotonic path that reaches a terminal
  outcome in one startup run, while alternate test workflow definitions will explicitly exercise the
  `stay` and `reject` outcomes.
- **Rationale**: The spec requires a clean boot demo with at least four visible lifecycle
  checkpoints, but it also requires test coverage for the supported validation outcomes. Keeping the
  bundled demo deterministic preserves a strong first-run experience without sacrificing validation
  coverage.
- **Alternatives considered**:
  - Make the default demo randomly complete or reject: rejected because it would make executable
    tests and user expectations unstable.
  - Force the default demo to exercise every possible outcome: rejected because it would complicate
    the boot demo and obscure the reusable pattern.

## Decision 7: Validation will focus on method tests first, then executable integration tests

- **Decision**: Add method-level tests for `bootstrap`, `workflow-coordinator`, `workflow-item`,
  `workflow-definition`, and `workflow-reporter`, then update executable tests to assert the new
  boot-time demo output, YAML definition-file reads, and persistence behavior.
- **Rationale**: This follows the repository's TDD expectations and keeps behavior changes anchored
  in method fixtures before touching end-to-end executable assertions.
- **Alternatives considered**:
  - Test only through `make run-exec`: rejected because it would hide method-level failures and
    make debugging much slower.
  - Skip executable coverage because feature logic is in methods: rejected because the user-facing
    requirement is specifically about the boot-driven executable experience.
