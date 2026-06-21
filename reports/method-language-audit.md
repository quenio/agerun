# Method Language Semantic Principles Audit
**Date**: 2026-06-14
**Initial audit base**: `origin/main` at `ae3a064d` (`Merge pull request #28 from quenio/codex/separate-routing-methods`)
**Current revision note**: The current workspace now includes the first cleanup from this audit:
function-call argument splitting and expression-nesting boundary handling are centralized in
`ar_function_call_parser`, and `SPEC.md` names shared function-argument grammar productions. The
spec now also distinguishes boundary-level quote handling from string-literal value semantics:
backslash parity affects whether a quote closes a function-call argument span, but string literal
values preserve source characters and do not decode escape sequences. A follow-up documentation
correction pass now names the language design principles in `SPEC.md`, separates those principles
from the current language state, fixes stale lazy-`if(...)` wording, and removes stale module-doc
claims that implied general escaped-quote or escape-sequence support. The current workspace also
includes the first production pure-call implementations: `parse(...)` and `build(...)` are now
represented by `AR_EXPRESSION_AST_TYPE__CALL`, evaluated as pure expressions through shared
`ar_parse` and `ar_build` semantics, and still supported through their compatibility instruction
paths.
**Focus**: AgeRun method definition grammar, parser architecture, evaluator semantics, documentation, tests, and current method corpus.

## Executive Summary

The current AgeRun method language is only partially composable and only partially orthogonal.

The expression subsystem is the strongest composable core: literals, accessors, parenthesized
expressions, arithmetic, comparison, and one-line list/map literals are parsed into
`ar_expression_ast_t` and evaluated by one expression evaluator. The missing explicit design rule is
that expressions must have no side effects. Under that rule, pure value-producing built-ins should be
composable as expressions, while effectful operations should remain sequenced instructions.

The other governing design rule is line-based parsing and evaluation. Each nonempty method line is
one instruction boundary, and method evaluation proceeds line by line in source order. That keeps
runtime behavior easy to trace and keeps parser/evaluator responsibilities small.

A related rule is syntax-directed semantics: each syntax construct should lead to one evaluation
behavior, and meaningful semantic differences should be visible in syntax. Several current gaps are
places where assignment-looking syntax, expression-looking arguments, or ordinary integer `0` carry
context-dependent behavior.

Today, `parse(...)` and `build(...)` are registered pure built-in calls modeled as expression AST
nodes. They can appear in expression contexts and return new values without mutating runtime state.
The remaining pure candidates, such as `head(...)`, `tail(...)`, lazy `if(...)`, and a redesigned
pure `append(...)`, still cannot be nested as expressions. Closing that remaining set is now the
largest gap against the composability principle.

One parser-level gap has now been closed: top-level function-call argument splitting has one
language owner. `ar_function_call_parser` applies the shared delimiter rule for current instruction
parsers and expression-call parsing, while `SPEC.md` expresses the same rule through reusable
`<function-argument>` productions. This gives both instruction calls and registered pure expression
calls the same argument-boundary rule.

That cleanup also exposed a narrower string-literal contract. The shared function-call scanner is
quote-aware enough to keep commas and closing parentheses inside quoted argument spans, including
spans whose closing quote follows an even number of consecutive backslashes. This is not general
escape-sequence support: the expression parser still treats string literal contents as raw source
characters between delimiters, does not decode backslash escapes, and does not currently specify an
escaped double quote as a string value character.

The main remaining semantic gaps are coupled to syntax and origin. Some value-producing functions
still bind results through instruction-specific storage instead of normal assignment of an
expression result. `append(...)` can accept any target expression syntactically, but mutates only
memory-owned lists. `head(...)`, `tail(...)`, and missing `message` fields rely on integer `0`
sentinels. These choices are useful in the current language, but they are not fully orthogonal
because equivalent-looking values can behave differently depending on where they came from or how
they are used.

The initial audit did not change behavior. This revision records completed documentation, parser,
pure-`parse(...)`, and pure-`build(...)` expression follow-ups and keeps the remaining
recommendations scoped to language behavior that is still unchanged.

## Audit Principles

This audit is guided by these principles:

1. **Line-Based Parsing and Evaluation**: Each nonempty source line should parse to one complete
   instruction, and instructions should evaluate in source order. A line boundary is therefore both a
   parser boundary and an evaluation boundary. This keeps control flow explicit, makes traces easy to
   follow, and avoids hidden multi-statement parsing rules.
2. **Expression Purity**: Expressions must have no side effects. Evaluating an expression should not
   mutate memory, send messages, create agents, deprecate methods, exit agents, complete work, or
   change observable runtime state. This keeps expression evaluation easy to reason about and makes
   nesting, reordering, and reuse safe.
3. **Composability**: Every pure grammar component should be usable in every compatible syntactic
   context. If a pure construct produces a value, it should be eligible wherever expressions are
   accepted unless the language defines a specific exception. This is the lens for questions such as
   whether pure built-in calls can appear inside call arguments, list/map literals, operators,
   assignments, and condition branches.
4. **Orthogonality**: Equivalent values should behave the same regardless of how they were produced.
   A value from a literal, accessor, function result, selected branch, or intermediate assignment
   should not gain hidden semantic differences unless the language defines an explicit distinction
   such as mutation target eligibility.
