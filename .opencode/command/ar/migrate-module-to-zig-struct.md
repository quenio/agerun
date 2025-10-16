Migrate a Zig C-ABI module to a pure Zig struct module.

## MANDATORY KB Consultation

Before starting migration:
1. Search: `grep "zig.*struct\|TitleCase" kb/README.md`
2. Must read:
   - zig-struct-modules-pattern
   - zig-integration-comprehensive
3. Apply struct module conventions

## CHECKPOINT WORKFLOW ENFORCEMENT

**CRITICAL**: This command MUST use checkpoint tracking for ALL execution.

### In-Progress Workflow Detection

If a `/migrate-module-to-zig-struct` workflow is already in progress:

```bash
make checkpoint-status CMD=migrate-module-to-zig-struct VERBOSE=--verbose
# Resume: make checkpoint-update CMD=migrate-module-to-zig-struct STEP=N
# Or reset: make checkpoint-cleanup CMD=migrate-module-to-zig-struct && make checkpoint-init CMD=migrate-module-to-zig-struct STEPS='"Read KB Article" "Check Current Implementation" "Check C Dependencies" "Check Zig Dependencies" "Verify Safety" "Create Struct Module" "Convert Functions" "Update Dependencies" "Run Tests" "Remove Old Module" "Update Documentation"'
```

### First-Time Initialization Check

```bash
if [ ! -f /tmp/migrate_module_to_zig_struct_progress.txt ]; then
  echo "⚠️  Initializing checkpoint tracking..."
  make checkpoint-init CMD=migrate-module-to-zig-struct STEPS='"Read KB Article" "Check Current Implementation" "Check C Dependencies" "Check Zig Dependencies" "Verify Safety" "Create Struct Module" "Convert Functions" "Update Dependencies" "Run Tests" "Remove Old Module" "Update Documentation"'
else
  make checkpoint-status CMD=migrate-module-to-zig-struct
fi
```

## PRECONDITION: Checkpoint Tracking Must Be Initialized

```bash
if [ ! -f /tmp/migrate_module_to_zig_struct_progress.txt ]; then
  echo "❌ ERROR: Checkpoint tracking not initialized!"
  exit 1
fi
```

# Migrate Module to Zig Struct
## Checkpoint Tracking

This command uses checkpoint tracking to ensure safe and systematic module migration. The process has 11 checkpoints across 4 phases with critical safety gates.

### Initialize Tracking
```bash
# Start the migration process (replace MODULE with actual module name)
MODULE={{1}}  # Set the module name
make checkpoint-init CMD=migrate-module-to-zig-struct STEPS='"Read KB Article" "Check Current Implementation" "Check C Dependencies" "Check Zig Dependencies" "Verify Safety" "Create Struct Module" "Convert Functions" "Update Dependencies" "Run Tests" "Remove Old Module" "Update Documentation"'
```

**Expected output:**
```
📍 Starting: migrate-module-to-zig-struct (11 steps)
📁 Tracking: /tmp/migrate-module-to-zig-struct_progress.txt
→ Run: make checkpoint-update CMD=migrate-module-to-zig-struct STEP=1
```

### Check Progress
```bash
make checkpoint-status CMD=migrate-module-to-zig-struct
```

**Expected output (example at 64% completion):**
```
📈 command: X/Y steps (Z%)
   [████░░░░░░░░░░░░░░░░] Z%
→ Next: make checkpoint-update CMD=command STEP=N
```

## Minimum Requirements

**MANDATORY for successful migration:**
- [ ] No C module dependencies (would block migration)
- [ ] No @cImport usage in target module
- [ ] All tests pass after migration
- [ ] Zero memory leaks after migration
- [ ] All old files removed
- [ ] Documentation updated

