# Method Language Composability and Orthogonality Audit
**Date**: 2026-06-14
**Audit base**: `origin/main` at `ae3a064d` (`Merge pull request #28 from quenio/codex/separate-routing-methods`)
**Focus**: AgeRun method definition grammar, parser architecture, evaluator semantics, documentation, tests, and current method corpus.

## Executive Summary

The current AgeRun method language is only partially composable and only partially orthogonal.

The expression subsystem is the strongest composable core: literals, accessors, parenthesized
expressions, arithmetic, comparison, and one-line list/map literals are parsed into
`ar_expression_ast_t` and evaluated by one expression evaluator. However, built-in calls are modeled
as instruction AST nodes rather than expression AST nodes, so calls cannot be nested inside
expressions, operators, list items, map values, or other call arguments. This is the largest gap
against the stated principle.

The main semantic gaps are coupled to syntax and origin. Function result storage is a separate
instruction concept instead of normal assignment of an expression result. `append(...)` can accept
any target expression syntactically, but mutates only memory-owned lists. `head(...)`, `tail(...)`,
and missing `message` fields rely on integer `0` sentinels. These choices are useful in the current
language, but they are not fully orthogonal because equivalent-looking values can behave differently
depending on where they came from or how they are used.

This audit does not change behavior. It records the current state and recommends follow-up work.

## Audit Principles

This audit is guided by these principles:

1. **Composability**: Every grammar component should be usable in every compatible syntactic
   context. If a construct produces a value, it should be eligible wherever expressions are accepted
   unless the language defines a specific exception. This is the lens for questions such as whether
   built-in calls can appear inside call arguments, list/map literals, operators, assignments, and
   condition branches.
2. **Orthogonality**: Equivalent values should behave the same regardless of how they were produced.
   A value from a literal, accessor, function result, selected branch, or intermediate assignment
   should not gain hidden semantic differences unless the language defines an explicit distinction
   such as mutation target eligibility.
3. **Single Source of Semantics**: Each language behavior should have one clear owner in the parser,
   AST, evaluator, and documentation. Duplicate paths, such as regular assignment versus
   instruction-specific result assignment, are treated as audit findings because they make behavior
   easier to drift.
4. **Explicit Exceptions**: Any non-composable or non-orthogonal behavior should be intentional,
   documented, and test-covered. Useful restrictions, such as memory-only mutation targets, are not
   automatically wrong, but they must be visible as language rules rather than incidental evaluator
   side effects.
5. **Evidence Before Aspiration**: The audit separates the current language from the desired
   principle. It records what the current commit actually parses, evaluates, documents, and tests
   before recommending future changes.
6. **Compatibility-Aware Evolution**: Recommendations should preserve existing method behavior
   unless a later migration explicitly changes the language contract. Current corpus shape and
   existing tests are part of the design evidence.

## Documented Constraints and Principle Alignment

The current documentation already names several language constraints. This audit treats those
constraints as current-state evidence, then checks whether each constraint aligns with the audit
principles or should be treated as a future design gap.