5. **Single Source of Semantics**: Each language behavior should have one clear owner in the parser,
   AST, evaluator, and documentation. Duplicate paths, such as regular assignment versus
   instruction-specific result assignment, are treated as audit findings because they make behavior
   easier to drift.
6. **Syntax-Directed Semantics**: Each syntax construct should map to one evaluation behavior.
   Semantic differences should be visible in the syntax, not hidden behind origin checks,
   evaluator-specific storage paths, or context-dependent reinterpretation of ordinary values.
7. **Explicit Exceptions**: Any non-composable or non-orthogonal behavior should be intentional,
   documented, and test-covered. Useful restrictions, such as memory-only mutation targets, are not
   automatically wrong, but they must be visible as language rules rather than incidental evaluator
   side effects.
8. **Evidence Before Aspiration**: The audit separates the current language from the desired
   principle. It records what the current commit actually parses, evaluates, documents, and tests
   before recommending future changes.
9. **Compatibility-Aware Evolution**: Recommendations should preserve existing method behavior
   unless a later migration explicitly changes the language contract. Current corpus shape and
   existing tests are part of the design evidence.

## Documented Constraints and Principle Alignment

The current documentation already names several language constraints. This audit treats those
constraints as current-state evidence, then checks whether each constraint aligns with the audit
principles or should be treated as a future design gap.

