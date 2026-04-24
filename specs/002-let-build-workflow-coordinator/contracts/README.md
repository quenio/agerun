# Workflow Coordinator Contracts

This directory documents the externally testable contracts of the currently implemented
workflow-coordinator feature.

## Files

- [workflow-definition-schema.md](./workflow-definition-schema.md)
  - definition-file contract currently recognized by `workflow-definition`
- [workflow-runtime-messages.md](./workflow-runtime-messages.md)
  - runtime message contracts among `bootstrap`, `workflow-coordinator`, `workflow-definition`,
    and `workflow-reporter`

## Contract Scope

These contracts currently describe:
- how bundled workflow definitions are identified at runtime
- how startup readiness and transition decisions are normalized
- how the workflow bootstrap methods exchange messages
- what visible runtime output is expected through the log delegate

They intentionally do **not** describe:
- hypothetical future workflow-definition/file-delegate parsing beyond the current implementation
- new C APIs or modules for workflow execution
- shell-driven operation
- graphical or network interfaces
