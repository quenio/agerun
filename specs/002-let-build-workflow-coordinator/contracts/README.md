# Workflow Coordinator Contracts

This directory documents the external, user-relevant contracts for the methods-only workflow
coordinator feature.

## Files

- [workflow-definition-schema.md](./workflow-definition-schema.md)
  - Logical schema required of YAML workflow definition files read through the file delegate
- [workflow-runtime-messages.md](./workflow-runtime-messages.md)
  - Message contracts exchanged among bootstrap, coordinator, item, definition, and reporter agents

## Contract Scope

These contracts intentionally describe:
- what a YAML workflow definition must expose
- how workflow agents talk to one another
- what tests may substitute with alternate YAML definition files

They intentionally do **not** describe:
- new C APIs or modules for workflow execution
- method-file replacements for the YAML workflow definition
- shell-driven operation
- graphical or network interfaces