| Documented Constraint | Documentation Evidence | Principle Alignment | Audit Treatment |
|-----------------------|------------------------|---------------------|-----------------|
| Methods are line-oriented: one instruction per line, no combined instructions, final newline required, empty lines ignored. | `SPEC.md`, `methods/README.md`, `modules/ar_method_parser.md` | Strongly aligns with Line-Based Parsing and Evaluation. It is an intentional instruction-boundary rule that simplifies parsing, evaluation, and traceability. | Preserve as a core language principle. Future expression work should compose inside a line, not erase line boundaries. |
| Standalone expressions are not instructions. Expressions must appear under assignment or allowed function calls. | `SPEC.md` | Aligns with the line-based instruction model because a method line remains an executable instruction, not an implicit expression statement. It still limits statement-level composability. | Preserve unless a future design explicitly adds expression statements without side effects. |
| Registered pure function calls are expressions; effectful built-ins remain instructions. | `SPEC.md`, `AGENTS.md`, `kb/agerun-method-language-nesting-constraint.md`, `modules/ar_expression_ast.h`, `modules/ar_expression_parser.c`, `modules/ar_expression_evaluator.zig`, `modules/ar_parse.md`, `modules/ar_build.md` | Partially aligns with composability and expression purity. `parse(...)` and `build(...)` are now pure expression calls; effectful operations such as `send(...)`, `spawn(...)`, `compile(...)`, `exit(...)`, `deprecate(...)`, and `complete(...)` remain sequenced instructions. The remaining gap is that other pure value-producing candidates are not yet registered pure calls. | Preserve the pure-expression versus effectful-instruction split. Promote the remaining pure candidates, currently `head(...)`, `tail(...)`, lazy `if(...)`, and redesigned pure `append(...)`, without admitting side-effectful operations into expression parsing. |
| Function-call argument boundaries use one shared grammar rule. | `SPEC.md`, `modules/ar_function_call_parser.md`, `modules/ar_function_call_parser_tests.c`, `modules/ar_expression_parser.c` | Aligns with Single Source of Semantics: top-level commas and closing parentheses delimit arguments, while quoted strings, parenthesized expression groups, one-line list/map literals, and registered pure calls are preserved inside an argument. | Preserve as the current baseline for both instruction calls and pure expression calls. Future pure-call-expression work should reuse this boundary rule instead of reintroducing per-call scanners. |
| String literal escaping is boundary-level, not value-level. | `SPEC.md`, `modules/ar_function_call_parser.zig`, `modules/ar_expression_parser.c`, parser tests | Partially aligns now that the distinction is documented. Backslash parity has parser-boundary meaning before quotes in function-call arguments, but the expression parser preserves backslashes as ordinary characters and does not decode escape sequences. | Do not describe this as general escaped-character support. Preserve raw string literals unless true expression-level escape parsing is added as an explicit language change. |
| Function result storage uses both ordinary assignment and instruction result binding. | `SPEC.md`, `modules/ar_instruction_ast.md`, `modules/ar_instruction_parser.md`, evaluator module docs | Partially conflicts with Syntax-Directed Semantics because ordinary assignment and instruction result binding look similar while using different AST/evaluator paths. `parse(...)` and `build(...)` now have ordinary expression assignment semantics, and their standalone compatibility instruction forms do not create result paths. Remaining instruction-result functions such as `head(...)`, `tail(...)`, `send(...)`, `if(...)`, `complete(...)`, `append(...)`, `compile(...)`, `spawn(...)`, `deprecate(...)`, and `exit(...)` still use function-result paths when assigned. | Keep pure call results on the expression-assignment path as each pure candidate is promoted, and centralize or visibly distinguish any remaining effectful or compatibility result binding. |
| One-line list/map literals can appear in expression contexts; multiline list/map literals are assignment-only and cannot appear as call arguments, list items, or map values. | `SPEC.md`, `README.md`, `modules/ar_method_parser.md` | Partially aligns. One-line literals are composable; multiline literals are an explicit formatting exception to the one-line instruction model. | Either document multiline literals as a deliberate source-format exception or move them into expression parsing while preserving clear line-boundary rules. |
| Map literal keys must be identifiers; quoted keys are not supported. | `SPEC.md`, `modules/ar_expression_parser.md` | Mostly compatible as an explicit grammar restriction. It limits data shape expressiveness but does not by itself create semantic drift. | Preserve unless future data requirements need arbitrary string keys. |
| There is no null type; integer `0` is used as the absent/failure/no-op sentinel in several places. | `AGENTS.md`, `SPEC.md`, `modules/ar_expression_evaluator.md` | Conflicts with orthogonality when unrelated cases share the same value: missing field, empty `head(...)`, invalid `tail(...)` input, no-op spawn, false condition. | Preserve existing `0` behavior for compatibility, but stop treating it as a default pattern for new features. Before adding another `0`-based case, decide whether the language should keep `0` as the official absence/no-op value, add explicit predicates such as "is missing" or "is empty", or introduce a distinct absence value. |
| No static type checking; methods must handle possible runtime types defensively. | `AGENTS.md`, `kb/agerun-language-constraint-workarounds.md` | Acceptable as a dynamic-language constraint, but it raises the burden on orthogonal runtime behavior and diagnostics. | Preserve as current-state behavior; improve documentation and tests around type-dependent built-ins. |
| `if(...)` is documented as lazy value selection: evaluate the condition, then evaluate and return only the selected branch expression. | `SPEC.md`, `modules/ar_condition_instruction_evaluator.md`, `kb/agerun-language-constraint-workarounds.md` | Aligns with Single Source of Semantics for current instruction-level conditionals. Selected-branch evaluation fits conditional value selection; expression purity means branch evaluation must not become a place to hide side effects. | Preserve the lazy selected-branch rule. If `if(...)` later becomes an expression-level conditional, keep it pure and lazy. |
| No conditional execution statement exists; all method instructions execute in order and conditional behavior is encoded with value selection and no-op targets. | `kb/agerun-language-constraint-workarounds.md`, coordination method patterns | Aligns with line-based sequential evaluation and pure expressions, but it couples conditional side effects to sentinel/no-op instruction behavior instead of explicit control flow. | Keep in scope for the line-based, pure-expression, sequenced-instruction, and sentinel semantics follow-up plans. |
| `send(0, message)` is a no-op; `spawn(0, ...)` and `spawn("", ...)` are no-ops returning/storing `0`. | `AGENTS.md`, `SPEC.md`, `kb/no-op-semantics-pattern.md`, `kb/no-op-instruction-semantics.md` | Aligns with Explicit Exceptions when documented and with Expression Purity when kept outside expressions. It still assigns special instruction semantics to ordinary integer/string values. | Preserve the current no-op rules, but document them as instruction-specific exceptions: in `send(...)`, `0` means "no destination, do not send"; in `spawn(...)`, `0` or `""` means "do not spawn and return/store `0`". Do not imply that integer `0` has this no-op meaning in ordinary expression evaluation or ordinary data values. |
| `append(target, value)` accepts any target expression syntactically, but only mutates an existing memory-owned list; message/context/fresh/missing/non-list/protected targets are no-ops. | `SPEC.md`, `modules/ar_append_instruction_evaluator.md` | Explicit but not fully orthogonal: identical list values differ by origin and ownership. The expression-looking target hides lvalue semantics unless the root/path syntax is treated as the mutation marker. | Change `append(...)` into a pure list-construction expression that returns a new list, and migrate or rename the current mutating form as an explicit effectful instruction only if compatibility requires it. |
| Writes are limited to `memory` paths, and `memory.self` plus nested `memory.self.*` are protected from assignment/result storage. | `SPEC.md`, `modules/ar_instruction_ast.md`, `modules/ar_assignment_instruction_evaluator.md`, `modules/ar_parse.md`, `modules/ar_build.md`, parse/build evaluator docs | Aligns with Explicit Exceptions and runtime identity safety. The `memory` root visibly marks writable state. `parse(...)` and `build(...)` arguments are now path-neutral; protected identity behavior belongs to storage rules, not pure-call argument names or paths. | Preserve and centralize the storage rule so assignment, compatibility function results, and future merge-like operations cannot drift. Do not reintroduce path-specific argument restrictions for pure calls. |
| `head(...)` and `tail(...)` use deep-copy list traversal, but their sentinel cases differ. `head(...)` stores integer `0` for empty, missing, non-list, or copy-failure inputs; `tail(...)` stores a new empty LIST for empty or single-item lists and stores integer `0` only for missing, non-list, or copy-failure inputs. | `SPEC.md`, `modules/ar_head_instruction_evaluator.md`, `modules/ar_tail_instruction_evaluator.md`, `modules/ar_expression_evaluator.md` | Partially aligns: list values are safely copied, and `tail(...)` distinguishes valid empty tails from invalid input. `head(...)` still uses integer `0` for both empty input and invalid input, so that sentinel remains coupled to valid integer payloads. | Keep as current behavior; include the asymmetric `head(...)`/`tail(...)` sentinel rules in sentinel semantics planning. |
| Non-empty list equality and map structural equality are not supported; list equality is limited to empty-list checks. | `modules/ar_expression_evaluator.md` | Limits orthogonality and syntax-directed semantics. The same `=` syntax changes behavior based on container shape and content. | Treat as a lower-risk expression semantics gap. |

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
- Applied five separate lenses:
  - Line-based parsing and evaluation: whether a construct preserves one instruction per source
    line and source-order evaluation.
  - Expression purity: whether a construct can be evaluated without side effects.
  - Syntax-directed semantics: whether syntax clearly selects one evaluation behavior.
  - Composability: where each pure grammar component can appear and combine.
  - Orthogonality: whether equivalent values behave the same regardless of syntactic form or origin.

