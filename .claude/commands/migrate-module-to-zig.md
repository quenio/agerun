# Migrate Module to Zig

{{#if 1}}
First, check if `modules/{{1}}.zig` already exists. If it does, inform the user about the migration status instead of creating a new plan.

If the module hasn't been migrated yet, migrate the implementation of the **{{1}}** module from C to Zig following the systematic approach documented in CLAUDE.md and the comprehensive migration guide at kb/c-to-zig-module-migration.md. For complete integration details, see ([details](kb/zig-integration-comprehensive.md)).
{{else}}
Before I can create a migration plan, I need to know which module you'd like to migrate to Zig.

Please provide the module name (without the `ar_` prefix). For example:
- `string` (for ar_string module)
- `expression_ast` (for ar_expression_ast module)
- `data` (for ar_data module)

Which module would you like to migrate to Zig?
{{/if}}

## Migration Requirements

1. **Follow the migration guide**: Use kb/c-to-zig-module-migration.md as your primary reference
2. **Maintain C API compatibility**: The Zig implementation must be a drop-in replacement
3. **Apply memory debugging patterns**: Use kb/zig-migration-memory-debugging.md for ownership issues
4. **Zero memory leaks**: Verify with individual test memory reports
5. **Validate documentation**: Ensure all documentation passes `make check-docs`

## Pre-Migration Checklist

{{#if 1}}
Before creating the plan, verify:
- **Check if already migrated**: If `modules/{{1}}.zig` exists, inform the user that migration is already done or in progress
- Module exists: `modules/{{1}}.c` and `modules/{{1}}.h`
- Tests exist: `modules/{{1}}_tests.c`
- Current test status: Run `make {{1}}_tests` and check memory report
- Dependencies: Audit with `grep -n "#include.*ar_" modules/{{1}}.h modules/{{1}}.c`

If `modules/{{1}}.zig` already exists:
- If `modules/{{1}}.c` also exists: "The {{1}} module migration to Zig appears to be in progress. The Zig file already exists alongside the C implementation."
- If `modules/{{1}}.c` doesn't exist: "The {{1}} module has already been migrated to Zig. The implementation is in `modules/{{1}}.zig`."
{{/if}}

## Plan Structure

Create a detailed migration plan that includes:

### Phase 1: Assessment
{{#if 1}}
- Dependency analysis (which modules does {{1}} depend on?)
{{else}}
- Dependency analysis (which modules does the target module depend on?)
{{/if}}
- Migration challenges (variadic functions, platform-specific code, etc.)
- Current API documentation review
- Memory ownership patterns identification

### Phase 2: Implementation Strategy
- Zig file creation approach
- Type mapping decisions
- Memory management approach
- Error handling strategy

### Phase 3: Testing Approach
- How to verify behavior matches C implementation
- Memory leak detection strategy
- Documentation validation approach

### Phase 4: Cleanup Plan
- C file deletion timing
- Documentation updates needed
- Verification steps

## Critical Reminders

- **All parameters must be nullable** for C API compatibility
- **Reserved keywords** like 'type' need renaming (e.g., 'node_type')
- **Match C behavior exactly** for ownership transfer functions ([details](kb/zig-migration-memory-debugging.md))
- **Use C types directly** to minimize casting
- **Compare implementations line-by-line** when debugging
- **Memory allocation**: Use ar_allocator for type-safe operations ([details](kb/zig-memory-allocation-with-ar-allocator.md))
- **Debug consistency**: Ensure -DDEBUG -D__ZIG__ flags ([details](kb/zig-c-memory-tracking-consistency.md))
- **Build configuration**: Follow proper Zig build flags ([details](kb/zig-build-flag-configuration.md))

## User Approval

{{#if 1}}
After creating the plan, ask for user approval with:
"I've created a migration plan for the {{1}} module. This plan follows the systematic C-to-Zig migration guide and includes all necessary verification steps. Would you like me to proceed with the migration?"
{{else}}
After the user provides the module name, create the plan and then ask for approval.
{{/if}}

Only proceed with implementation after receiving explicit approval.

## Post-Migration Verification

{{#if 1}}
After migration, ensure:
1. All tests pass: `make {{1}}_tests`
2. Zero memory leaks: Check `bin/memory_report_{{1}}_tests.log`
3. Documentation valid: `make check-docs`
4. Clean build: `make clean build`
{{else}}
After migration, ensure all tests pass, zero memory leaks, documentation is valid, and clean build succeeds.
{{/if}}

Remember to follow TDD principles - complete any in-progress development cycles before starting the migration.