{{#if 1}}
Migrate the **ar_{{1}}** module from a C-ABI compatible Zig module to a pure Zig struct module following the pattern documented in kb/zig-struct-modules-pattern.md.

### Stage 1: Assessment (Steps 1-5)

#### [CHECKPOINT START - STAGE 1]

#### [CHECKPOINT END]

#### Read KB Article First

#### Step 1: Read KB Article

I'll start by reading the knowledge base article to understand the proper patterns for implementing Zig struct modules.

```bash
# Read the KB article about Zig struct modules
cat kb/zig-struct-modules-pattern.md

# Initialize tracking
MODULE="{{1}}"
echo "MODULE=$MODULE" > /tmp/migration-tracking.txt
echo "CREATED_FILES=0" >> /tmp/migration-tracking.txt
echo "DELETED_FILES=0" >> /tmp/migration-tracking.txt
echo "UPDATED_DEPS=0" >> /tmp/migration-tracking.txt
echo "FUNCTIONS_CONVERTED=0" >> /tmp/migration-tracking.txt

make checkpoint-update CMD=migrate-module-to-zig-struct STEP=1
```

#### [KNOWLEDGE GATE]
```bash
# Confirm KB article was read and understood
make checkpoint-gate CMD=migrate-module-to-zig-struct GATE="Knowledge" REQUIRED="1"
```

**Expected gate output:**
```
✅ GATE 'Knowledge' - PASSED
   Verified: Steps 1
```

#### Pre-Migration Dependency Check

#### Step 2: Check Current Implementation

First, I need to verify that ar_{{1}} is safe to migrate by checking:

1. **Current Implementation**: Verify that `modules/ar_{{1}}.zig` exists (must be a Zig ABI module)

```bash
# Check if source module exists and is in Zig
if [ -f "modules/ar_${MODULE}.zig" ]; then
  echo "✅ Module ar_${MODULE}.zig exists"
  make checkpoint-update CMD=migrate-module-to-zig-struct STEP=2
else
  echo "❌ Module ar_${MODULE}.zig not found"
  echo "Migration requires existing Zig ABI module"
  exit 1
fi
```

#### Step 3: Check C Dependencies

2. **C Module Dependencies**: Search for any C modules (.c files) that include or depend on ar_{{1}}

```bash
# Check for C module dependencies (these would block migration)
C_DEPS=$(grep -l "ar_${MODULE}.h" modules/*.c | grep -v "_tests.c" | wc -l)
if [ $C_DEPS -gt 0 ]; then
  echo "❌ MIGRATION BLOCKED: $C_DEPS C modules depend on ar_${MODULE}"
  echo "C modules found:"
  grep -l "ar_${MODULE}.h" modules/*.c | grep -v "_tests.c"
  echo "Migration cannot proceed - C modules cannot import Zig struct modules"
  exit 1
else
  echo "✅ No C module dependencies found"
  make checkpoint-update CMD=migrate-module-to-zig-struct STEP=3
fi
```

#### Step 4: Check Zig Dependencies

3. **Zig Module Dependencies**: Identify Zig modules that depend on ar_{{1}} (these can be updated)

```bash
# Check for Zig module dependencies (these can be updated)
ZIG_DEPS=$(grep -l "ar_${MODULE}__\|@cImport.*ar_${MODULE}.h" modules/*.zig | grep -v "ar_${MODULE}.zig" | wc -l)
echo "Found $ZIG_DEPS Zig modules that depend on ar_${MODULE}"
echo "These will need to be updated:"
grep -l "ar_${MODULE}__\|@cImport.*ar_${MODULE}.h" modules/*.zig | grep -v "ar_${MODULE}.zig"

echo "UPDATED_DEPS=$ZIG_DEPS" >> /tmp/migration-tracking.txt
make checkpoint-update CMD=migrate-module-to-zig-struct STEP=4
```

#### Step 5: Verify Safety

4. **Module's Own Dependencies**: Check if ar_{{1}} depends on C-ABI modules (would block migration)

#### Dependency Analysis Steps

```bash
# Check what C headers ar_{{1}} itself imports (these would block migration)
echo "Checking ar_${MODULE}'s dependencies on C-ABI modules:"
if grep -q "@cImport\|@cInclude" "modules/ar_${MODULE}.zig"; then
  echo "❌ MIGRATION BLOCKED: Module uses @cImport for C dependencies"
  echo "Found C imports:"
  grep -n "@cImport\|@cInclude" "modules/ar_${MODULE}.zig" | head -10
  echo "Type incompatibility prevents migration"
  exit 1
else
  echo "✅ No @cImport usage found - module can be migrated"
  make checkpoint-update CMD=migrate-module-to-zig-struct STEP=5
fi
```

#### [CRITICAL SAFETY GATE]
```bash
# ⚠️ CRITICAL: Verify migration is safe
make checkpoint-gate CMD=migrate-module-to-zig-struct GATE="Safety" REQUIRED="2,3,4,5"
```

**Expected gate output:**
```
✅ GATE 'Safety' - PASSED
   Verified: Steps 2,3,4,5
```

**Migration Blockers**:
- If any C modules depend on ar_{{1}}, migration cannot proceed
- If ar_{{1}} itself uses `@cImport` to depend on C-ABI modules, migration cannot proceed due to type incompatibility issues between different `@cImport` namespaces

### Stage 2: Implementation (Steps 6-7)

#### [CHECKPOINT START - STAGE 2]

#### [CHECKPOINT END]

#### Migration Plan

If the module is safe to migrate:

#### Step 6: Create Struct Module

##### Create New Zig Struct Module
1. Create `modules/{{1|pascal}}.zig` following TitleCase naming

```bash
# Create new struct module
echo "Creating modules/{{1|pascal}}.zig..."
touch "modules/{{1|pascal}}.zig"

source /tmp/migration-tracking.txt
CREATED_FILES=$((CREATED_FILES + 1))
echo "CREATED_FILES=$CREATED_FILES" >> /tmp/migration-tracking.txt

make checkpoint-update CMD=migrate-module-to-zig-struct STEP=6
```

#### Step 7: Convert Functions

2. Convert the module to use idiomatic Zig patterns:
   - Change from `export fn` to `pub fn`
   - Use `init`/`deinit` instead of `create`/`destroy`
   - Use camelCase for function names
   - Use Zig types (slices, error unions) instead of C types
   - Maintain ownership prefixes (`own_`, `mut_`, `ref_`)

```bash
# Track function conversion progress
TOTAL_FUNCTIONS=$(grep -c "export fn" "modules/ar_${MODULE}.zig")
echo "Converting $TOTAL_FUNCTIONS functions to Zig struct patterns..."

# After converting functions
source /tmp/migration-tracking.txt
FUNCTIONS_CONVERTED=$TOTAL_FUNCTIONS
echo "FUNCTIONS_CONVERTED=$FUNCTIONS_CONVERTED" >> /tmp/migration-tracking.txt
echo "✅ Converted $FUNCTIONS_CONVERTED functions"

make checkpoint-update CMD=migrate-module-to-zig-struct STEP=7
```

#### [IMPLEMENTATION GATE]
```bash
# Verify implementation is complete
make checkpoint-gate CMD=migrate-module-to-zig-struct GATE="Implementation" REQUIRED="6,7"
```

**Expected gate output:**
```
✅ GATE 'Implementation' - PASSED
   Verified: Steps 6,7
```

### Stage 3: Testing (Steps 8-9)

#### [CHECKPOINT START - STAGE 3]

#### [CHECKPOINT END]

#### Step 8: Update Dependencies

### Update Zig Dependencies
For each Zig module that depends on ar_{{1}}:
1. Change from `@cImport` to direct `@import("{{1|pascal}}.zig")`
2. Update function calls to use new API names
3. Update type usage to Zig types

```bash
# Update dependent modules
source /tmp/migration-tracking.txt
echo "Updating $UPDATED_DEPS dependent Zig modules..."

# After updating dependencies
echo "✅ Updated $UPDATED_DEPS modules to use {{1|pascal}}.zig"

make checkpoint-update CMD=migrate-module-to-zig-struct STEP=8
```

#### Step 9: Run Tests

```bash
# Run tests and verify no memory leaks
echo "Running tests for migrated module..."
make {{1|pascal}}Tests 2>&1 | tee test_output.log

# Check for memory leaks
if grep -q "memory leaks detected" test_output.log; then
  echo "❌ Memory leaks detected in migrated module"
  exit 1
else
  echo "✅ No memory leaks - all tests passed"
  make checkpoint-update CMD=migrate-module-to-zig-struct STEP=9
fi
```

#### [CRITICAL TESTING GATE]
```bash
# ⚠️ CRITICAL: Verify all tests pass with no leaks
make checkpoint-gate CMD=migrate-module-to-zig-struct GATE="Testing" REQUIRED="8,9"
```

**Expected gate output:**
```
✅ GATE 'Testing' - PASSED
   Verified: Steps 8,9
```

### Stage 4: Cleanup (Steps 10-11)

#### [CHECKPOINT START - STAGE 4]

#### [CHECKPOINT END]

#### Step 10: Remove Old Module

### Remove Old Module
1. Delete `modules/ar_{{1}}.zig`
2. Delete `modules/ar_{{1}}.h`
3. Update `modules/ar_{{1}}.md` to `modules/{{1|pascal}}.md`

```bash
# Remove old module files
echo "Removing old module files..."
rm -f "modules/ar_${MODULE}.zig"
rm -f "modules/ar_${MODULE}.h"
mv "modules/ar_${MODULE}.md" "modules/{{1|pascal}}.md" 2>/dev/null || true

# Remove old test file if exists
rm -f "modules/ar_${MODULE}_tests.c"

source /tmp/migration-tracking.txt
DELETED_FILES=3
echo "DELETED_FILES=$DELETED_FILES" >> /tmp/migration-tracking.txt
echo "✅ Removed $DELETED_FILES old files"

make checkpoint-update CMD=migrate-module-to-zig-struct STEP=10
```

#### Step 11: Update Documentation

### Update Documentation
1. Update module documentation to reflect new API
2. Update modules/README.md to list under "Zig Struct Modules"
3. Add references to kb/zig-struct-modules-pattern.md

```bash
# Update documentation
echo "Updating documentation..."
echo "- Updated modules/{{1|pascal}}.md with new API"
echo "- Added to Zig Struct Modules section in README.md"
echo "- Added KB article references"

make checkpoint-update CMD=migrate-module-to-zig-struct STEP=11
```

#### [CHECKPOINT COMPLETE]
```bash
# Show final summary
make checkpoint-status CMD=migrate-module-to-zig-struct
```

**Expected completion output:**
```
========================================
   CHECKPOINT STATUS: migrate-module-to-zig-struct
========================================

Progress: 11/11 steps (100%)

[████████████████████] 100%

✅ ALL CHECKPOINTS COMPLETE!

Migration Summary:
Successfully migrated: ar_{{1}} → {{1|pascal}}.zig

- Functions converted: 12
- Dependencies updated: 5
- Tests passed: 15/15
- Memory leaks: 0
- Files removed: 3
- Documentation updated: ✓

The module is now a pure Zig struct module!
```

```bash
# Clean up tracking
make checkpoint-cleanup CMD=migrate-module-to-zig-struct
rm -f /tmp/migration-tracking.txt
rm -f test_output.log
```

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

## Troubleshooting

### If Safety Gate fails:
```bash
# The Safety Gate is CRITICAL - it prevents breaking the build
# Common reasons for failure:
1. C modules depend on the target module
2. Module uses @cImport for C dependencies
3. Module is part of public API

# To check specific issues:
grep -l "ar_${MODULE}.h" modules/*.c  # Check C dependencies
grep "@cImport" modules/ar_${MODULE}.zig  # Check for C imports
```

### If tests fail after migration:
```bash
# Debug test failures
1. Check function name conversions (camelCase)
2. Verify ownership prefixes maintained
3. Check type conversions (C types to Zig types)
4. Run with verbose output: make {{1|pascal}}Tests VERBOSE=1
```

### If memory leaks detected:
```bash
# Memory leak debugging
1. Check init/deinit pairing
2. Verify ownership transfer in functions
3. Check allocator usage consistency
4. Review error handling paths
```

## Related Documentation

### Checkpoint Patterns
- [Multi-Step Checkpoint Tracking Pattern](../../../kb/multi-step-checkpoint-tracking-pattern.md)
- [Gate Enforcement Exit Codes Pattern](../../../kb/gate-enforcement-exit-codes-pattern.md)
- [Command Thoroughness Requirements Pattern](../../../kb/command-thoroughness-requirements-pattern.md)

### Zig Migration Patterns
- [Zig Struct Modules Pattern](../../../kb/zig-struct-modules-pattern.md)
- [Zig Integration Comprehensive](../../../kb/zig-integration-comprehensive.md)
- [C to Zig Module Migration](../../../kb/c-to-zig-module-migration.md)
- [Implementation Plan Line References](../../../kb/implementation-plan-line-references.md)

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