## Key Findings

| ID | Area | Composability Status | Orthogonality Status | Evidence | Risk |
|----|------|----------------------|----------------------|----------|------|
| F0 | Line-based parsing and evaluation | Statement-level composition is deliberately limited: each nonempty line is one instruction. Pure expression composition should happen inside that instruction boundary. | Evaluation order is explicit and source-ordered, which reduces hidden semantic coupling across lines. | `SPEC.md`, `methods/README.md`, and `ar_method_parser.md` document one instruction per line, no combined instructions, final newline requirement, and ignored empty lines. | Low |
| F1 | Built-in calls | Partially composable. `parse(...)` and `build(...)` are registered pure expression calls and can appear wherever expressions are accepted. The remaining pure candidates are still top-level function instructions. | `parse(...)` and `build(...)` results can flow through ordinary expression assignment, while remaining result-producing instruction calls still use instruction-specific result paths. Effectful built-ins are correctly kept out of expressions if expression purity is a hard rule. | `SPEC.md` includes `<pure-function-call>`, `ar_expression_ast_t` has `AR_EXPRESSION_AST_TYPE__CALL`, `ar_expression_parser.c` registers `parse` and `build`, and `ar_expression_evaluator.zig` dispatches pure calls to `ar_parse` and `ar_build`. | Medium |
| F1a | Function-call argument boundaries | Argument splitting is now consistent across instruction parsers and pure expression-call parsing. It preserves nested expression syntax inside one argument and then requires the argument to parse as an expression. | The boundary rule is no longer duplicated across built-in parsers. Arity and instruction-specific semantics remain per call, while pure-call arity is registered for expression parsing. | `SPEC.md` defines shared `<function-argument>` productions. `ar_function_call_parser` owns splitting and argument AST-list creation for C and Zig instruction parsers, and `ar_expression_parser.c` uses the same boundary behavior for registered pure calls. | Low |
| F1b | Quote and escape handling | Function-call boundary parsing is quote-aware, including even/odd backslash parity before quotes. Expression string parsing remains a simple raw span between delimiters. | Backslash has context-dependent meaning: it can keep a quote from closing an argument span, but it is preserved as data and is not decoded by expression evaluation. Escaped quotes are not currently string value characters. | `SPEC.md` and parser module docs document the split. `_isQuote` in `ar_function_call_parser.zig` counts consecutive backslashes before quotes. `ar_expression_parser.c` copies bytes between the opening quote and the next quote. | Low |
| F2 | Multiline list/map literals | Not composable. Multiline literals are canonicalized only as top-level assignment RHS values. | A list/map value has different syntax availability depending on whether it is one-line or multiline. | `SPEC.md`, `README.md`, and `ar_method_parser.md` say multiline lists/maps are assignment-only. Current corpus has 36 top-level multiline literal assignments. | Medium |
| F3 | `if(...)` condition and branch evaluation | Partially composable. `SPEC.md` now states that the first argument is an `<expression>`, parser tests accept `if(1, 1, 0)`, and registered pure calls such as `parse(...)` and `build(...)` can appear in branch expressions. `if(...)` itself is not yet an expression call. | Docs now agree that the evaluator checks the condition first, evaluates only the selected branch, treats integer `0` as false, treats non-zero integers as true, and sends non-integer condition values to the false branch. That is more orthogonal than the earlier stale both-branches wording. | `ar_condition_instruction_parser.c` parses all three arguments through `ar_expression_parser`; `ar_condition_instruction_evaluator.zig` selects one branch; `SPEC.md`, `modules/README.md`, `ar_condition_instruction_evaluator.md`, and `kb/agerun-language-constraint-workarounds.md` document selected-branch evaluation. | Medium |
| F4 | Assignment vs result assignment | Expression assignment is normal for `memory.path := <expression>`, including pure `parse(...)` and `build(...)` calls. Function result assignment still exists for remaining instruction-result functions such as `head(...)`, `tail(...)`, and effectful result-binding instructions. | Pure expression results and effectful or compatibility instruction results are still represented through overlapping storage paths. The syntax may remain compact, but storage validation should have one owner. | `ar_instruction_ast_t` stores assignment data separately from function-call result paths. The instruction parser now leaves assigned `parse(...)` and `build(...)` calls to assignment parsing, while `_is_instruction_result_function()` still routes assigned `head(...)`, `tail(...)`, `send(...)`, `if(...)`, `complete(...)`, `append(...)`, `compile(...)`, `spawn(...)`, `deprecate(...)`, and `exit(...)` through function-call result paths. | Medium |
| F5 | `append(...)` target | Syntactically accepts any expression for the target. Semantically only memory-owned lists can mutate. | A list value from `memory.results`, `message.results`, and `[1]` is not interchangeable for mutation. This is the strongest candidate for redesign as a pure value-producing list operation. | `ar_append_instruction_evaluator.md` documents that message/context/fresh/non-list/missing/protected targets are no-ops. Tests cover message-owned, literal, and non-list no-op targets. | Medium |
| F6 | Missing field and empty-list sentinels | Composable as expressions once produced, but sentinel values leak into method logic. | Missing `message.field`, empty `head(...)`, invalid `tail(...)`, failed spawn, and no-op send/spawn all use integer `0` in different roles. | `SPEC.md` documents integer `0` sentinel behavior for `head(...)`, `tail(...)`, `send(0, ...)`, and `spawn(0, ...)`. Tests cover missing message fields for head/tail. | Medium |
| F7 | `memory.self` protection | Protection is consistently enforced for assignment and many result paths, and pure `parse(...)`/`build(...)` no longer treat `self` names or `memory.self` argument paths specially. | Write permission depends on target root/path. Pure call argument handling is now value-based and path-neutral; protected identity behavior belongs to assignment and result-storage rules. | `SPEC.md` says protected identity behavior is enforced by storage rules, `ar_instruction_ast__has_protected_memory_self_assignment()` supports instruction-level checks, and `ar_parse.md`/`ar_build.md` document path-neutral argument handling. | Low |
| F8 | Operators over containers | One-line literals are composable in expressions, but equality semantics are partial. | Empty list equality exists; non-empty list structural equality does not. Maps are constructible but not structurally comparable. | `ar_expression_evaluator.md` says list equality is limited to empty-list checks and non-empty lists are not structurally compared. | Low |
| F9 | Documentation consistency | `SPEC.md` now names the language design principles, separates them from current language state, and documents pure `parse(...)` and `build(...)` expression calls. The earlier stale `if(...)`, string-escape, and pure-call wording has been corrected in the directly affected docs. | The remaining risk is future drift across the spec, README, KB, parser docs, module index, and this audit report as language behavior evolves. | `SPEC.md` documents `Language Design Principles`, `Current Language State`, `<pure-function-call>`, `parse(template: data, input: data) -> map`, and `build(template: data, values: data) -> string`. `AGENTS.md` and `kb/agerun-method-language-nesting-constraint.md` document registered pure calls as expressions. | Low |
| F10 | Syntax-directed semantics | Several constructs use one syntax shape while dispatching to context-dependent evaluation behavior. | Semantic differences are sometimes hidden behind evaluator checks rather than syntax: result binding, lvalue eligibility, sentinel `0`, and partial equality. | Assignment-looking function results, `append(...)` targets, no-op `0` semantics, and container equality restrictions. | Medium |