| Documented Constraint | Documentation Evidence | Principle Alignment | Audit Treatment |
|-----------------------|------------------------|---------------------|-----------------|
| Methods are line-oriented: one instruction per line, no combined instructions, final newline required, empty lines ignored. | `SPEC.md`, `methods/README.md`, `modules/ar_method_parser.md` | Compatible with composability if it remains a statement-boundary rule. It is an explicit syntax constraint, not a semantic coupling. | Preserve unless the language later gains block or expression-statement syntax. |
| Standalone expressions are not instructions. Expressions must appear under assignment or allowed function calls. | `SPEC.md` | Limits composability at the statement level. It is explicit, but it keeps expression evaluation from being uniformly usable as a top-level form. | Track as a statement/expression split that must be revisited if calls become expressions. |
| Function calls are documented as instructions, not expressions; nested calls are forbidden. | `AGENTS.md`, `kb/agerun-method-language-nesting-constraint.md`, disabled parser tests | Directly conflicts with the composability principle. A value-producing construct cannot appear wherever expressions are accepted. | Treat as the primary composability gap. |
| One-line list/map literals can appear in expression contexts; multiline list/map literals are assignment-only and cannot appear as call arguments, list items, or map values. | `SPEC.md`, `README.md`, `modules/ar_method_parser.md` | Partially aligns. One-line literals are composable; multiline literals are an explicit but formatting-dependent exception. | Either document multiline literals as a deliberate source-format exception or move them into expression parsing. |
| Map literal keys must be identifiers; quoted keys are not supported. | `SPEC.md`, `modules/ar_expression_parser.md` | Mostly compatible as an explicit grammar restriction. It limits data shape expressiveness but does not by itself create semantic drift. | Preserve unless future data requirements need arbitrary string keys. |
| There is no null type; integer `0` is used as the absent/failure/no-op sentinel in several places. | `AGENTS.md`, `SPEC.md`, `modules/ar_expression_evaluator.md` | Conflicts with orthogonality when unrelated cases share the same value: missing field, empty `head(...)`, invalid `tail(...)`, no-op spawn, false condition. | Treat as a sentinel-coupling gap; consider an explicit absence value or predicates before adding more sentinel semantics. |
| No static type checking; methods must handle possible runtime types defensively. | `AGENTS.md`, `kb/agerun-language-constraint-workarounds.md` | Acceptable as a dynamic-language constraint, but it raises the burden on orthogonal runtime behavior and diagnostics. | Preserve as current-state behavior; improve documentation and tests around type-dependent built-ins. |
| `if(...)` is documented inconsistently: some docs say value selection, current evaluator docs say selected-branch evaluation, and one KB article says both branches are evaluated. | `SPEC.md`, `modules/ar_condition_instruction_evaluator.md`, `kb/agerun-language-constraint-workarounds.md` | Violates Single Source of Semantics. The current implementation is more orthogonal than the stale guidance, but the documented contract is unclear. | Correct documentation after deciding the expression-level `if(...)` model. |
| No conditional execution statement exists; all method instructions execute in order and conditional behavior is encoded with value selection and no-op targets. | `kb/agerun-language-constraint-workarounds.md`, coordination method patterns | Compatible with a small language, but it couples control flow to sentinels and no-op behavior. | Keep in scope for the call-as-expression and sentinel semantics follow-up plans. |
| `send(0, message)` is a no-op; `spawn(0, ...)` and `spawn("", ...)` are no-ops returning/storing `0`. | `AGENTS.md`, `SPEC.md`, `kb/no-op-semantics-pattern.md`, `kb/no-op-instruction-semantics.md` | Aligns with Explicit Exceptions when documented. It still adds semantic meaning to integer `0`. | Preserve current behavior, but classify it under sentinel/no-op semantics rather than ordinary value behavior. |
| `append(target, value)` accepts any target expression syntactically, but only mutates an existing memory-owned list; message/context/fresh/missing/non-list/protected targets are no-ops. | `SPEC.md`, `modules/ar_append_instruction_evaluator.md` | Explicit but not fully orthogonal: identical list values differ by origin and ownership. | Define lvalue/mutation-target rules before making mutating calls expression-level. |
| Writes are limited to `memory` paths, and `memory.self` plus nested `memory.self.*` are protected from assignment/result storage. | `SPEC.md`, `modules/ar_instruction_ast.md`, parse evaluator docs | Aligns with Explicit Exceptions and runtime identity safety. It is a legitimate non-orthogonal write rule if kept visible and test-covered. | Preserve, but centralize the rule so assignment, function results, parse templates, and future merge-like operations cannot drift. |
| `head(...)` and `tail(...)` use deep-copy list traversal and integer `0` for empty, missing, non-list, or copy-failure cases. | `SPEC.md`, `modules/ar_head_instruction_evaluator.md`, `modules/ar_tail_instruction_evaluator.md`, `modules/ar_expression_evaluator.md` | Partially aligns: list values are safely copied, but stop/failure behavior is sentinel-coupled and not fully orthogonal to valid integer payloads. | Keep as current behavior; include in sentinel semantics planning. |
| Non-empty list equality and map structural equality are not supported; list equality is limited to empty-list checks. | `modules/ar_expression_evaluator.md` | Limits orthogonality across container values. Empty and non-empty lists do not share a single structural comparison rule. | Treat as a lower-risk expression semantics gap. |

