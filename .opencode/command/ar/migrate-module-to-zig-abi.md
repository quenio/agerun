Migrate a C module to Zig while maintaining C API compatibility.

## MANDATORY KB Consultation

Before starting migration:
1. Search: `grep "zig\|migration\|c-to-zig" kb/README.md`
2. Must read these articles:
   - c-to-zig-module-migration
   - zig-integration-comprehensive
   - zig-module-development-guide
3. Apply migration patterns and best practices

## CHECKPOINT WORKFLOW ENFORCEMENT

**CRITICAL**: This command MUST use checkpoint tracking for ALL execution. All verification is done via step-verifier sub-agent, NOT via checkpoint scripts ([details](../../../kb/checkpoint-tracking-verification-separation.md)).

## STEP VERIFICATION ENFORCEMENT

**MANDATORY**: After completing each step, you MUST verify step completion using the **step-verifier sub-agent** before proceeding to the next step ([details](../../../kb/sub-agent-verification-pattern.md)).

### About the step-verifier Sub-Agent

The **step-verifier** is a specialized sub-agent that independently verifies step completion:

- **Reads command files** to understand step requirements
- **Checks files, git status/diff, test results, build outputs** to verify accomplishments
- **Compares accomplishments against requirements** systematically
- **Reports verification results with evidence** (what was verified, what's missing)
- **Provides STOP instructions** when failures are detected (blocks execution until fixed)
- **Read-only agent**: Never modifies files, commits changes, or makes autonomous decisions

**CRITICAL**: The step-verifier independently verifies your claims. You report accomplishments with evidence; the step-verifier verifies by reading files and checking outputs.

### Step Verification Process

After completing each step (before calling `checkpoint-update.sh`), you MUST:

1. **Report accomplishments with evidence**
   - Describe what was accomplished (files created/modified, commands executed, outputs produced)
   - Provide evidence (file paths, command outputs, git status/diff)
   - **DO NOT** tell step-verifier what to verify - report what was done

2. **Invoke step-verifier sub-agent**
   - Use `mcp_sub-agents_run_agent` tool with:
     - Agent: `"step-verifier"`
     - Prompt: See format below
     - The step-verifier will independently verify your claims

3. **Handle Verification Results**
  
   **If verification PASSES** (report shows "✅ STEP VERIFIED" or "All requirements met"):
     - Proceed to next step
     - Mark checkpoint step as complete (for progress tracking only - verification already done by step-verifier)
  
   **If verification FAILS** (report shows "⚠️ STOP EXECUTION" or missing elements):
     - **STOP execution immediately** - do not proceed to next step
     - Fix all reported issues from verification report
     - Re-invoke step-verifier with updated evidence after fixes
     - Only proceed after verification report shows "✅ STEP VERIFIED"
  
   **If sub-agent CANNOT be executed** (MCP unavailable or tool error):
     - STOP execution immediately
     - Inform user: "⚠️ Step verification sub-agent unavailable. Please manually verify Step N completion before proceeding."
     - Wait for explicit user confirmation before proceeding

### How to Invoke step-verifier

Use the `mcp_sub-agents_run_agent` tool:

```
Agent: "step-verifier"
Prompt: "Verify Step N: [Step Title] completion for migrate-module-to-zig-abi command.

Todo Item: [Description of what the step accomplished]
Command File: .opencode/command/ar/migrate-module-to-zig-abi.md
Step: Step N: [Step Title]

Accomplishment Report:
[Report what was accomplished with evidence: files created/modified, commands executed, outputs produced, etc. The step-verifier will independently verify these claims by reading files, checking git status, etc.]"
```

**CRITICAL**: 
- Report accomplishments with evidence, NOT instructions
- The step-verifier independently verifies by reading command files, checking files, git status/diff, etc.
- If step-verifier reports "⚠️ STOP EXECUTION", you MUST fix issues before proceeding


## MANDATORY: Initialize All Todo Items

**CRITICAL**: Before executing ANY steps, add ALL step and verification todo items to the session todo list using `todo_write`:

**Step and Verification Todo Items:**
- Add todo item: "Step 1: Check Existing" - Status: pending
- Add todo item: "Verify Step 1: Check Existing" - Status: pending
- Add todo item: "Step 2: Analyze Dependencies" - Status: pending
- Add todo item: "Verify Step 2: Analyze Dependencies" - Status: pending
- Add todo item: "Step 3: Identify Challenges" - Status: pending
- Add todo item: "Verify Step 3: Identify Challenges" - Status: pending
- Add todo item: "Step 4: Review API" - Status: pending
- Add todo item: "Verify Step 4: Review API" - Status: pending
- Add todo item: "Step 5: Create Zig File" - Status: pending
- Add todo item: "Verify Step 5: Create Zig File" - Status: pending
- Add todo item: "Step 6: Map Types" - Status: pending
- Add todo item: "Verify Step 6: Map Types" - Status: pending
- Add todo item: "Step 7: Implement Functions" - Status: pending
- Add todo item: "Verify Step 7: Implement Functions" - Status: pending
- Add todo item: "Step 8: Run Tests" - Status: pending
- Add todo item: "Verify Step 8: Run Tests" - Status: pending
- Add todo item: "Step 9: Verify Memory" - Status: pending
- Add todo item: "Verify Step 9: Verify Memory" - Status: pending
- Add todo item: "Step 10: Cleanup and Document" - Status: pending
- Add todo item: "Verify Step 10: Cleanup and Document" - Status: pending
- Add todo item: "Verify Complete Workflow: migrate-module-to-zig-abi" - Status: pending

**Important**: All todo items are initialized as `pending` and will be updated to `in_progress` when their respective step/verification begins, then to `completed` after verification passes.


### In-Progress Workflow Detection

If a `/migrate-module-to-zig-abi` workflow is already in progress:

```bash
./scripts/checkpoint-status.sh migrate-module-to-zig-abi --verbose
# Resume: ./scripts/checkpoint-update.sh migrate-module-to-zig-abi STEP=N
# Or reset: ./scripts/checkpoint-cleanup.sh migrate-module-to-zig-abi && ./scripts/checkpoint-init.sh migrate-module-to-zig-abi "Check Existing" "Analyze Dependencies" "Identify Challenges" "Review API" "Create Zig File" "Map Types" "Implement Functions" "Run Tests" "Verify Memory" "Cleanup and Document"
```

### First-Time Initialization Check

```bash
./scripts/checkpoint-init.sh migrate-module-to-zig-abi "Check Existing" "Analyze Dependencies" "Identify Challenges" "Review API" "Create Zig File" "Map Types" "Implement Functions" "Run Tests" "Verify Memory" "Cleanup and Document"
```

## PRECONDITION: Checkpoint Tracking Must Be Initialized

```bash
./scripts/checkpoint-require.sh migrate-module-to-zig-abi
```

# Migrate Module to Zig ABI
## Checkpoint Tracking

This command uses checkpoint tracking to ensure systematic C-to-Zig migration while maintaining C API compatibility. The process has 10 checkpoints across 4 phases with critical verification gates.

### Initialize Tracking
```bash
# Start the migration process (replace MODULE with actual module name)
MODULE={{1}}  # Set the module name
./scripts/checkpoint-init.sh migrate-module-to-zig-abi "Check Existing" "Analyze Dependencies" "Identify Challenges" "Review API" "Create Zig File" "Map Types" "Implement Functions" "Run Tests" "Verify Memory" "Cleanup and Document"
```

**Expected output:**
```
📍 Starting: migrate-module-to-zig-abi (10 steps)
📁 Tracking: /tmp/migrate-module-to-zig-abi-progress.txt
→ Run: ./scripts/checkpoint-update.sh migrate-module-to-zig-abi STEP=1
```

### Check Progress
```bash
./scripts/checkpoint-status.sh migrate-module-to-zig-abi
```

**Expected output (example at 50% completion):**
```
📈 command: X/Y steps (Z%)
   [████░░░░░░░░░░░░░░░░] Z%
→ Next: ./scripts/checkpoint-update.sh command STEP=N
```

## Minimum Requirements

**MANDATORY for successful migration:**
- [ ] Zero memory leaks after migration
- [ ] All tests pass
- [ ] C API compatibility maintained
- [ ] Documentation validates
- [ ] Clean build succeeds

{{#if 1}}
First, check if `modules/{{1}}.zig` already exists. If it does, inform the user about the migration status instead of creating a new plan.

If the module hasn't been migrated yet, migrate the implementation of the **{{1}}** module from C to Zig following the systematic approach documented in AGENTS.md and the comprehensive migration guide at kb/c-to-zig-module-migration.md. For complete integration details, see ([details](../../../kb/zig-integration-comprehensive.md)).
{{else}}
Before I can create a migration plan, I need to know which module you'd like to migrate to Zig.

Please provide the module name (without the `ar_` prefix). For example:
- `string` (for ar_string module)
- `expression_ast` (for ar_expression_ast module)
- `data` (for ar_data module)

Which module would you like to migrate to Zig?
{{/if}}

### Migration Requirements

1. **Follow the migration guide**: Use kb/c-to-zig-module-migration.md as your primary reference
2. **Maintain C API compatibility**: The Zig implementation must be a drop-in replacement
3. **Apply memory debugging patterns**: Use kb/zig-migration-memory-debugging.md for ownership issues
4. **Zero memory leaks**: Verify with individual test memory reports
5. **Validate documentation**: Ensure all documentation passes `make check-docs`

### Stage 1: Assessment (Steps 1-4)

#### [CHECKPOINT START - STAGE 1]

#### [CHECKPOINT END]

#### Pre-Migration Checklist

#### Step 1: Check Existing

{{#if 1}}
Before creating the plan, verify:

```bash
# Check if already migrated
if [ -f "modules/{{1}}.zig" ]; then
  if [ -f "modules/{{1}}.c" ]; then
    echo "⚠️ Migration in progress: Both .c and .zig files exist"
  else
    echo "✅ Already migrated: Only .zig file exists"
    exit 0
  fi
else
  echo "Ready for migration: No .zig file exists"
fi

# Verify source files exist
if [ ! -f "modules/{{1}}.c" ] || [ ! -f "modules/{{1}}.h" ]; then
  echo "❌ Source files not found"
  exit 1
fi

./scripts/checkpoint-update.sh migrate-module-to-zig-abi STEP=1
```

#### Step 2: Analyze Dependencies

```bash
# Audit dependencies
echo "Analyzing dependencies for {{1}} module:"
grep -n "#include.*ar_" modules/{{1}}.h modules/{{1}}.c | head -20

# Count dependencies
DEP_COUNT=$(grep -o "#include.*ar_" modules/{{1}}.h modules/{{1}}.c | wc -l)
echo "Total dependencies: $DEP_COUNT"
echo "DEP_COUNT=$DEP_COUNT" > /tmp/migration-stats.txt

./scripts/checkpoint-update.sh migrate-module-to-zig-abi STEP=2
```

#### Step 3: Identify Challenges

```bash
# Check for migration challenges
echo "Checking for migration challenges:"

# Variadic functions
if grep -q "\.\.\.)" modules/{{1}}.h; then
  echo "⚠️ Contains variadic functions - needs special handling"
fi

# Platform-specific code
if grep -q "#ifdef.*_WIN32\|#ifdef.*__linux__" modules/{{1}}.c; then
  echo "⚠️ Contains platform-specific code"
fi

# Complex macros
if grep -q "^#define.*\\\\$" modules/{{1}}.c; then
  echo "⚠️ Contains multi-line macros"
fi

./scripts/checkpoint-update.sh migrate-module-to-zig-abi STEP=3
```

#### Step 4: Review API

```bash
# Review current API
echo "Current API functions:"
grep "^ar_{{1}}__" modules/{{1}}.h | wc -l
grep "^ar_{{1}}__" modules/{{1}}.h | head -10

# Check ownership patterns
echo "\nOwnership patterns:"
grep -c "own_\|mut_\|ref_" modules/{{1}}.c

./scripts/checkpoint-update.sh migrate-module-to-zig-abi STEP=4
```

#### [ASSESSMENT GATE]
```bash
# Verify assessment is complete
./scripts/checkpoint-gate.sh migrate-module-to-zig-abi "Assessment" "1,2,3,4"
```

**Expected gate output:**
```
✅ GATE 'Assessment' - PASSED
   Verified: Steps 1,2,3,4
```

If `modules/{{1}}.zig` already exists:
- If `modules/{{1}}.c` also exists: "The {{1}} module migration to Zig appears to be in progress. The Zig file already exists alongside the C implementation."
- If `modules/{{1}}.c` doesn't exist: "The {{1}} module has already been migrated to Zig. The implementation is in `modules/{{1}}.zig`."
{{/if}}

### Stage 2: Implementation (Steps 5-7)

#### [CHECKPOINT START - STAGE 2]

#### [CHECKPOINT END]

### Implementation Strategy

#### Step 5: Create Zig File

```bash
# Create Zig file with proper structure
echo "Creating modules/{{1}}.zig..."
touch modules/{{1}}.zig

# Add basic structure (will be filled in during implementation)
echo "const std = @import(\"std\");" > modules/{{1}}.zig
echo "const ar_allocator = @import(\"ar_allocator.zig\");" >> modules/{{1}}.zig
echo "" >> modules/{{1}}.zig
echo "// Migrated from {{1}}.c - maintains C API compatibility" >> modules/{{1}}.zig

./scripts/checkpoint-update.sh migrate-module-to-zig-abi STEP=5
```

#### Step 6: Map Types

```bash
# Document type mappings
echo "Type mapping for {{1}} module:"
echo "- size_t → usize"
echo "- char* → [*c]u8 or [*:0]u8"
echo "- void* → ?*anyopaque"
echo "- bool → bool (C stdbool)"
echo "- int64_t → i64"

# Count functions to migrate
FUNC_COUNT=$(grep -c "^ar_{{1}}__" modules/{{1}}.h)
echo "Functions to migrate: $FUNC_COUNT"
echo "FUNC_COUNT=$FUNC_COUNT" >> /tmp/migration-stats.txt

./scripts/checkpoint-update.sh migrate-module-to-zig-abi STEP=6
```

#### Step 7: Implement Functions

```bash
# Track implementation progress
source /tmp/migration-stats.txt
echo "Implementing $FUNC_COUNT functions..."

# After implementing functions
echo "✅ Implemented all $FUNC_COUNT functions"
echo "Maintained C API compatibility with export fn"

./scripts/checkpoint-update.sh migrate-module-to-zig-abi STEP=7
```

#### [IMPLEMENTATION GATE]
```bash
# Verify implementation is complete
./scripts/checkpoint-gate.sh migrate-module-to-zig-abi "Implementation" "5,6,7"
```

**Expected gate output:**
```
✅ GATE 'Implementation' - PASSED
   Verified: Steps 5,6,7
```

### Stage 3: Testing (Steps 8-9)

#### [CHECKPOINT START - STAGE 3]

#### [CHECKPOINT END]

### Testing Approach

#### Step 8: Run Tests

```bash
# Rename C file to allow Zig to take over
mv modules/{{1}}.c modules/{{1}}.c.bak

# Run tests
echo "Running tests with Zig implementation..."
if ! make {{1}}_tests 2>&1; then
  echo "❌ Tests failed - need to debug"
  # Restore C file for comparison
  mv modules/{{1}}.c.bak modules/{{1}}.c
  exit 1
fi

echo "✅ All tests passed!"
./scripts/checkpoint-update.sh migrate-module-to-zig-abi STEP=8
```

#### Step 9: Verify Memory

```bash
# Check memory report
MEMORY_REPORT="bin/run-tests/memory_report_{{1}}_tests.log"
if [ -f "$MEMORY_REPORT" ]; then
  if grep -q "Actual memory leaks: 0" "$MEMORY_REPORT"; then
    echo "✅ No memory leaks detected"
  else
    echo "❌ Memory leaks detected:"
    grep "Actual memory leaks:" "$MEMORY_REPORT"
    exit 1
  fi
fi

./scripts/checkpoint-update.sh migrate-module-to-zig-abi STEP=9
```

#### [CRITICAL TESTING GATE]
```bash
# ⚠️ CRITICAL: Verify all tests pass with no leaks
./scripts/checkpoint-gate.sh migrate-module-to-zig-abi "Testing" "8,9"
```

**Expected gate output:**
```
✅ GATE 'Testing' - PASSED
   Verified: Steps 8,9
```

### Stage 4: Cleanup (Step 10)

#### [CHECKPOINT START - STAGE 4]

#### [CHECKPOINT END]

### Cleanup Plan

#### Step 10: Cleanup and Document

```bash
# Remove backup file
rm -f modules/{{1}}.c.bak

# Update documentation
echo "Updating documentation..."
if [ -f "modules/{{1}}.md" ]; then
  echo "- Added Zig implementation notes to modules/{{1}}.md"
fi

# Verify documentation
if ! make check-docs; then
  echo "❌ Documentation validation failed"
  exit 1
fi

# Final build verification
echo "Running final clean build..."
if ! make clean build 2>&1; then
  echo "❌ Build failed"
  exit 1
fi

echo "✅ Migration complete!"
./scripts/checkpoint-update.sh migrate-module-to-zig-abi STEP=10
```

#### [CHECKPOINT COMPLETE]
```bash
./scripts/checkpoint-complete.sh migrate-module-to-zig-abi
```

**Expected completion output:**
```
========================================
   CHECKPOINT COMPLETION SUMMARY
========================================

📈 migrate-module-to-zig-abi: X/Y steps (Z%)
   [████████████████████] 100%

✅ Checkpoint workflow complete
```
rm -f /tmp/migration-stats.txt
```

## Critical Reminders

- **All parameters must be nullable** for C API compatibility
- **Reserved keywords** like 'type' need renaming (e.g., 'node_type')
- **Match C behavior exactly** for ownership transfer functions ([details](../../../kb/zig-migration-memory-debugging.md))
- **Use C types directly** to minimize casting
- **Compare implementations line-by-line** when debugging
- **Memory allocation**: Use ar_allocator for type-safe operations ([details](../../../kb/zig-memory-allocation-with-ar-allocator.md))
- **Debug consistency**: Ensure -DDEBUG -D__ZIG__ flags ([details](../../../kb/zig-c-memory-tracking-consistency.md))
- **Build configuration**: Follow proper Zig build flags ([details](../../../kb/zig-build-flag-configuration.md))
- **Error cleanup**: Use errdefer but beware it doesn't work with `orelse return null` ([details](../../../kb/zig-errdefer-value-capture-pattern.md))

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
2. Zero memory leaks: Check `bin/run-tests/memory_report_{{1}}_tests.log`
3. Documentation valid: `make check-docs`
4. Clean build: `make clean build`
{{else}}
After migration, ensure all tests pass, zero memory leaks, documentation is valid, and clean build succeeds.
{{/if}}

Remember to follow TDD principles - complete any in-progress development cycles before starting the migration.

## Troubleshooting

### If tests fail after migration:
```bash
# Compare implementations side-by-side
diff -u modules/{{1}}.c.bak modules/{{1}}.zig

# Check specific function behavior
grep -A 10 "function_name" modules/{{1}}.c.bak
grep -A 10 "function_name" modules/{{1}}.zig
```

### Common migration issues:
1. **Null pointer handling**: Zig requires explicit optionals
2. **String ownership**: Use ar_allocator.strdup for owned strings
3. **Error unions**: Map C error returns to Zig error unions
4. **Reserved keywords**: Rename parameters like 'type' to 'node_type'

### If memory leaks occur:
```bash
# Check allocation/deallocation patterns
grep "AR__HEAP__MALLOC\|AR__HEAP__FREE" modules/{{1}}.c.bak
grep "allocator.alloc\|allocator.free" modules/{{1}}.zig

# Verify errdefer cleanup
grep "errdefer" modules/{{1}}.zig
```

### Recovery from failed migration:
```bash
# Restore original C file
mv modules/{{1}}.c.bak modules/{{1}}.c
rm modules/{{1}}.zig

# Reset checkpoint tracking
./scripts/checkpoint-cleanup.sh migrate-module-to-zig-abi
```

## Related Documentation

### Checkpoint Patterns
- [Multi-Step Checkpoint Tracking Pattern](../../../kb/multi-step-checkpoint-tracking-pattern.md)
- [Gate Enforcement Exit Codes Pattern](../../../kb/gate-enforcement-exit-codes-pattern.md)
- [Command Thoroughness Requirements Pattern](../../../kb/command-thoroughness-requirements-pattern.md)

### Zig Migration Patterns
- [C to Zig Module Migration](../../../kb/c-to-zig-module-migration.md)
- [Zig Integration Comprehensive](../../../kb/zig-integration-comprehensive.md)
- [Zig Migration Memory Debugging](../../../kb/zig-migration-memory-debugging.md)
- [Zig Memory Allocation with ar_allocator](../../../kb/zig-memory-allocation-with-ar-allocator.md)
- [Zig C Memory Tracking Consistency](../../../kb/zig-c-memory-tracking-consistency.md)
- [Zig Build Flag Configuration](../../../kb/zig-build-flag-configuration.md)
- [Zig Error Path Testing Pattern](../../../kb/zig-error-path-testing-pattern.md)
- [Zig Errdefer Value Capture Pattern](../../../kb/zig-errdefer-value-capture-pattern.md)