## Detailed Observations

### 1. Line Boundaries Are an Intentional Semantic Boundary

The current language uses line boundaries as instruction boundaries. That is more than a parser
shortcut: it also makes evaluation order explicit. A method can be read from top to bottom as a
sequence of instructions, with pure expressions evaluated inside each instruction rather than as
separate hidden steps.

This principle changes how composability should be applied. Expression composition should become
richer inside a single instruction, but the language does not need to make multiple instructions
share one line, introduce implicit expression statements, or hide side effects inside nested
expressions.

Current documentation baseline: `SPEC.md` now names Line-Based Parsing and Evaluation beside
Expression Purity and the other language design principles, then separately records the current
language state. Multiline literals remain documented as an assignment-only source-format exception
whose parser behavior must preserve clear instruction boundaries.

### 2. Syntax Should Reveal Evaluation Behavior

The language is easiest to reason about when syntax tells the reader which evaluator behavior will
run. This is the natural extension of Single Source of Semantics: not only should each behavior have
one owner, each syntax construct should select one behavior clearly.

The current language already does this in useful places. A `memory.*` path visibly identifies
writable state, and one method line visibly identifies one instruction. The less clear cases are
where one syntactic form hides multiple evaluation paths:

- `memory.value := build(template, values)` is ordinary assignment, while
  `memory.value := head(memory.items)` still uses compatibility function-instruction result storage.
- `append(memory.items, value)` and `append(message.items, value)` use the same expression-looking
  target position, but only the `memory` target can mutate.
- Integer `0` can mean ordinary data, condition false, missing value, empty `head(...)`, invalid
  `tail(...)`, failed/no-op `spawn(...)`, or no-op `send(...)` destination.
- The `=` operator has special empty-list equality behavior but not general list/map structural
  equality.

Recommended follow-up: when a semantic distinction is intentional, either make it visible in syntax
or document it as an explicit exception. Avoid adding new constructs where the evaluator has to
infer a different behavior from hidden origin, ownership, or data-shape checks.

### 3. Pure Function Calls Remain the Central Composability Gap

The grammar now has a narrow expression-call path. `parse(...)` and `build(...)` are registered pure
function calls, represented by `AR_EXPRESSION_AST_TYPE__CALL`, parsed through the shared
function-call boundary rule, and evaluated by `ar_expression_evaluator` through shared `ar_parse`
and `ar_build` semantics. `parse(...)` returns a new map, and `build(...)` returns a new string.
Both use path-neutral argument handling and reserve protected `memory.self` behavior for assignment
and result-storage rules.

That closes the first two production slices of the pure-call gap, but it does not make all
value-producing built-ins composable. The remaining candidates still go through
`<function-instruction>` and instruction AST/evaluator paths, so these pure value compositions are
still not supported:

- `memory.items := [head(memory.items), tail(memory.items)]`
- `memory.count_like := head(tail(memory.items))`
- `memory.values := {item: head(memory.items)}`
- `memory.labels := [if(memory.enabled = 1, "ready", "blocked")]`

