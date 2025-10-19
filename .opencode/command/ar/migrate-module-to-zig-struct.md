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
./scripts/checkpoint-status.sh migrate-module-to-zig-struct --verbose
# Resume: ./scripts/checkpoint-update.sh migrate-module-to-zig-struct STEP=N
# Or reset: ./scripts/checkpoint-cleanup.sh migrate-module-to-zig-struct && ./scripts/checkpoint-init.sh migrate-module-to-zig-struct "Read KB Article" "Check Current Implementation" "Check C Dependencies" "Check Zig Dependencies" "Verify Safety" "Create Struct Module" "Convert Functions" "Update Dependencies" "Run Tests" "Remove Old Module" "Update Documentation"
```

### First-Time Initialization Check

```bash
./scripts/checkpoint-init.sh migrate-module-to-zig-struct "Read KB Article" "Check Current Implementation" "Check C Dependencies" "Check Zig Dependencies" "Verify Safety" "Create Struct Module" "Convert Functions" "Update Dependencies" "Run Tests" "Remove Old Module" "Update Documentation"
```

## PRECONDITION: Checkpoint Tracking Must Be Initialized

```bash
./scripts/checkpoint-require.sh migrate-module-to-zig-struct
```

# Migrate Module to Zig Struct
## Checkpoint Tracking

This command uses checkpoint tracking to ensure safe and systematic module migration. The process has 11 checkpoints across 4 phases with critical safety gates.

### Initialize Tracking
```bash
# Start the migration process (replace MODULE with actual module name)
MODULE={{1}}  # Set the module name
./scripts/checkpoint-init.sh migrate-module-to-zig-struct "Read KB Article" "Check Current Implementation" "Check C Dependencies" "Check Zig Dependencies" "Verify Safety" "Create Struct Module" "Convert Functions" "Update Dependencies" "Run Tests" "Remove Old Module" "Update Documentation"
```

**Expected output:**
```
📍 Starting: migrate-module-to-zig-struct (11 steps)
📁 Tracking: /tmp/migrate-module-to-zig-struct-progress.txt
→ Run: ./scripts/checkpoint-update.sh migrate-module-to-zig-struct STEP=1
```

### Check Progress
```bash
./scripts/checkpoint-status.sh migrate-module-to-zig-struct
```

**Expected output (example at 64% completion):**
```
📈 command: X/Y steps (Z%)
   [████░░░░░░░░░░░░░░░░] Z%
→ Next: ./scripts/checkpoint-update.sh command STEP=N
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

./scripts/checkpoint-update.sh migrate-module-to-zig-struct STEP=1
```

#### [KNOWLEDGE GATE]
```bash
# Confirm KB article was read and understood
./scripts/checkpoint-gate.sh migrate-module-to-zig-struct "Knowledge" "1"
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
  ./scripts/checkpoint-update.sh migrate-module-to-zig-struct STEP=2
else
  echo "❌ Module ar_${MODULE}.zig not found"
  echo "Migration requires existing Zig ABI module"
  exit 1
fi
```

#### Steps 3-5: Check Dependencies

Run dependency analysis using helper script:

```bash
# Check all dependencies (Steps 3-5 combined)
./scripts/check-module-dependencies.sh ${MODULE} | tee -a /tmp/migration-tracking.txt

# If script exits 0, migration is safe - mark steps complete
if [ $? -eq 0 ]; then
  ./scripts/checkpoint-update.sh migrate-module-to-zig-struct STEP=3
  ./scripts/checkpoint-update.sh migrate-module-to-zig-struct STEP=4
  ./scripts/checkpoint-update.sh migrate-module-to-zig-struct STEP=5
else
  echo "❌ Migration blocked - see errors above"
  exit 1
fi
```

The script checks:
1. **C Module Dependencies** - Any C modules that depend on this module (blocks migration)
2. **Zig Module Dependencies** - Zig modules that need updates after migration
3. **Module's Own Dependencies** - Whether module uses @cImport (blocks migration)

#### [CRITICAL SAFETY GATE]
```bash
# ⚠️ CRITICAL: Verify migration is safe
./scripts/checkpoint-gate.sh migrate-module-to-zig-struct "Safety" "2,3,4,5"
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

./scripts/checkpoint-update.sh migrate-module-to-zig-struct STEP=6
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

./scripts/checkpoint-update.sh migrate-module-to-zig-struct STEP=7
```

#### [IMPLEMENTATION GATE]
```bash
# Verify implementation is complete
./scripts/checkpoint-gate.sh migrate-module-to-zig-struct "Implementation" "6,7"
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

./scripts/checkpoint-update.sh migrate-module-to-zig-struct STEP=8
```

#### Step 9: Run Tests

```bash
# Run tests and verify no memory leaks
echo "Running tests for migrated module..."
make {{1|pascal}}Tests 2>&1 | tee test-output.log

# Check for memory leaks
if grep -q "memory leaks detected" test-output.log; then
  echo "❌ Memory leaks detected in migrated module"
  exit 1
else
  echo "✅ No memory leaks - all tests passed"
  ./scripts/checkpoint-update.sh migrate-module-to-zig-struct STEP=9
fi
```

#### [CRITICAL TESTING GATE]
```bash
# ⚠️ CRITICAL: Verify all tests pass with no leaks
./scripts/checkpoint-gate.sh migrate-module-to-zig-struct "Testing" "8,9"
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

./scripts/checkpoint-update.sh migrate-module-to-zig-struct STEP=10
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

./scripts/checkpoint-update.sh migrate-module-to-zig-struct STEP=11
```

#### [CHECKPOINT COMPLETE]
```bash
./scripts/checkpoint-complete.sh migrate-module-to-zig-struct
```

**Expected completion output:**
```
========================================
   CHECKPOINT COMPLETION SUMMARY
========================================

📈 migrate-module-to-zig-struct: X/Y steps (Z%)
   [████████████████████] 100%

✅ Checkpoint workflow complete
```
rm -f /tmp/migration-tracking.txt
rm -f test-output.log
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