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
claims that implied general escaped-quote or escape-sequence support.
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

Today, all built-in calls are modeled as instruction AST nodes rather than expression AST nodes, so
even pure calls cannot be nested inside expressions, operators, list items, map values, or other call
arguments. This is the largest gap against the composability principle.

One parser-level gap has now been closed: top-level function-call argument splitting has one
language owner. `ar_function_call_parser` applies the shared delimiter rule for all current C and
Zig instruction parsers, while `SPEC.md` expresses the same rule through reusable
`<function-argument>` productions. This does not make calls into expressions; it only makes the
current instruction-call argument boundary consistent.

That cleanup also exposed a narrower string-literal contract. The shared function-call scanner is
quote-aware enough to keep commas and closing parentheses inside quoted argument spans, including
spans whose closing quote follows an even number of consecutive backslashes. This is not general
escape-sequence support: the expression parser still treats string literal contents as raw source
characters between delimiters, does not decode backslash escapes, and does not currently specify an
escaped double quote as a string value character.

The main semantic gaps are coupled to syntax and origin. Pure function result storage is currently
mixed with instruction result storage instead of normal assignment of an expression result.
`append(...)` can accept any target expression syntactically, but mutates only memory-owned lists.
`head(...)`, `tail(...)`, and missing `message` fields rely on integer `0` sentinels. These choices
are useful in the current language, but they are not fully orthogonal because equivalent-looking
values can behave differently depending on where they came from or how they are used.