By contrast, `parse(...)` and `build(...)` are now valid in expression contexts such as assignment
RHS values, instruction arguments, list/map literal values, and selected `if(...)` branch
expressions when their arguments evaluate to values that the shared pure operation can reasonably
interpret.

The existing method corpus appears to work around non-composable calls by using intermediate memory
slots. The audit regex found 1,023 assigned built-in call lines and no nested built-in call lines in
`.method` sources at the initial audit base. That is strong evidence that the grammar shape controls
method style.

Recommended follow-up: extend the existing pure-call AST/evaluator path only for calls classified as
pure. The classification should remain explicit:

| Built-in | Recommendation | Rationale |
|----------|----------------|-----------|
| `parse(template, input)` | Completed as a pure expression and shared value operation. | It produces a new map from argument values and does not mutate runtime state. `STRING`, `INTEGER`, and `DOUBLE` arguments are interpreted as strings; malformed templates, non-matching input, missing values, `LIST`, `MAP`, or otherwise unsuitable values return an empty map. Placeholder names and argument paths are path-neutral, so `self` and nested `self.*` fields may appear in the returned map when stored outside protected paths. Protected `memory.self` writes remain storage-rule failures, not parse-argument failures. |
| `build(template, values)` | Completed as a pure expression and shared value operation. | It constructs a new string from argument values and does not mutate runtime state. `STRING`, `INTEGER`, and `DOUBLE` templates are interpreted as strings; missing or non-primitive templates use an empty string fallback. When `values` is not a map, placeholders remain unchanged. Missing placeholders and non-primitive placeholder values also remain unchanged. The top-level build instruction preserves its compatibility contract while delegating value-level building to `ar_build`. |
| `head(list)` | Promote to pure expression. | It returns an independent copy of the first item or integer `0` and never mutates the source list. |
| `tail(list)` | Promote to pure expression. | It returns a new list or integer `0` and never mutates the source list. |
| `if(condition, true_value, false_value)` | Promote to a pure lazy expression. | It is value selection, not control flow; only the selected branch should be evaluated. |
| `append(list, value)` | Redesign as a pure expression. | It should return a new list with the value appended, or `0` for invalid/non-list/copy-failure inputs, instead of mutating only memory-owned lists. |
| `send(recipient_id, message)` | Keep as an instruction. | It enqueues messages, routes to agents or delegates, and transfers message ownership. |
| `compile(method_name, instructions, version)` | Keep as an instruction. | It registers a method in the methodology. |
| `spawn(method_name, version, context)` | Keep as an instruction. | It creates a runtime agent and returns a new agent ID. |
| `exit(agent_id)` | Keep as an instruction. | It destroys an agent. |
| `deprecate(method_name, method_version)` | Keep as an instruction. | It unregisters or deprecates a method version. |
| `complete(template[, values])` | Keep as an instruction. | It performs local completion work, may initialize or reuse backend/model state, can time out or depend on configured runtime resources, and is explicitly outside expression purity. |

The expression grammar should continue rejecting instruction-only built-ins. If an effectful
instruction still needs to bind a result, that result-binding rule should remain visibly
statement-level rather than borrowing ordinary expression assignment semantics.

### 4. String Literal Escaping Is Boundary-Level Today

The current implementation has two different quote-related behaviors:

- Function-call argument boundary parsing treats a double quote as a quote delimiter only when it is
  preceded by zero or an even number of consecutive backslashes. This keeps calls such as
  `build("C:\\", memory.data)` from being rejected as unterminated during argument splitting.
- Expression string literal parsing still copies the raw source characters between the opening
  delimiter and the next double quote. Backslashes are preserved; they are not decoded into escape
  sequences, and `\"` is not currently a specified way to include a double quote in a string value.

This means "escaping" is currently a parser-boundary rule, not a value-level string semantics
rule. That distinction matters for documentation and future expression-call work. If the language
keeps raw string literals, module docs should keep avoiding general escaped-quote or
escape-sequence support claims. If the language wants escaped characters in values, that should be
added as an explicit expression-parser/evaluator feature with tests for raw value contents.

Recommended follow-up: decide whether AgeRun string literals remain raw-delimited strings or gain
defined escape sequences. Until then, preserve the current boundary behavior and avoid treating
backslash parity before quotes as a general string escape contract.

### 5. Multiline Literals Are a Syntax Convenience, Not a True Expression Form

One-line list and map literals compose through the expression parser. Multiline list and map
literals are recognized by the method parser before instruction parsing and canonicalized only when
the opener is the complete right side of a memory assignment.

That makes formatting affect grammar availability. A list literal value can be used as a function
argument when one-line, but the same value cannot be written multiline in the same position.

Recommended follow-up: either document multiline literals as a source-format shorthand with an
explicit exception to composability, or promote multiline literal parsing into expression parsing
with clear layout rules.

### 6. `if(...)` Has Better Runtime Orthogonality Than Some Docs Say

The current parser parses all three `if(...)` arguments as normal expressions, and tests show
literal integer conditions are accepted. `SPEC.md` now reflects that condition grammar. The evaluator
checks integer truthiness and evaluates only the selected branch, and the KB/module docs now match
that selected-branch model.