## Analysis Methodology

- Refreshed `origin/main` with `git fetch origin main`; verified `HEAD`, `origin/main`, and
  `FETCH_HEAD` all resolve to `ae3a064d`.
- Reviewed the formal grammar and semantics in `SPEC.md`, `README.md`, `AGENTS.md`, module docs,
  KB articles, parser/evaluator source, and parser/evaluator tests.
- Counted the current corpus to estimate migration surface:
  - 25 `.method` files under `methods/` and `methodologies/`
  - 1,851 nonblank, non-comment method-source lines
  - 1,023 assigned built-in call lines
  - 11 standalone built-in call lines
  - 36 top-level multiline literal assignments
  - 0 nested built-in call lines in `.method` files under the audit regex
  - 75 incomplete `TODO.md` items; none directly supersedes this audit
- Applied two separate lenses:
  - Composability: where each grammar component can appear and combine.
  - Orthogonality: whether equivalent values behave the same regardless of syntactic form or origin.

## Key Findings

| ID | Area | Composability Status | Orthogonality Status | Evidence | Risk |
|----|------|----------------------|----------------------|----------|------|
| F1 | Built-in calls | Not composable as expressions. Calls are accepted only as top-level function instructions, with optional result assignment. | Function results are stored through instruction-specific result assignment, not by normal expression assignment. | `SPEC.md` separates `<function-instruction>` from `<expression>`. `ar_expression_ast_t` has no call node, while `ar_instruction_ast_t` has per-call instruction types. `AGENTS.md` says function calls are not expressions. Disabled tests state function calls in expressions are not supported. | High |
| F2 | Multiline list/map literals | Not composable. Multiline literals are canonicalized only as top-level assignment RHS values. | A list/map value has different syntax availability depending on whether it is one-line or multiline. | `SPEC.md`, `README.md`, and `ar_method_parser.md` say multiline lists/maps are assignment-only. Current corpus has 36 top-level multiline literal assignments. | Medium |
| F3 | `if(...)` condition and branch evaluation | Partially composable. Parser tests accept `if(1, 1, 0)`, but `SPEC.md` says the first argument is `<comparison-expression>`. Calls still cannot appear inside branches because calls are not expressions. | Docs disagree. Current evaluator evaluates only the selected branch, while one KB article still says both branches are evaluated. | `ar_condition_instruction_parser.c` parses all three arguments through `ar_expression_parser`; `ar_condition_instruction_evaluator.zig` selects one branch; `ar_condition_instruction_evaluator.md` documents short-circuit behavior; `kb/agerun-language-constraint-workarounds.md` contradicts it. | High |
| F4 | Assignment vs result assignment | Expression assignment is normal only for `memory.path := <expression>`. Function result assignment is encoded inside function instruction AST nodes. | The same storage action is represented and validated through two paths: assignment evaluator vs each function evaluator's result-path handling. | `ar_instruction_ast_t` stores assignment data separately from function-call result paths. Instruction evaluators use `ar_instruction_ast__has_result_assignment()` and `ar_instruction_ast__get_function_result_path()`. | High |
| F5 | `append(...)` target | Syntactically accepts any expression for the target. Semantically only memory-owned lists can mutate. | A list value from `memory.results`, `message.results`, and `[1]` is not interchangeable for mutation. Non-memory targets become no-ops. | `ar_append_instruction_evaluator.md` documents that message/context/fresh/non-list/missing/protected targets are no-ops. Tests cover message-owned, literal, and non-list no-op targets. | Medium |
| F6 | Missing field and empty-list sentinels | Composable as expressions once produced, but sentinel values leak into method logic. | Missing `message.field`, empty `head(...)`, invalid `tail(...)`, failed spawn, and no-op send/spawn all use integer `0` in different roles. | `SPEC.md` documents integer `0` sentinel behavior for `head(...)`, `tail(...)`, `send(0, ...)`, and `spawn(0, ...)`. Tests cover missing message fields for head/tail. | Medium |
| F7 | `memory.self` protection | Protection is consistently enforced for assignment and many result paths, but it is bolted onto instruction semantics. | Write permission depends on target root/path and, for parse, placeholder names/input origin. | `SPEC.md` says method instructions cannot assign or store into `memory.self`; `ar_instruction_ast__has_protected_memory_self_assignment()` supports instruction-level checks; parse evaluator adds placeholder/input-specific checks. | Medium |
| F8 | Operators over containers | One-line literals are composable in expressions, but equality semantics are partial. | Empty list equality exists; non-empty list structural equality does not. Maps are constructible but not structurally comparable. | `ar_expression_evaluator.md` says list equality is limited to empty-list checks and non-empty lists are not structurally compared. | Low |
| F9 | Documentation consistency | Documentation documents real constraints but not under a single principle. Some docs are stale. | Stale docs can make semantics appear less orthogonal than the implementation or hide actual couplings. | `AGENTS.md` and `kb/agerun-method-language-nesting-constraint.md` document non-expression calls. `kb/agerun-language-constraint-workarounds.md` contradicts current lazy `if(...)`. | Medium |