The initial audit did not change behavior. This revision records completed documentation and parser
follow-ups and keeps the remaining recommendations scoped to language behavior that is still
unchanged.

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
| Function calls are documented as instructions, not expressions; nested calls are forbidden. | `SPEC.md`, `AGENTS.md`, `kb/agerun-method-language-nesting-constraint.md`, disabled parser tests | Conflicts with composability for pure value-producing calls, but protects expression purity for effectful operations such as `send(...)`, `spawn(...)`, and `append(...)`. Syntax should make the pure-expression versus effectful-instruction split clear. | Treat pure call composition as the primary gap; keep side-effectful operations sequenced as instructions. |
| Function-call argument boundaries use one shared grammar rule. | `SPEC.md`, `modules/ar_function_call_parser.md`, `modules/ar_function_call_parser_tests.c` | Aligns with Single Source of Semantics: top-level commas and closing parentheses delimit arguments, while quoted strings, parenthesized expression groups, and one-line list/map literals are preserved inside an argument. | Preserve as the current baseline. Future pure-call-expression work should reuse this boundary rule instead of reintroducing per-call scanners. |
| String literal escaping is boundary-level, not value-level. | `SPEC.md`, `modules/ar_function_call_parser.zig`, `modules/ar_expression_parser.c`, parser tests | Partially aligns now that the distinction is documented. Backslash parity has parser-boundary meaning before quotes in function-call arguments, but the expression parser preserves backslashes as ordinary characters and does not decode escape sequences. | Do not describe this as general escaped-character support. Preserve raw string literals unless true expression-level escape parsing is added as an explicit language change. |
| Function result storage uses assignment-looking syntax on function instructions. | `SPEC.md`, `modules/ar_instruction_ast.md`, evaluator module docs | Conflicts with Syntax-Directed Semantics because ordinary assignment and instruction result binding look similar while using different AST/evaluator paths. | Move pure call results into expression assignment, and centralize or visibly distinguish any remaining effectful result binding. |
| One-line list/map literals can appear in expression contexts; multiline list/map literals are assignment-only and cannot appear as call arguments, list items, or map values. | `SPEC.md`, `README.md`, `modules/ar_method_parser.md` | Partially aligns. One-line literals are composable; multiline literals are an explicit formatting exception to the one-line instruction model. | Either document multiline literals as a deliberate source-format exception or move them into expression parsing while preserving clear line-boundary rules. |
| Map literal keys must be identifiers; quoted keys are not supported. | `SPEC.md`, `modules/ar_expression_parser.md` | Mostly compatible as an explicit grammar restriction. It limits data shape expressiveness but does not by itself create semantic drift. | Preserve unless future data requirements need arbitrary string keys. |
| There is no null type; integer `0` is used as the absent/failure/no-op sentinel in several places. | `AGENTS.md`, `SPEC.md`, `modules/ar_expression_evaluator.md` | Conflicts with orthogonality when unrelated cases share the same value: missing field, empty `head(...)`, invalid `tail(...)` input, no-op spawn, false condition. | Preserve existing `0` behavior for compatibility, but stop treating it as a default pattern for new features. Before adding another `0`-based case, decide whether the language should keep `0` as the official absence/no-op value, add explicit predicates such as "is missing" or "is empty", or introduce a distinct absence value. |
| No static type checking; methods must handle possible runtime types defensively. | `AGENTS.md`, `kb/agerun-language-constraint-workarounds.md` | Acceptable as a dynamic-language constraint, but it raises the burden on orthogonal runtime behavior and diagnostics. | Preserve as current-state behavior; improve documentation and tests around type-dependent built-ins. |
| `if(...)` is documented as lazy value selection: evaluate the condition, then evaluate and return only the selected branch expression. | `SPEC.md`, `modules/ar_condition_instruction_evaluator.md`, `kb/agerun-language-constraint-workarounds.md` | Aligns with Single Source of Semantics for current instruction-level conditionals. Selected-branch evaluation fits conditional value selection; expression purity means branch evaluation must not become a place to hide side effects. | Preserve the lazy selected-branch rule. If `if(...)` later becomes an expression-level conditional, keep it pure and lazy. |
| No conditional execution statement exists; all method instructions execute in order and conditional behavior is encoded with value selection and no-op targets. | `kb/agerun-language-constraint-workarounds.md`, coordination method patterns | Aligns with line-based sequential evaluation and pure expressions, but it couples conditional side effects to sentinel/no-op instruction behavior instead of explicit control flow. | Keep in scope for the line-based, pure-expression, sequenced-instruction, and sentinel semantics follow-up plans. |
| `send(0, message)` is a no-op; `spawn(0, ...)` and `spawn("", ...)` are no-ops returning/storing `0`. | `AGENTS.md`, `SPEC.md`, `kb/no-op-semantics-pattern.md`, `kb/no-op-instruction-semantics.md` | Aligns with Explicit Exceptions when documented and with Expression Purity when kept outside expressions. It still assigns special instruction semantics to ordinary integer/string values. | Preserve the current no-op rules, but document them as instruction-specific exceptions: in `send(...)`, `0` means "no destination, do not send"; in `spawn(...)`, `0` or `""` means "do not spawn and return/store `0`". Do not imply that integer `0` has this no-op meaning in ordinary expression evaluation or ordinary data values. |
| `append(target, value)` accepts any target expression syntactically, but only mutates an existing memory-owned list; message/context/fresh/missing/non-list/protected targets are no-ops. | `SPEC.md`, `modules/ar_append_instruction_evaluator.md` | Explicit but not fully orthogonal: identical list values differ by origin and ownership. The expression-looking target hides lvalue semantics unless the root/path syntax is treated as the mutation marker. | Define lvalue/mutation-target rules for mutating instructions, not expression calls. |
| Writes are limited to `memory` paths, and `memory.self` plus nested `memory.self.*` are protected from assignment/result storage. | `SPEC.md`, `modules/ar_instruction_ast.md`, parse evaluator docs | Aligns with Explicit Exceptions and runtime identity safety. The `memory` root visibly marks writable state, but protected subpaths and function-result storage need one documented rule. | Preserve, but centralize the rule so assignment, function results, parse templates, and future merge-like operations cannot drift. |
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
| F1 | Built-in calls | Pure built-in calls are not composable as expressions. Calls are accepted only as top-level function instructions, with optional result assignment. | Function results are stored through instruction-specific result assignment, not by normal expression assignment. Effectful built-ins are correctly kept out of expressions if expression purity is a hard rule. | `SPEC.md` separates `<function-instruction>` from `<expression>`. `ar_expression_ast_t` has no call node, while `ar_instruction_ast_t` has per-call instruction types. `AGENTS.md` says function calls are not expressions. Disabled tests state function calls in expressions are not supported. | High |
| F1a | Function-call argument boundaries | Argument splitting is now consistent across instruction parsers. It preserves nested expression syntax inside one argument but still requires the argument to parse as an expression afterward. | The boundary rule is no longer duplicated across built-in parsers. Arity and instruction-specific semantics remain per call. | `SPEC.md` defines shared `<function-argument>` productions. `ar_function_call_parser` owns splitting and argument AST-list creation for C and Zig instruction parsers. `ar_function_call_parser_tests` covers nested list/map/quoted commas and nested call rejection as an expression. | Low |
| F1b | Quote and escape handling | Function-call boundary parsing is quote-aware, including even/odd backslash parity before quotes. Expression string parsing remains a simple raw span between delimiters. | Backslash has context-dependent meaning: it can keep a quote from closing an argument span, but it is preserved as data and is not decoded by expression evaluation. Escaped quotes are not currently string value characters. | `SPEC.md` and parser module docs document the split. `_isQuote` in `ar_function_call_parser.zig` counts consecutive backslashes before quotes. `ar_expression_parser.c` copies bytes between the opening quote and the next quote. | Low |
| F2 | Multiline list/map literals | Not composable. Multiline literals are canonicalized only as top-level assignment RHS values. | A list/map value has different syntax availability depending on whether it is one-line or multiline. | `SPEC.md`, `README.md`, and `ar_method_parser.md` say multiline lists/maps are assignment-only. Current corpus has 36 top-level multiline literal assignments. | Medium |
| F3 | `if(...)` condition and branch evaluation | Partially composable. `SPEC.md` now states that the first argument is an `<expression>`, and parser tests accept `if(1, 1, 0)`. Calls still cannot appear inside branches because calls are not expressions. | Docs now agree that the evaluator checks the condition first and evaluates only the selected branch. That is more orthogonal than the earlier stale both-branches wording. | `ar_condition_instruction_parser.c` parses all three arguments through `ar_expression_parser`; `ar_condition_instruction_evaluator.zig` selects one branch; `SPEC.md`, `ar_condition_instruction_evaluator.md`, and `kb/agerun-language-constraint-workarounds.md` document selected-branch evaluation. | Medium |
| F4 | Assignment vs result assignment | Expression assignment is normal only for `memory.path := <expression>`. Function result assignment is encoded inside function instruction AST nodes. | Pure expression results and effectful instruction results are represented through overlapping storage paths. The syntax may remain compact, but storage validation should have one owner. | `ar_instruction_ast_t` stores assignment data separately from function-call result paths. Instruction evaluators use `ar_instruction_ast__has_result_assignment()` and `ar_instruction_ast__get_function_result_path()`. | High |
| F5 | `append(...)` target | Syntactically accepts any expression for the target. Semantically only memory-owned lists can mutate. | A list value from `memory.results`, `message.results`, and `[1]` is not interchangeable for mutation. Non-memory targets become no-ops. As a mutating operation, `append(...)` should remain outside expression grammar. | `ar_append_instruction_evaluator.md` documents that message/context/fresh/non-list/missing/protected targets are no-ops. Tests cover message-owned, literal, and non-list no-op targets. | Medium |
| F6 | Missing field and empty-list sentinels | Composable as expressions once produced, but sentinel values leak into method logic. | Missing `message.field`, empty `head(...)`, invalid `tail(...)`, failed spawn, and no-op send/spawn all use integer `0` in different roles. | `SPEC.md` documents integer `0` sentinel behavior for `head(...)`, `tail(...)`, `send(0, ...)`, and `spawn(0, ...)`. Tests cover missing message fields for head/tail. | Medium |
| F7 | `memory.self` protection | Protection is consistently enforced for assignment and many result paths, but it is bolted onto instruction semantics. | Write permission depends on target root/path and, for parse, placeholder names/input origin. | `SPEC.md` says method instructions cannot assign or store into `memory.self`; `ar_instruction_ast__has_protected_memory_self_assignment()` supports instruction-level checks; parse evaluator adds placeholder/input-specific checks. | Medium |
| F8 | Operators over containers | One-line literals are composable in expressions, but equality semantics are partial. | Empty list equality exists; non-empty list structural equality does not. Maps are constructible but not structurally comparable. | `ar_expression_evaluator.md` says list equality is limited to empty-list checks and non-empty lists are not structurally compared. | Low |
| F9 | Documentation consistency | `SPEC.md` now names the language design principles and separates them from current language state. The earlier stale `if(...)` and string-escape wording has been corrected in the directly affected docs. | The remaining risk is future drift across the spec, KB, parser docs, and this audit report as language behavior evolves. | `SPEC.md` documents `Language Design Principles` and `Current Language State`. `AGENTS.md` and `kb/agerun-method-language-nesting-constraint.md` still document non-expression calls as current constraints. `kb/agerun-language-constraint-workarounds.md` now matches lazy `if(...)`. | Low |
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

