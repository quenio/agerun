# Migrate to Zig Struct Module

**Role: Migration Specialist**

{{#if 1}}
Migrate the **ar_{{1}}** module from a C-ABI compatible Zig module to a pure Zig struct module following the pattern documented in kb/zig-struct-modules-pattern.md.

## Read KB Article First

I'll start by reading the knowledge base article to understand the proper patterns for implementing Zig struct modules.

```bash
# Read the KB article about Zig struct modules
cat kb/zig-struct-modules-pattern.md
```

## Pre-Migration Dependency Check

First, I need to verify that ar_{{1}} is safe to migrate by checking:

1. **Current Implementation**: Verify that `modules/ar_{{1}}.zig` exists (must be a Zig ABI module)
2. **C Module Dependencies**: Search for any C modules (.c files) that include or depend on ar_{{1}}
3. **Zig Module Dependencies**: Identify Zig modules that depend on ar_{{1}} (these can be updated)
4. **Module's Own Dependencies**: Check if ar_{{1}} depends on C-ABI modules (would block migration)

### Dependency Analysis Steps:

```bash
# Check if source module is already in Zig
ls -la modules/ar_{{1}}.zig

# Check for C module dependencies (these would block migration)
grep -l "ar_{{1}}.h" modules/*.c | grep -v "_tests.c"

# Check for includes in C headers (transitive dependencies)
grep -l "ar_{{1}}.h" modules/*.h

# Check for Zig module dependencies (these can be updated)
grep -l '@cImport.*ar_{{1}}.h' modules/*.zig
grep -l 'ar_{{1}}__' modules/*.zig

# Check what C headers ar_{{1}} itself imports (these would block migration)
echo "Checking ar_{{1}}'s dependencies on C-ABI modules:"
grep -n "@cInclude" modules/ar_{{1}}.zig | head -20
```

**Migration Blockers**:
- If any C modules depend on ar_{{1}}, migration cannot proceed
- If ar_{{1}} itself uses `@cImport` to depend on C-ABI modules, migration cannot proceed due to type incompatibility issues between different `@cImport` namespaces

## Migration Plan (if dependencies allow)

If the module is safe to migrate:

### Phase 1: Create New Zig Struct Module
1. Create `modules/{{1|pascal}}.zig` following TitleCase naming
2. Convert the module to use idiomatic Zig patterns:
   - Change from `export fn` to `pub fn`
   - Use `init`/`deinit` instead of `create`/`destroy`
   - Use camelCase for function names
   - Use Zig types (slices, error unions) instead of C types
   - Maintain ownership prefixes (`own_`, `mut_`, `ref_`)

### Phase 2: Update Zig Dependencies
For each Zig module that depends on ar_{{1}}:
1. Change from `@cImport` to direct `@import("{{1|pascal}}.zig")`
2. Update function calls to use new API names
3. Update type usage to Zig types

### Phase 3: Remove Old Module
1. Delete `modules/ar_{{1}}.zig`
2. Delete `modules/ar_{{1}}.h`
3. Update `modules/ar_{{1}}.md` to `modules/{{1|pascal}}.md`

### Phase 4: Create Tests
1. Create `modules/{{1|pascal}}Tests.zig` using Zig test framework
2. Remove old `modules/ar_{{1}}_tests.c` if it exists

### Phase 5: Update Documentation
1. Update module documentation to reflect new API
2. Update modules/README.md to list under "Zig Struct Modules"
3. Add references to kb/zig-struct-modules-pattern.md

## Safety Checks

Before proceeding:
- [ ] No C modules depend on ar_{{1}}
- [ ] ar_{{1}} does not use @cImport to depend on C-ABI modules
- [ ] All Zig dependencies identified and migration plan created
- [ ] Module is truly internal (not part of public API)
- [ ] Tests exist to verify functionality during migration

{{else}}
Before I can create a migration plan, I need to know which Zig ABI module you'd like to migrate to a Zig struct module.

Please provide the module name (without the `ar_` prefix). For example:
- `string` (to migrate ar_string to String.zig)
- `heap` (to migrate ar_heap to Heap.zig)

Note: Only modules that are:
1. Currently implemented in Zig (have .zig files)
2. NOT referenced by any C modules
3. Internal utilities (not part of public API)

can be migrated to Zig struct modules.

Which module would you like to migrate?
{{/if}}

## Important Constraints

**CRITICAL**: Zig struct modules have strict limitations:

1. **C modules cannot import Zig struct modules** - Any C dependency blocks migration
2. **Type incompatibility with @cImport** - If the module uses @cImport to call C functions, it cannot be migrated due to type mismatches between different @cImport namespaces
3. **Only pure Zig or Zig-struct dependencies allowed** - The module can only depend on:
   - Other Zig struct modules
   - Pure Zig code with no C dependencies
   - The Zig standard library

This migration can only proceed if:
- No C modules depend on the target module
- The target module does not depend on any C-ABI modules
- Only Zig modules depend on it (these can be updated)
- The module is not part of the public C API

If the module needs to interact with C code in any way, it must remain C-ABI compatible.