## Detailed Observations

### 1. Function Calls Are the Central Composability Gap

The grammar accepts built-ins through `<function-instruction>`, not `<expression>`. Each built-in
has a specialized parser and instruction AST type. The expression AST supports literals, accessors,
and binary operations only. Because of that split, these natural compositions are not supported:

- `send(0, build("hello {name}", memory))`
- `memory.payload := parse("name={name}", build("name={name}", memory))`
- `memory.ok := if(send(memory.target, message), 1, 0)`
- `memory.items := [head(memory.items), tail(memory.items)]`
- `memory.count_like := head(tail(memory.items))`

The existing method corpus appears to work around this by using intermediate memory slots. The audit
regex found 1,023 assigned built-in call lines and no nested built-in call lines in `.method`
sources. That is strong evidence that the grammar shape controls method style.

Recommended follow-up: add an expression-level call AST and call evaluator only after defining a
side-effect model. Pure calls such as `parse`, `build`, `head`, `tail`, and value-returning
`if(...)` are easier to compose than side-effectful calls such as `send`, `spawn`, `exit`,
`deprecate`, `append`, `compile`, and `complete`.

### 2. Multiline Literals Are a Syntax Convenience, Not a True Expression Form

One-line list and map literals compose through the expression parser. Multiline list and map
literals are recognized by the method parser before instruction parsing and canonicalized only when
the opener is the complete right side of a memory assignment.

That makes formatting affect grammar availability. A list literal value can be used as a function
argument when one-line, but the same value cannot be written multiline in the same position.

Recommended follow-up: either document multiline literals as a source-format shorthand with an
explicit exception to composability, or promote multiline literal parsing into expression parsing
with clear layout rules.

### 3. `if(...)` Has Better Runtime Orthogonality Than Some Docs Say

The current parser parses all three `if(...)` arguments as normal expressions, and tests show
literal integer conditions are accepted. The evaluator checks integer truthiness and evaluates only
the selected branch. This is more orthogonal than the stale KB guidance that says both branches are
evaluated.

However, `SPEC.md` still states the condition grammar as `<comparison-expression>`, and calls cannot
appear inside branches because calls are not expressions. The result is an inconsistent contract:
implementation allows expression conditions, current evaluator is lazy, but some docs describe a
stricter or older semantic model.

Recommended follow-up: update documentation after deciding whether `if(...)` remains a function
instruction or becomes an expression-level conditional. Do not update the spec before that decision,
because the grammar and AST design are the actual source of the larger composability problem.

### 4. Result Assignment Is Duplicated Semantics

Regular assignment stores an evaluated expression into memory. Function result assignment stores a
function result into memory from inside the function instruction evaluator. This duplicate path
means every instruction evaluator that can store a result must repeat result-path validation,
ownership transfer, and protected `memory.self` handling.

