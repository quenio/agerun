# ar_complete_instruction_evaluator

The `ar_complete_instruction_evaluator` module evaluates `complete(...)` instructions.

## Responsibility

- resolve target paths from placeholders for top-level and nested `memory...` writes
- request local placeholder values from `ar_local_completion`
- validate generated strings before any memory mutation occurs
- reject empty values, leading/trailing whitespace, and generated `{` or `}` characters
- verify that the generated values can be substituted back into the original template while preserving literal text
- write `1` or `0` to the instruction result path when the call is assigned
- return handled failure status without aborting the method for normal `complete(...)` failure cases

## Current implementation notes

- placeholder names are extracted once per template and staged before completion is requested
- nested base maps are created only after validation succeeds so semantic failures do not partially mutate memory
- successful writes store only strings in AgeRun memory for the first release
