Run static analysis on the test code.

## ⚠️ CRITICAL: Let the script manage checkpoints

**DO NOT manually initialize before running this command.** The script handles all checkpoint initialization, execution, and cleanup automatically. Just run the script and let it complete.

## Quick Start

```bash
./scripts/run-analyze-tests.sh
```

That's it! The script will handle everything automatically. Do not run any `make checkpoint-*` commands manually unless the script fails.

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

After completing each step, you MUST:

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
     -   
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
Prompt: "Verify Step N: [Step Title] completion for analyze-tests command.

Todo Item: [Description of what the step accomplished]
Command File: .opencode/command/ar/analyze-tests.md
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
- Add todo item: "Step 1: Build Tests" - Status: pending
- Add todo item: "Verify Step 1: Build Tests" - Status: pending
- Add todo item: "Step 2: Run Static Analysis" - Status: pending
- Add todo item: "Verify Step 2: Run Static Analysis" - Status: pending
- Add todo item: "Step 3: Report Results" - Status: pending
- Add todo item: "Verify Step 3: Report Results" - Status: pending
- Add todo item: "Verify Complete Workflow: analyze-tests" - Status: pending

**Important**: All todo items are initialized as `pending` and will be updated to `in_progress` when their respective step/verification begins, then to `completed` after verification passes.

## Workflow Execution

Run the complete workflow:

```bash
./scripts/run-analyze-tests.sh
```

This script handles all stages of the test static analysis process:

### What the Script Does

1. **Build Tests**: Compiles the test executable if needed
2. **Run Static Analysis**: Executes static analysis tools on the test code
3. **Report Results**: Presents analysis findings and code quality metrics
4. **Completion**: Marks the workflow as complete

## Troubleshooting

If the script fails, simply rerun it:

```bash
./scripts/run-analyze-tests.sh
```

## Minimum Requirements

**MANDATORY for successful completion:**
- [ ] Command executes without errors
- [ ] Expected output is produced
- [ ] No unexpected warnings or issues

**CRITICAL**: Test code quality matters as much as production code. Poor test quality leads to false confidence.

**Before running**: Review test coverage to identify gaps. The analyzer checks for:
- Memory leaks in test fixtures
- Improper cleanup
- Test logic errors
- Resource management issues

**Questions to consider**:
- Do all tests properly clean up resources?
- Are test assertions actually testing the right things?
- Is there dead code in the tests?

```bash
# Verify ready to execute
```

**Expected gate output:**
```
✅ GATE 'Ready' - PASSED
   Verified: Steps 1
```

## Command

```bash
make analyze-tests 2>&1

# Mark execution complete
```

## Expected Output

### Success State
```
Running static analysis on tests...
Analyzing ar_agent_tests.c...
Analyzing ar_agency_tests.c...
Analyzing ar_data_tests.c...
... (65 more test files)

Static analysis complete!
No issues found in test code.
```

### Warning States

**Unused Variables:**
```
Running static analysis on tests...
Analyzing ar_data_tests.c...
ar_data_tests.c:45:10: warning: unused variable 'result' [-Wunused-variable]
    int result = 42;
        ^~~~~~

Analyzing ar_string_tests.c...
ar_string_tests.c:123:15: warning: unused variable 'buffer' [-Wunused-variable]
    char buffer[256];
         ^~~~~~

Static analysis complete!
Found 2 warnings in test code.
```

**Potential NULL Dereference:**
```
Running static analysis on tests...
Analyzing ar_agent_tests.c...
ar_agent_tests.c:78:5: warning: Access to field 'id' results in a dereference of a null pointer
    agent->id = 123;
    ^~~~~~~~~

Static analysis complete!
Found 1 potential bug in test code.
make: *** [analyze-tests] Error 1
```

**Resource Leak:**
```
Running static analysis on tests...
Analyzing ar_data_tests.c...
ar_data_tests.c:156:1: warning: Potential leak of memory pointed to by 'data'
}
^

Static analysis complete!
Found 1 potential leak in test code.
```

## Key Points

- **Clang Static Analyzer** checks for bugs without running code
- **Common issues**: NULL dereferences, resource leaks, dead code
- **Test-specific checks**: Ensures test quality and correctness
- **Non-blocking warnings** but bugs should be fixed
- **Always use `2>&1`** to capture all analyzer output

## Test Quality Patterns

Beyond static analysis, ensure test quality by following these patterns:

- **Test complexity warning**: If tests have retry loops or complex state tracking, see [Test Complexity as Code Smell](../../../kb/test-complexity-as-code-smell.md)
- **Mock at the right level**: Avoid mocking system calls when function-level mocking suffices. See [Mock at Right Level Pattern](../../../kb/mock-at-right-level-pattern.md)
- **Check existing solutions**: Before creating complex test infrastructure, see how other tests solve similar problems. See [Check Existing Solutions First](../../../kb/check-existing-solutions-first.md)
- **Strong assertions**: Use AR_ASSERT with descriptive messages. See [Test Assertion Strength Patterns](../../../kb/test-assertion-strength-patterns.md)
- **Test effectiveness**: Verify tests actually catch failures. See [Test Effectiveness Verification](../../../kb/test-effectiveness-verification.md)