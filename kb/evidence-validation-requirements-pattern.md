# Evidence Validation Requirements Pattern

## Learning
Verification systems must validate that all evidence provided in accomplishment reports actually exists and is valid, not just accept claims. Evidence validation requires checking file existence, path correctness, line number accuracy, git diff matching, command output validation, and test result validation.

## Importance
Without evidence validation, accomplishment reports can contain fabricated or inaccurate evidence, undermining verification quality. User feedback revealed that vague reports were being accepted without validation, leading to verification failures. Evidence validation ensures that step-verifier can independently verify all claims by checking that evidence actually exists and matches reality.

## Example
**Session context**: User feedback "Also make sure the step-verifier sub-agent checks the existence and validity of the provided evidence" led to adding evidence validation requirements.

**Before (no validation)**:
```markdown
Accomplishment Report:
Updated execute-plan.md to remove checkpoint references. Please verify the changes are correct.
```
**Problem**: Step-verifier accepts claim without validating that file exists, changes match, or references were actually removed.

**After (with validation)**:
```markdown
Accomplishment Report:
Updated `.opencode/command/ar/execute-plan.md` line 2356: `git diff` shows lines changed from `### If progress tracking` to `### If step tracking`. Verification: `grep -i 'checkpoint' .opencode/command/ar/execute-plan.md` returned no matches (exit code 1)
```
**Validation**: Step-verifier checks:
- File exists at `.opencode/command/ar/execute-plan.md` ✓
- Line 2356 contains claimed content ✓
- `git diff` matches claimed changes ✓
- `grep` output matches claim (no matches, exit code 1) ✓

## Generalization
**Evidence Validation Checklist** (apply to EVERY accomplishment report):

1. **File Path Validation**:
   - Verify file exists at exact path mentioned
   - Verify line numbers exist in file
   - Execute `git diff` to verify changes match claims
   - STOP if file doesn't exist or path incorrect

2. **Command Output Validation**:
   - Verify output is plausible (not fabricated)
   - Verify exit codes match expected values
   - Verify test files exist and results match claims
   - STOP if output appears fabricated

3. **Git Status/Diff Validation**:
   - Execute `git status` and `git diff` to verify
   - Verify files mentioned as modified appear in git status
   - Verify git diff output matches claims
   - STOP if git status/diff doesn't match claims

4. **Line Number Validation**:
   - Read file and verify line numbers contain claimed content
   - Verify line numbers are accurate (not off-by-one errors)
   - STOP if line numbers are incorrect

5. **Content Validation**:
   - Read file and verify content matches claims
   - Compare claimed changes against actual file content
   - STOP if content doesn't match

6. **Test Result Validation**:
   - Verify test files exist and contain claimed results
   - Verify test names match actual test functions
   - Verify pass/fail counts match actual test output
   - STOP if test results don't match

**STOP Instructions**: When evidence validation fails, verification system must STOP execution and require accurate evidence before proceeding.

## Implementation
**Step-verifier evidence validation process** (from `.claude/step-verifier.md` lines 122-168):

```markdown
4. **⚠️ MANDATORY: Validate Evidence Existence and Validity**
   **CRITICAL**: You MUST verify that all evidence provided in the accomplishment report actually exists and is valid:
   
   - **File Path Validation**: 
     - If accomplishment report mentions a file path, verify the file exists at that exact path
     - If line numbers are mentioned, verify those lines exist in the file
     - If git diff is mentioned, execute `git diff` yourself to verify the changes match what was claimed
     - **STOP if file doesn't exist or path is incorrect**: "STOP: Evidence validation failed. File '[path]' mentioned in accomplishment report does not exist or path is incorrect."
   
   - **Command Output Validation**:
     - If accomplishment report includes command output, verify the output is plausible (not fabricated)
     - If exit codes are mentioned, verify they match expected values
     - If test results are mentioned, verify test files exist and results match claims
     - **STOP if output appears fabricated or doesn't match reality**: "STOP: Evidence validation failed. Command output in accomplishment report does not match actual execution results."
   
   - **Git Status/Diff Validation**:
     - If accomplishment report mentions `git status` or `git diff`, execute these commands yourself to verify
     - Verify that files mentioned as modified actually appear in git status
     - Verify that git diff output matches what was claimed in the accomplishment report
     - **STOP if git status/diff doesn't match claims**: "STOP: Evidence validation failed. Git status/diff does not match claims in accomplishment report. Actual status: [your git status output]"
   
   - **Line Number Validation**:
     - If accomplishment report mentions specific line numbers, read the file and verify those lines contain what was claimed
     - Verify line numbers are accurate (not off-by-one errors)
     - **STOP if line numbers are incorrect**: "STOP: Evidence validation failed. Line [N] in file '[path]' does not contain claimed content. Actual content: [what you found]"
   
   - **Content Validation**:
     - If accomplishment report claims specific content changes, read the file and verify the content matches
     - Compare claimed changes against actual file content
     - **STOP if content doesn't match**: "STOP: Evidence validation failed. File '[path]' content does not match claims. Expected: [claimed content], Actual: [actual content]"
   
   - **Test Result Validation**:
     - If accomplishment report mentions test results, verify test files exist and contain the claimed results
     - Verify test names match actual test functions
     - Verify pass/fail counts match actual test output
     - **STOP if test results don't match**: "STOP: Evidence validation failed. Test results in accomplishment report do not match actual test execution. Expected: [claimed], Actual: [actual]"
```

**Evidence validation in verification reports**:
- Report evidence validation status (what was verified, what failed validation)
- Include specific validation failures with evidence
- STOP execution when validation fails
- Require accurate evidence before proceeding

## Common Validation Failures
1. **Fabricated evidence**: Evidence doesn't exist or doesn't match reality
2. **Incorrect paths**: File paths are wrong or files don't exist
3. **Wrong line numbers**: Line numbers don't contain claimed content
4. **Mismatched git diff**: Git diff output doesn't match claims
5. **Fabricated command output**: Command output appears fabricated or doesn't match execution
6. **Mismatched test results**: Test results don't match actual test execution

## Prevention Strategy
1. **Require concrete evidence**: Accomplishment reports must include concrete evidence (file paths, line numbers, git diff, command outputs, test results)
2. **Validate all evidence**: Verification system must validate all evidence existence and validity
3. **STOP on failure**: When validation fails, STOP execution and require accurate evidence
4. **Report validation status**: Include evidence validation status in verification reports

## Related Patterns
- [Sub-Agent Verification Pattern](sub-agent-verification-pattern.md) - Evidence-based reporting requirements
- [MCP Sub-Agent Integration Pattern](mcp-sub-agent-integration-pattern.md) - Sub-agent invocation and result interpretation
- [Evidence-Based Debugging](evidence-based-debugging.md) - Using evidence to verify claims
- [Checkpoint Work Verification Antipattern](checkpoint-work-verification-antipattern.md) - Consequences of accepting claims without validation