- `memory.value := 1 + 1` is ordinary assignment, while `memory.value := build(...)` is parsed and
  evaluated through function-instruction result storage.
- `append(memory.items, value)` and `append(message.items, value)` use the same expression-looking
  target position, but only the `memory` target can mutate.
- Integer `0` can mean ordinary data, condition false, missing value, empty `head(...)`, invalid
  `tail(...)`, failed/no-op `spawn(...)`, or no-op `send(...)` destination.
- The `=` operator has special empty-list equality behavior but not general list/map structural
  equality.

Recommended follow-up: when a semantic distinction is intentional, either make it visible in syntax
or document it as an explicit exception. Avoid adding new constructs where the evaluator has to
infer a different behavior from hidden origin, ownership, or data-shape checks.

### 3. Pure Function Calls Are the Central Composability Gap

The grammar accepts built-ins through `<function-instruction>`, not `<expression>`. Each built-in
has a specialized parser and instruction AST type. The current workspace has already removed the
duplicated function-call argument scanner: the specialized parsers share `ar_function_call_parser`
for argument boundaries and argument AST-list creation, and `SPEC.md` now reflects that shared
argument grammar. The expression AST still has no call node. Because of that split, pure value
compositions are not supported:

- `memory.payload := parse("name={name}", build("name={name}", memory))`
- `memory.items := [head(memory.items), tail(memory.items)]`
- `memory.count_like := head(tail(memory.items))`
- `memory.label := if(memory.enabled = 1, build("ready {id}", memory), "blocked")`