Calls still cannot appear inside branches because calls are not expressions. If a future language
change makes `if(...)` an expression-level conditional, it should remain pure and lazy: only the
selected branch should be evaluated, and neither branch should be able to perform effects through
expression evaluation.

### 7. Result Assignment Is Duplicated Semantics

Regular assignment stores an evaluated expression into memory. `parse(...)` and `build(...)` now
participate in that path when used as pure expressions, and their standalone compatibility
instruction forms do not create result-storage paths. The remaining result-producing function
instructions still store results from inside instruction evaluators, so that duplicate path has not
been eliminated yet. Every instruction evaluator that can store a result must still repeat or
delegate result-path validation, ownership transfer, and protected `memory.self` handling.

Recommended follow-up: as each remaining pure candidate becomes an expression, reduce its result
storage to ordinary assignment:

- current compatibility head/tail form: `memory.value := head(memory.items)`
- intended pure-expression model: assignment evaluates a pure call expression and stores the result

Effectful instructions that return values, such as a future `spawn(...)` result binding, may still
need statement-level result storage. If so, that storage rule should be centralized and explicitly
separate from expression assignment rather than repeated in each evaluator.

### 8. `append(...)` Should Become Pure List Construction

`append(...)` already reveals an important design boundary. Its target argument parses as a normal
expression, but mutation succeeds only when the expression resolves to a memory-owned list. A
message-owned list, context-owned list, fresh literal list, missing target, non-list, or protected
`memory.self` target becomes a no-op.

That current runtime safety model is useful, but it is not semantically orthogonal if all expressions
are treated as interchangeable values. The better language direction is to make `append(...)` a pure
list-construction function:

- `memory.items := append(memory.items, message.value)` should evaluate a list value and return a new
  list with `message.value` appended.
- `memory.items := append(message.items, message.value)` should behave the same for an equivalent
  message-owned list because ownership origin should not affect pure value construction.
- Invalid, missing, non-list, or not-copyable inputs should return the language's chosen failure value
  for list operations, currently integer `0`.

If compatibility requires preserving the current memory-mutating behavior, it should be renamed or
documented as a separate effectful instruction with explicit lvalue semantics. The unqualified
`append(...)` name should move toward pure expression semantics because list append is naturally a
value transformation.

### 9. Sentinel `0` Is Useful but Leaks Across Components

The language has no null type, so integer `0` appears as:

- missing `message.field`
- empty or invalid `head(...)`
- missing, non-LIST, or copy-failed `tail(...)` input
- failed/no-op `spawn(...)`
- `send(0, ...)` no-op destination
- common conditional false value

This is workable as current behavior, but it couples unrelated components through a shared sentinel.
It also makes method authors choose item domains carefully, such as avoiding integer `0` as a valid
list item when using `head(...)` as a stop condition.

The audit treatment is not "change all existing `0` behavior now." It is a design warning for future
language work: avoid adding more feature-specific meanings to `0` until the language explicitly
chooses one absence/no-op model. The follow-up decision should be one of:

- Keep integer `0` as the official absence/no-op value and document that contract centrally.
- Add explicit predicates, such as "is missing" or "is empty", so methods do not rely only on value
  equality with `0`.
- Introduce a distinct absence value if `0` needs to remain ordinary integer data.

Recommended follow-up: make that choice before adding more sentinel-based built-ins, then update the
grammar, evaluator docs, and method guidance consistently.

For the existing `send(...)` and `spawn(...)` no-op rules, the important classification is scope.
Those rules are instruction-specific exceptions: `0` is interpreted specially only in the documented
instruction argument or result position. In ordinary expression evaluation and ordinary data storage,
`0` remains just the integer value `0`.

## Remaining Recommended Follow-Up Order

1. **Pure `head(...)` and `tail(...)` expression implementation**: move list-decomposition calls into
   the pure-call path while preserving copy semantics and the current asymmetric sentinel behavior
   unless the sentinel plan changes it first.
2. **Lazy `if(...)` expression implementation**: promote `if(condition, true_value, false_value)` into
   a pure lazy expression call where only the selected branch is evaluated, while preserving current
   line-based instruction evaluation for method statements.
3. **Pure `append(...)` redesign**: introduce `append(list, value)` as pure list construction that
   returns a new list, and either migrate or explicitly separate the current memory-mutating
   instruction form.
4. **Pure-call metadata and result-binding consolidation**: define pure-call metadata once, reduce
   parser/evaluator duplication, make `memory.path := <expression>` the only pure-expression storage
   mechanism, and centralize any statement-level result binding still needed by effectful or
   compatibility instructions.
5. **Multiline expression plan**: choose between documenting assignment-only multiline literals as
   an explicit exception or promoting them into the expression parser.
6. **Sentinel semantics plan**: evaluate whether integer `0` remains the language-wide absent value
   or whether the data model needs an explicit absence representation.

## Current Baseline Now Satisfied

- Function-call argument splitting and expression-nesting boundary handling have one source of truth
  in `ar_function_call_parser`.
- `SPEC.md` names shared function-argument productions instead of spelling each built-in argument
  list independently.
- `SPEC.md` now documents that quote/backslash handling for function-call argument splitting is a
  boundary rule, while string literal values preserve source characters and do not decode escapes.
