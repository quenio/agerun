#!/usr/bin/env python3
"""
Update execute-plan.md to add step-verifier verification sections to Steps 2-12.
This script adds the same verification pattern as Step 1 to all remaining steps.
"""

import re
import sys

def update_step(file_path):
    with open(file_path, 'r') as f:
        content = f.read()
    
    # Template for step todo addition (before step work)
    step_todo_template = """**MANDATORY: Add step to session todo list**

Before starting this step, add it to the session todo list using `todo_write`:
- Add todo item: "Step {step_num}: {step_title}"
- Status: in_progress

"""
    
    # Template for verification section
    verification_template = """**⚠️ MANDATORY STEP VERIFICATION**

**MANDATORY: Add verification to session todo list**

Before proceeding to Step {next_step}, add verification to the session todo list using `todo_write`:
- Add todo item: "Verify Step {step_num}: {step_title}"
- Status: in_progress

Before proceeding to Step {next_step}, you MUST verify Step {step_num} completion via step-verifier sub-agent:

1. **Invoke step-verifier sub-agent** to verify:
   - {verification_points}
   - Step objectives were met

2. **If verification fails**: Fix issues and re-verify before proceeding

3. **If sub-agent unavailable**: Stop and request user manual verification

**Only after step-verifier verification passes** (checkpoint-update is for progress tracking only, NOT verification):

1. **Mark verification complete in session todo list** using `todo_write`:
   - Update todo item: "Verify Step {step_num}: {step_title}"
   - Status: completed

2. **Mark step complete in session todo list** using `todo_write`:
   - Update todo item: "Step {step_num}: {step_title}"
   - Status: completed

3. **Update checkpoint** (for progress tracking only):
```bash
./scripts/checkpoint-update.sh execute-plan STEP={step_num}
```

"""
    
    # Step-specific verification points
    step_verification_points = {
        2: "- Plan document was read completely\n   - Key information extracted (iteration count, cycle structure, module names)\n   - Test and implementation function names identified",
        3: "- Plan validator was executed\n   - Plan validation passed (all 14 lessons verified)\n   - Validator output shows ✅ Plan validation PASSED",
        4: "- IMPLEMENTED iteration check was executed\n   - Count of IMPLEMENTED iterations determined\n   - List of IMPLEMENTED iterations extracted (if any found)",
        5: "- Code existence verified for all IMPLEMENTED iterations\n   - Test and implementation files read and verified\n   - Git status checked and classification completed (CASE 1, 2, or 3)\n   - Classification decision documented",
        6: "- COMMITTED iteration check was executed\n   - All COMMITTED iterations verified (code exists, tests pass, in git history)\n   - Verification results documented (PASSED or FAILED with details)",
        7: "- extract-tdd-cycles.sh was executed\n   - REVIEWED/REVISED iterations extracted and listed\n   - Total count of iterations to implement determined\n   - Extraction checklist completed",
        8: "- All REVIEWED/REVISED iterations executed with RED-GREEN-REFACTOR cycles\n   - All iterations marked as IMPLEMENTED in plan file\n   - All tests passing after each iteration\n   - Iteration checkpoint tracking completed",
        9: "- Complete test suite was executed\n   - All tests passing (0 failures)\n   - No compilation errors\n   - No runtime errors\n   - Test count matches plan iteration count",
        10: "- Memory reports checked for all test modules\n   - All reports show: \"Actual memory leaks: 0 (0 bytes)\"\n   - No leaks detected in any iteration\n   - Memory verification checklist completed",
        11: "- All IMPLEMENTED iterations updated to ✅ COMMITTED (Step 11A)\n   - Plan file updated with status markers\n   - Completion status header added if applicable (Step 11B)\n   - Plan status update checklist completed",
        12: "- Execution summary generated with all required sections\n   - Status updates documented\n   - Execution metrics included\n   - TDD methodology compliance verified (all 14 lessons)\n   - Test execution results documented\n   - Build verification documented\n   - Plan file updates documented\n   - Files modified listed\n   - Completion checklist completed"
    }
    
    # Step titles
    step_titles = {
        2: "Read Plan",
        3: "Validate Plan Compliance",
        4: "Check for IMPLEMENTED Iterations",
        5: "Verify IMPLEMENTED Iterations",
        6: "Verify COMMITTED Iterations",
        7: "Extract REVIEWED or REVISED iterations",
        8: "Execute Iterations",
        9: "Run Tests",
        10: "Verify Memory",
        11: "Update Plan Status",
        12: "Summary"
    }
    
    # Update each step
    for step_num in range(2, 13):
        step_title = step_titles[step_num]
        next_step = step_num + 1 if step_num < 12 else "completion"
        
        # Pattern to find step header
        step_header_pattern = rf'(#### Step {step_num}: {re.escape(step_title)}\n\n)'
        
        # Check if step already has todo section
        if re.search(rf'{step_header_pattern}.*?MANDATORY: Add step to session todo list', content, re.DOTALL):
            print(f"Step {step_num} already has todo section, skipping...")
            continue
        
        # Add todo section after step header
        step_todo = step_todo_template.format(step_num=step_num, step_title=step_title)
        content = re.sub(step_header_pattern, rf'\1{step_todo}', content)
        
        # Find where to insert verification section
        # For Step 2, it's after "Expected implementation function names"
        # For other steps, it's before the checkpoint-update command or next step header
        
        if step_num == 2:
            # Remove checkpoint-update from code block
            content = re.sub(
                r'(# <use Read tool with plan file path>\n\n)(\./scripts/checkpoint-update\.sh execute-plan STEP=2\n```)',
                r'\1```',
                content
            )
            # Add verification before Step 3
            pattern = r'(- Expected implementation function names\n\n)(#### Step 3:)'
            replacement = rf'\1{verification_template.format(step_num=step_num, step_title=step_title, next_step=next_step, verification_points=step_verification_points[step_num])}\2'
            content = re.sub(pattern, replacement, content)
        
        elif step_num == 3:
            # Remove checkpoint-update and add verification
            pattern = r'(\*\*If validator passes:\*\*\n\n)(```bash\n\./scripts/checkpoint-update\.sh execute-plan STEP=3\n```\n\n---)'
            replacement = rf'\1**⚠️ MANDATORY STEP VERIFICATION**\n\n**MANDATORY: Add verification to session todo list**\n\nBefore proceeding to Step {next_step}, add verification to the session todo list using `todo_write`:\n- Add todo item: "Verify Step {step_num}: {step_title}"\n- Status: in_progress\n\nBefore proceeding to Step {next_step}, you MUST verify Step {step_num} completion via step-verifier sub-agent:\n\n1. **Invoke step-verifier sub-agent** to verify:\n   - {step_verification_points[step_num]}\n   - Step objectives were met\n\n2. **If verification fails**: Fix issues and re-verify before proceeding\n\n3. **If sub-agent unavailable**: Stop and request user manual verification\n\n**Only after step-verifier verification passes** (checkpoint-update is for progress tracking only, NOT verification):\n\n1. **Mark verification complete in session todo list** using `todo_write`:\n   - Update todo item: "Verify Step {step_num}: {step_title}"\n   - Status: completed\n\n2. **Mark step complete in session todo list** using `todo_write`:\n   - Update todo item: "Step {step_num}: {step_title}"\n   - Status: completed\n\n3. **Update checkpoint** (for progress tracking only):\n```bash\n./scripts/checkpoint-update.sh execute-plan STEP={step_num}\n```\n\n---'
            content = re.sub(pattern, replacement, content)
        
        else:
            # For other steps, find checkpoint-update and replace with verification + checkpoint-update
            pattern = rf'(```bash\n\./scripts/checkpoint-update\.sh execute-plan STEP={step_num}\n```)'
            if re.search(pattern, content):
                replacement = verification_template.format(
                    step_num=step_num,
                    step_title=step_title,
                    next_step=next_step,
                    verification_points=step_verification_points[step_num]
                )
                content = re.sub(pattern, replacement, content)
            else:
                # If no checkpoint-update found, add before next step header
                next_step_header = rf'#### Step {next_step}:'
                pattern = rf'(\n\n)({next_step_header})'
                replacement = rf'\1{verification_template.format(step_num=step_num, step_title=step_title, next_step=next_step, verification_points=step_verification_points[step_num])}\2'
                content = re.sub(pattern, replacement, content, count=1)
        
        print(f"Updated Step {step_num}: {step_title}")
    
    # Write updated content
    with open(file_path, 'w') as f:
        f.write(content)
    
    print("\nAll steps updated successfully!")

if __name__ == '__main__':
    file_path = '.opencode/command/ar/execute-plan.md'
    update_step(file_path)
