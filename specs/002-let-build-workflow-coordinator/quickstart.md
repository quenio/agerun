# Quickstart: Workflow Coordinator

This document describes the intended user-visible flow for the boot-launched, methods-only workflow
coordinator example.

## Prerequisites

- The runtime loads the bundled workflow method assets from `methods/`
- `bootstrap-1.0.0.method` remains the executable's startup entry point on fresh runs
- The default workflow definition file is available under the top-level `workflows/` directory
  and within the allowed file-delegate path
- The bundled default workflow's `complete(...)` dependency is available locally. For a normal demo
  run, prepare the local runtime/model with `make complete-runtime-ready 2>&1` or an equivalent
  existing repository flow that leaves the vendored runtime and model available
- Deterministic tests may instead use `AGERUN_COMPLETE_RUNNER` as an explicit fake-runner override
- The repository builds successfully with `make clean build 2>&1`

## 1. Run the executable

```bash
make run-exec 2>&1
```

## 2. Expected startup behavior

On a fresh run with no restored workflow agents:

- the executable loads the bundled method assets from `methods/`
- the executable creates the `bootstrap` agent and queues `"__boot__"`
- `bootstrap` spawns `workflow-coordinator`
- if `bootstrap` receives a duplicate boot trigger later in the same runtime, that duplicate trigger
  is ignored and no duplicate workflow agents are created
- `workflow-coordinator` spawns:
  - the generic workflow-definition agent
  - the workflow reporter agent
- the workflow-definition agent reads `workflows/default.workflow` through the file delegate
- the workflow-definition agent validates the workflow definition shape and, when the bundled workflow requires
  local completion, probes `complete(...)` readiness before item creation
- only after the definition/dependency probe succeeds does `workflow-coordinator` spawn one
  workflow item agent for the bundled demo item
- the workflow item begins evaluating transitions through the workflow-definition agent
- each transition decision comes from a definition-owned `complete(...)` template that includes
  `{outcome}` and `{reason}` placeholders
- the workflow reporter emits human-readable lifecycle progress, retryable error messages, and a
  final summary to the normal runtime log output

If the required local `complete(...)` runtime/model is unavailable for the bundled default
workflow:

- the executable emits a clear startup dependency failure
- the workflow item is not created
- the run does **not** pretend a transition or terminal outcome occurred

On a run where persisted workflow agents were restored:

- the executable does **not** create a duplicate fresh workflow demo
- restored workflow agents resume from their last valid state instead

## 3. Expected lifecycle checkpoints

A successful bundled default workflow demonstrates at least these visible lifecycle checkpoints:

1. item created / intake started
2. item advanced to triage
3. item advanced to active work
4. item advanced to review
5. item reached a definition-driven terminal outcome (`completed` or `rejected`)
6. final summary emitted

The exact terminal outcome depends on the workflow definition's transition prompts, not on
hard-coded boot script logic. The visible messages should surface the relevant decision reason when
one is available.

## 4. Example observable output

Illustrative examples of human-readable log messages:

```text
workflow demo started: item=demo-001 stage=intake status=new owner=ops reason="workflow initialized"
workflow progress: item=demo-001 stage=triage status=triaged owner=ops transitions=1 reason="intake details satisfy triage entry"
workflow progress: item=demo-001 stage=active status=in_progress owner=ops transitions=2 reason="triage selected the active-work path"
workflow progress: item=demo-001 stage=review status=ready_for_review owner=ops transitions=3 reason="implementation details are ready for review"
workflow summary: item=demo-001 outcome=completed stage=completion status=completion owner=ops transitions=4 reason="review confirmed completion criteria"
```

A retryable completion/runtime failure during a transition could instead produce output such as:

```text
workflow progress: item=demo-001 stage=review status=waiting owner=ops transitions=3 reason="retryable completion failure: local model timed out"
```

A startup dependency failure could produce output such as:

```text
workflow startup failure: definition=default-workflow reason="complete() runtime unavailable; run make complete-runtime-ready"
```

If the normal final summary cannot be produced or delivered, the runtime emits a fallback summary
that still identifies the item, the last lifecycle stage reached, and the terminal outcome or
failure reason.

## 5. Test-oriented usage

Method-level validation should use `ar_method_fixture` and load:

- `workflows/default.workflow` for bundled behavior checks
- `workflows/test.workflow` when a test must force `advance`, `stay`, `reject`, or startup
  dependency failure paths deterministically
- `AGERUN_COMPLETE_RUNNER=<fake-runner>` when tests need exact `outcome=` / `reason=` lines without
  the real local model/runtime

Illustrative validation commands:

```bash
make workflow_definition_tests 2>&1
make workflow_item_tests 2>&1
make workflow_coordinator_tests 2>&1
make workflow_reporter_tests 2>&1
make bootstrap_tests 2>&1
make ar_executable_tests 2>&1
make ar_complete_instruction_evaluator_tests 2>&1
make ar_local_completion_tests 2>&1
```

## 6. Documentation expectations

When the feature lands, repository-facing documentation should make clear that:

- the workflow coordinator is launched by the executable boot method, not by `arsh`
- the workflow behavior lives in AgeRun methods, not in new C feature modules
- the workflow definition is swappable through workflow files read by the file delegate and validated
  against the documented definition/schema contracts
- the bundled default workflow uses `complete(...)` as part of each configured transition decision
- bundled startup fails clearly if the required local completion dependency is unavailable
- workflow-visible progress and final summaries surface the relevant transition `reason` when one
  exists