- `SPEC.md` now has separate `Language Design Principles` and `Current Language State` sections.
- `SPEC.md`, `kb/agerun-language-constraint-workarounds.md`, `modules/README.md`, and
  `modules/ar_condition_instruction_evaluator.md` now agree on lazy selected-branch `if(...)`
  evaluation and the current non-integer-falls-through-false condition behavior.
- `README.md` and `modules/README.md` now avoid invalid branch-side assignment/call examples and
  stale string-truthiness/conditional-execution wording.
- Parser module docs now describe quote/backslash handling as function-call boundary parsing rather
  than value-level escaped-quote or escape-sequence support.
- `AR_EXPRESSION_AST_TYPE__CALL` exists for pure function calls.
- `parse(...)` is the first registered pure expression call and can compose anywhere expressions are
  accepted.
- `ar_parse` owns the shared pure parse semantics used by expression evaluation and parse instruction
  evaluation.
- `parse(...)` argument handling is path-neutral; `self`, nested `self.*`, and `memory.self` paths are
  not rejected by name. Protected `memory.self` behavior is enforced by assignment and result-storage
  rules.
- `build(...)` is the second registered pure expression call and can compose anywhere expressions are
  accepted.
- `ar_build` owns the shared pure build semantics used by expression evaluation and build instruction
  evaluation.
- `build(...)` argument handling is path-neutral; non-string-compatible templates fall back to an
  empty string, non-map values leave placeholders unchanged, and protected `memory.self` behavior is
  enforced only by assignment and result-storage rules.

## Acceptance Criteria for Remaining Future Implementation

- Each nonempty source line remains one complete instruction boundary.
- Method evaluation remains source-ordered and line-based.
- Expression evaluation is side-effect free by grammar and by evaluator contract.
- Each syntax construct maps to one documented evaluation behavior.
- Semantic differences are visible in syntax or documented as explicit exceptions.
- Registered pure built-in calls can appear anywhere an expression can appear.
- Each newly promoted pure built-in call uses value-based argument semantics and returns the most
  reasonable value possible instead of failing ordinary expression evaluation.
- Side-effectful built-ins remain explicit sequenced instructions.
- Assignment evaluates and stores pure expression results through one storage path.
- `append(...)` is either a pure list-construction expression or the mutating compatibility form is
  renamed/documented as an explicit effectful instruction with lvalue rules.
- Multiline literal behavior is either composable or documented as a deliberate source-format
  exception.
- Documentation and tests agree on `if(...)` laziness.
- Documentation and tests agree on whether string literals are raw strings or define escape
  sequences.
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

After the docs-only refinements that added the explicit audit principles, documented constraint
alignment, expression-purity guidance, line-based parsing/evaluation guidance, and
syntax-directed-semantics guidance, only lightweight documentation checks were needed:

- `make check-docs`: passed; 743 documentation files checked.
- `git diff --check`: passed.

After the function-call parser cleanup and SPEC grammar update, this report was revised to separate
the now-satisfied shared argument-boundary baseline from the remaining pure-call-expression work:

- `make check-docs`: passed; 746 documentation files checked.
- `git diff --check reports/method-language-audit.md`: passed.

After the quote/backslash SPEC clarification, this report was revised to distinguish boundary-level
quote handling from raw string-literal value semantics:

- `make check-docs`: passed; 746 documentation files checked.
- `git diff --check reports/method-language-audit.md`: passed.

After the documentation correction pass that split `SPEC.md` language principles from current
language state, fixed stale lazy-`if(...)` wording, and aligned parser docs with raw string values
and boundary-only quote/backslash handling, this report was revised to reflect that completed
baseline:

- `make check-docs`: passed; 746 documentation files checked.
- `git diff --check`: passed.

After the user-facing `if(...)` documentation cleanup that removed an invalid README example and
stale module-index condition wording, this report was revised to include those docs in the current
baseline:

- `make check-docs`: passed; 746 documentation files checked.
- `git diff --check`: passed.

After review identified that the README counter example still relied on uninitialized memory fields
and the condition evaluator docs overstated non-integer rejection, this report was revised to record
the initialized example and current non-integer-falls-through-false behavior:

- `make check-docs`: passed; 746 documentation files checked.
- `git diff --check`: passed.

After the pure built-in classification update, this report was revised to recommend promoting
`parse(...)`, `build(...)`, `head(...)`, `tail(...)`, lazy `if(...)`, and redesigned pure
`append(...)` into expression calls while keeping `send(...)`, `compile(...)`, `spawn(...)`,
`exit(...)`, `deprecate(...)`, and `complete(...)` as sequenced instructions:

- `make check-docs`: passed; 746 documentation files checked.
- `git diff --check reports/method-language-audit.md`: passed.

After pure `parse(...)` expressions landed on `origin/main`, this report was revised to mark the
`parse(...)` slice and expression-call AST baseline as satisfied, to record path-neutral
`parse(...)` argument handling, and to recut the remaining follow-up order around the still
unimplemented pure candidates:

- `make check-docs`: passed; 748 documentation files checked.
- `git diff --check reports/method-language-audit.md`: passed.

After pure `build(...)` expressions landed on `origin/main`, this report was revised to mark the
`build(...)` slice and shared `ar_build` baseline as satisfied, to record path-neutral
`build(...)` argument handling and fallback behavior, and to make pure `head(...)`/`tail(...)`
expressions the next recommended follow-up:

- `make check-docs`: passed; 750 documentation files checked.
- `git diff --check reports/method-language-audit.md`: passed.