The existing method corpus appears to work around this by using intermediate memory slots. The audit
regex found 1,023 assigned built-in call lines and no nested built-in call lines in `.method`
sources. That is strong evidence that the grammar shape controls method style.

Recommended follow-up: reuse the shared function-call boundary parser while adding an
expression-level call AST and call evaluator only for calls classified as pure. Calls such as
`parse`, `build`, `head`, `tail`, and value-returning `if(...)` are good candidate expressions.
Side-effectful operations such as `send`, `spawn`, `exit`, `deprecate`, `append`, `compile`, and
`complete` should remain sequenced instructions unless their semantics are split into pure value
production plus a separate effectful instruction.

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

Regular assignment stores an evaluated expression into memory. Function result assignment stores a
function result into memory from inside the function instruction evaluator. This duplicate path
means every instruction evaluator that can store a result must repeat result-path validation,
ownership transfer, and protected `memory.self` handling.

Recommended follow-up: once pure calls are expressions, reduce pure result storage to ordinary
assignment:

- current parse/build/head/tail form: `memory.value := build("hello {name}", memory)`
- intended pure-expression model: assignment evaluates a pure call expression and stores the result

Effectful instructions that return values, such as a future `spawn(...)` result binding, may still
need statement-level result storage. If so, that storage rule should be centralized and explicitly
separate from expression assignment rather than repeated in each evaluator.

### 8. Mutation Semantics Need an Explicit Lvalue Concept

`append(...)` already reveals an important design boundary. Its target argument parses as a normal
expression, but mutation succeeds only when the expression resolves to a memory-owned list. A
message-owned list, context-owned list, fresh literal list, missing target, non-list, or protected
`memory.self` target becomes a no-op.

That may be the right runtime safety model, but it is not semantically orthogonal if all expressions
are treated as interchangeable values. It needs explicit language wording: some call arguments are
values, while mutation targets are lvalues with memory ownership requirements.

Recommended follow-up: define lvalue semantics for mutating instructions without making those
instructions expression-level. That definition should cover `append(...)` now and any future
mutating operations.

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

1. **Purity and syntax classification design note**: decide which built-ins are pure expressions,
   which must remain sequenced instructions because they mutate state, communicate, create agents, or
   otherwise perform effects, and how those differences are visible in syntax.
2. **AST refactor plan**: add `AR_EXPRESSION_AST_TYPE__CALL` for pure calls, reuse
   `ar_function_call_parser` for expression-call argument boundaries, define function metadata once,
   and reduce remaining per-built-in metadata/evaluator duplication without admitting
   side-effectful operations into expression parsing or weakening line-based instruction parsing.
3. **Result binding plan**: make `memory.path := <expression>` the only pure-expression storage
   mechanism, and centralize any statement-level result binding needed by effectful instructions.
4. **Multiline expression plan**: choose between documenting assignment-only multiline literals as
   an explicit exception or promoting them into the expression parser.
5. **Sentinel semantics plan**: evaluate whether integer `0` remains the language-wide absent value
   or whether the data model needs an explicit absence representation.

## Current Baseline Now Satisfied

- Function-call argument splitting and expression-nesting boundary handling have one source of truth
  in `ar_function_call_parser`.
- `SPEC.md` names shared function-argument productions instead of spelling each built-in argument
  list independently.
- `SPEC.md` now documents that quote/backslash handling for function-call argument splitting is a
  boundary rule, while string literal values preserve source characters and do not decode escapes.
- `SPEC.md` now has separate `Language Design Principles` and `Current Language State` sections.
- `SPEC.md`, `kb/agerun-language-constraint-workarounds.md`, and
  `modules/ar_condition_instruction_evaluator.md` now agree on lazy selected-branch `if(...)`
  evaluation.
- Parser module docs now describe quote/backslash handling as function-call boundary parsing rather
  than value-level escaped-quote or escape-sequence support.

## Acceptance Criteria for Remaining Future Implementation

- Each nonempty source line remains one complete instruction boundary.
- Method evaluation remains source-ordered and line-based.
- Expression evaluation is side-effect free by grammar and by evaluator contract.
- Each syntax construct maps to one documented evaluation behavior.
- Semantic differences are visible in syntax or documented as explicit exceptions.
- Pure built-in calls can appear anywhere an expression can appear.
- Side-effectful built-ins remain explicit sequenced instructions.
- Assignment evaluates and stores pure expression results through one storage path.
- Mutating instructions have explicit lvalue rules instead of hidden origin checks.
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
