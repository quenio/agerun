# ar_complete_instruction_evaluator

The `ar_complete_instruction_evaluator` module evaluates `complete(...)` instructions.

## Responsibility

- evaluate an optional input values map without mutating it
- apply build-style substitution for placeholders already present in the input map
- request local placeholder values from `ar_local_completion` only for missing placeholders
- validate generated strings before storing the result map
- reject empty values, leading/trailing whitespace, and generated `{` or `}` characters
- write a new result map to the instruction result path when the call is assigned
- include both original input-map values and completed values in the returned map
- return handled failure status without aborting the method for normal `complete(...)` failure cases

## Current implementation notes

- placeholder names are extracted once per template and staged before completion is requested
- templates with no supported `{name}` placeholders return a copied input map or an empty map without invoking local completion
- invalid second arguments fail unless they evaluate to map values
- the input values map is recursively copied before generated values are added, so the original map is not mutated
- runtime and semantic failure logs preserve actionable fields such as `failure_category`, `cause`, and `recovery_hint`
- generated-value validation failures return the copied input values map while omitting invalid generated values
- completed values are stored as strings in the returned map for the first release