Recommended follow-up: once calls are expressions, reduce function result assignment to ordinary
assignment:

- current form: `memory.ok := send(memory.target, memory.payload)`
- intended model: assignment evaluates a call expression and stores the returned value

This would make assignment the single storage operator and reduce semantic drift among instruction
evaluators.

### 5. Mutation Semantics Need an Explicit Lvalue Concept

`append(...)` already reveals an important design boundary. Its target argument parses as a normal
expression, but mutation succeeds only when the expression resolves to a memory-owned list. A
message-owned list, context-owned list, fresh literal list, missing target, non-list, or protected
`memory.self` target becomes a no-op.

That may be the right runtime safety model, but it is not semantically orthogonal if all expressions
are treated as interchangeable values. It needs explicit language wording: some call arguments are
values, while mutation targets are lvalues with memory ownership requirements.

Recommended follow-up: define lvalue semantics before making mutating calls fully expression-level.
That definition should cover `append(...)` now and any future mutating operations.

### 6. Sentinel `0` Is Useful but Leaks Across Components

The language has no null type, so integer `0` appears as:

- missing `message.field`
- empty or invalid `head(...)`
- invalid `tail(...)`
- failed/no-op `spawn(...)`
- `send(0, ...)` no-op destination
- common conditional false value

This is workable, but it couples unrelated components through a shared sentinel. It also makes
method authors choose item domains carefully, such as avoiding integer `0` as a valid list item when
using `head(...)` as a stop condition.

Recommended follow-up: if orthogonality becomes the guiding principle, consider a first-class absent
value, option type, or explicit predicates before adding more sentinel-based built-ins.

## Recommended Follow-Up Order

1. **Documentation correction pass**: fix the stale `if(...)` branch-evaluation KB wording and add
   the composability/orthogonality principle as a language design goal, clearly labeled as goal vs
   current state.
2. **Call-as-expression design note**: decide which built-ins are pure expressions, which are
   effectful expressions, and which must remain statements until evaluation order is formalized.
3. **AST refactor plan**: add `AR_EXPRESSION_AST_TYPE__CALL`, define function metadata once, and
   reduce per-built-in parser duplication.
4. **Assignment unification plan**: make `memory.path := <expression>` the only result-storage
   mechanism and deprecate special result assignment handling inside function instruction nodes.
5. **Multiline expression plan**: choose between documenting assignment-only multiline literals as
   an explicit exception or promoting them into the expression parser.
6. **Sentinel semantics plan**: evaluate whether integer `0` remains the language-wide absent value
   or whether the data model needs an explicit absence representation.

## Acceptance Criteria for Future Implementation

- Function-call parsing has one source of truth for argument splitting and expression nesting.
- Pure built-in calls can appear anywhere an expression can appear.
- Assignment evaluates and stores any expression result through one storage path.
- Mutating calls have explicit lvalue rules instead of hidden origin checks.
- Multiline literal behavior is either composable or documented as a deliberate source-format
  exception.
- Documentation and tests agree on `if(...)` laziness.
- Existing method corpus behavior remains unchanged unless a migration is explicitly planned.

## Verification Performed for This Audit

This report is an evidence artifact. No behavior, tests, or specifications were changed by the
audit itself.

Initial verification commands run after creating the report:

- `make check-docs`: passed; 743 documentation files checked.
- `git diff --check`: passed.
- `make build 2>&1`: passed; 110 tests ran in normal, sanitizer, and thread-sanitizer legs; static
  analysis found no bugs; no memory leaks detected.
- `make check-logs`: passed; no assertion failures, crashes, memory errors, thread safety issues,
  compilation warnings/errors, linker warnings/errors, unexpected method evaluation failures, or
  unwhitelisted error/warning patterns were found.

After the docs-only refinements that added the explicit audit principles and documented constraint
alignment, only lightweight documentation checks were needed:

- `make check-docs`: passed; 743 documentation files checked.
- `git diff --check`: passed.
