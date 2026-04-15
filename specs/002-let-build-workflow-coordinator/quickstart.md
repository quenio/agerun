# Quickstart: Workflow Coordinator

This document describes the intended user-visible flow for the boot-launched, methods-only workflow
coordinator example.

## Prerequisites

- The runtime loads the bundled workflow method assets from `methods/`
- `bootstrap-1.0.0.method` remains the executable's startup entry point on fresh runs
- The default YAML workflow definition file is available under the top-level `workflows/` directory
  and within the allowed file-delegate path
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
- `workflow-coordinator` spawns:
  - the generic workflow-definition agent
  - the workflow reporter agent
  - one workflow item agent for the bundled demo item
- the workflow-definition agent reads `workflows/default-workflow.yaml` through the file delegate
- the workflow item begins evaluating transitions through the workflow-definition agent
- the workflow reporter emits human-readable lifecycle progress and a final summary to the normal
  runtime log output

On a run where persisted workflow agents were restored:

- the executable does **not** create a duplicate fresh workflow demo
- restored workflow agents resume from their last valid state instead

## 3. Expected lifecycle checkpoints

The bundled default workflow demonstrates at least these visible lifecycle checkpoints:

1. item created / intake started
2. item advanced to triage
3. item advanced to active work
4. item advanced to review
5. item reached a validation-driven terminal outcome (`completed` or `rejected`)
6. final summary emitted

The exact terminal outcome depends on the YAML workflow definition's transition validation, not on
hard-coded boot script logic.

## 4. Example observable output

Illustrative examples of human-readable log messages:

```text
workflow demo started: item=demo-001 stage=intake status=new owner=ops
workflow progress: item=demo-001 stage=triage status=triaged owner=ops transitions=1
workflow progress: item=demo-001 stage=active status=in_progress owner=ops transitions=2
workflow progress: item=demo-001 stage=review status=ready_for_review owner=ops transitions=3
workflow summary: item=demo-001 outcome=completed stage=completed owner=ops transitions=4
```

A rejection-oriented workflow definition or item state would instead end with a rejection summary.

## 5. Test-oriented usage

Method-level validation should use `ar_method_fixture` and load:

- `workflows/default-workflow.yaml` for default behavior checks
- `workflows/test-workflow.yaml` when a test must force `stay` or `reject`

Illustrative validation commands:

```bash
make workflow_definition_default_tests 2>&1
make workflow_item_tests 2>&1
make workflow_coordinator_tests 2>&1
make workflow_reporter_tests 2>&1
make bootstrap_tests 2>&1
make ar_executable_tests 2>&1
```

## 6. Documentation expectations

When the feature lands, repository-facing documentation should make clear that:

- the workflow coordinator is launched by the executable boot method, not by `arsh`
- the workflow behavior lives in AgeRun methods, not in new C feature modules
- the workflow definition is swappable through YAML files read by the file delegate and validated
  against the documented definition/schema contracts
