#!/usr/bin/env python3
"""
Update file_delegate_plan.md to add todo tracking and step-verifier verification
sections to all iterations following the latest create-plan template.
"""

import re
import sys

def get_iteration_number(iteration_header):
    """Extract iteration number from header like '#### Iteration 8.1.3:'"""
    match = re.search(r'Iteration\s+([\d.]+)', iteration_header)
    return match.group(1) if match else None

def add_todo_tracking_section(iteration_num, content, after_line):
    """Add the todo tracking section after the Objective/NOTE/Test File lines"""
    todo_section = f"""**⚠️ MANDATORY: Todo Item Tracking and Step-Verifier Verification**

When executing this iteration, follow these steps:

1. **Initialize todo items** before starting:
   - "Iteration {iteration_num} RED Phase" - Status: pending
   - "Verify Iteration {iteration_num} RED Phase" - Status: pending
   - "Iteration {iteration_num} GREEN Phase" - Status: pending
   - "Verify Iteration {iteration_num} GREEN Phase" - Status: pending
   - "Iteration {iteration_num} REFACTOR Phase" - Status: pending
   - "Verify Iteration {iteration_num} REFACTOR Phase" - Status: pending
   - "Iteration {iteration_num}: Update Plan Status" - Status: pending

2. **For each phase (RED, GREEN, REFACTOR)**:
   - Mark phase todo item as `in_progress` before starting
   - Complete the phase work
   - Invoke step-verifier sub-agent to verify phase completion
   - Mark verification todo item as `completed` after verification passes
   - Mark phase todo item as `completed` after verification passes
   - Only then proceed to next phase

3. **After all phases complete**:
   - Update plan status from PENDING REVIEW/REVIEWED/REVISED to IMPLEMENTED
   - Mark plan status update todo item as `completed`

"""
    return content[:after_line] + todo_section + content[after_line:]

def add_red_todo_status(iteration_num, content, red_phase_pos):
    """Add todo status update before RED Phase"""
    todo_status = f"""**MANDATORY: Update todo item status**
Before starting RED phase, update the todo item status to `in_progress`:
- Update todo item: "Iteration {iteration_num} RED Phase"
- Status: in_progress

"""
    return content[:red_phase_pos] + todo_status + content[red_phase_pos:]

def add_red_verification(iteration_num, content, before_green_pos):
    """Add step-verifier verification after RED Phase"""
    verification = f"""**⚠️ MANDATORY STEP VERIFICATION**

**MANDATORY: Update verification todo item status**

Before proceeding to GREEN phase, update the verification todo item status to `in_progress`:
- Update todo item: "Verify Iteration {iteration_num} RED Phase"
- Status: in_progress

Before proceeding to GREEN phase, you MUST verify RED phase completion via step-verifier sub-agent:

1. **Invoke step-verifier sub-agent** to verify:
   - Test written and added to test file
   - Test fails for the correct reason (matches plan's documented corruption)
   - RED phase objectives were met

2. **If verification fails**: Fix issues and re-verify before proceeding

3. **If sub-agent unavailable**: Stop and request user manual verification

**Only after step-verifier verification passes**:

1. **Mark verification complete in session todo list** using `todo_write`:
   - Update todo item: "Verify Iteration {iteration_num} RED Phase"
   - Status: completed

2. **Mark phase complete in session todo list** using `todo_write`:
   - Update todo item: "Iteration {iteration_num} RED Phase"
   - Status: completed

"""
    return content[:before_green_pos] + verification + content[before_green_pos:]

def add_green_todo_status(iteration_num, content, green_phase_pos):
    """Add todo status update before GREEN Phase"""
    todo_status = f"""**MANDATORY: Update todo item status**
Before starting GREEN phase, update the todo item status to `in_progress`:
- Update todo item: "Iteration {iteration_num} GREEN Phase"
- Status: in_progress

"""
    return content[:green_phase_pos] + todo_status + content[green_phase_pos:]

def add_green_verification(iteration_num, content, before_refactor_or_next_pos):
    """Add step-verifier verification after GREEN Phase"""
    verification = f"""**⚠️ MANDATORY STEP VERIFICATION**

**MANDATORY: Update verification todo item status**

Before proceeding to REFACTOR phase, update the verification todo item status to `in_progress`:
- Update todo item: "Verify Iteration {iteration_num} GREEN Phase"
- Status: in_progress

Before proceeding to REFACTOR phase, you MUST verify GREEN phase completion via step-verifier sub-agent:

1. **Invoke step-verifier sub-agent** to verify:
   - Implementation added to code
   - Test now passes
   - GREEN phase objectives were met

2. **If verification fails**: Fix issues and re-verify before proceeding

3. **If sub-agent unavailable**: Stop and request user manual verification

**Only after step-verifier verification passes**:

1. **Mark verification complete in session todo list** using `todo_write`:
   - Update todo item: "Verify Iteration {iteration_num} GREEN Phase"
   - Status: completed

2. **Mark phase complete in session todo list** using `todo_write`:
   - Update todo item: "Iteration {iteration_num} GREEN Phase"
   - Status: completed

**REFACTOR Phase (MANDATORY):**

**MANDATORY: Update todo item status**
Before starting REFACTOR phase, update the todo item status to `in_progress`:
- Update todo item: "Iteration {iteration_num} REFACTOR Phase"
- Status: in_progress

[Apply improvements if any, or state "No refactoring needed for this iteration"]

**⚠️ MANDATORY STEP VERIFICATION**

**MANDATORY: Update verification todo item status**

Before updating plan status, update the verification todo item status to `in_progress`:
- Update todo item: "Verify Iteration {iteration_num} REFACTOR Phase"
- Status: in_progress

Before updating plan status, you MUST verify REFACTOR phase completion via step-verifier sub-agent:

1. **Invoke step-verifier sub-agent** to verify:
   - REFACTOR phase completed (improvements applied or documented as not needed)
   - Tests still passing after refactoring
   - REFACTOR phase objectives were met

2. **If verification fails**: Fix issues and re-verify before proceeding

3. **If sub-agent unavailable**: Stop and request user manual verification

**Only after step-verifier verification passes**:

1. **Mark verification complete in session todo list** using `todo_write`:
   - Update todo item: "Verify Iteration {iteration_num} REFACTOR Phase"
   - Status: completed

2. **Mark phase complete in session todo list** using `todo_write`:
   - Update todo item: "Iteration {iteration_num} REFACTOR Phase"
   - Status: completed

"""
    return content[:before_refactor_or_next_pos] + verification + content[before_refactor_or_next_pos:]

def update_plan_file(file_path):
    """Update plan file with new template sections"""
    with open(file_path, 'r') as f:
        content = f.read()
    
    # Find all iteration headers
    iteration_pattern = r'(#### Iteration\s+([\d.]+):[^\n]+- (?:IMPLEMENTED|REVIEWED|✅ COMMITTED)\n)'
    iterations = list(re.finditer(iteration_pattern, content))
    
    print(f"Found {len(iterations)} iterations")
    
    # Process iterations in reverse order to preserve positions
    for match in reversed(iterations):
        iteration_num = match.group(2)
        iteration_header_pos = match.end()
        
        # Skip if already has todo tracking section
        if '⚠️ MANDATORY: Todo Item Tracking' in content[iteration_header_pos:iteration_header_pos+500]:
            print(f"  Iteration {iteration_num}: Already has todo tracking, skipping...")
            continue
        
        # Find RED Phase start
        red_phase_match = re.search(r'\n\*\*RED Phase:\*\*\n', content[iteration_header_pos:])
        if not red_phase_match:
            print(f"  Iteration {iteration_num}: Could not find RED Phase, skipping...")
            continue
        
        red_phase_pos = iteration_header_pos + red_phase_match.start() + 1
        
        # Find where to insert todo tracking (after Objective/NOTE/Test File, before RED Phase)
        # Look for last line before RED Phase
        insert_pos = red_phase_pos - len("**RED Phase:**\n")
        
        # Add todo tracking section
        content = add_todo_tracking_section(iteration_num, content, insert_pos)
        
        # Update positions after insertion
        red_phase_match = re.search(r'\n\*\*RED Phase:\*\*\n', content[insert_pos:])
        red_phase_pos = insert_pos + red_phase_match.start() + 1
        
        # Add RED todo status
        content = add_red_todo_status(iteration_num, content, red_phase_pos)
        
        # Find GREEN Phase
        green_phase_match = re.search(r'\n\*\*GREEN Phase:\*\*\n', content[red_phase_pos:])
        if green_phase_match:
            green_phase_pos = red_phase_pos + green_phase_match.start() + 1
            
            # Add RED verification before GREEN
            content = add_red_verification(iteration_num, content, green_phase_pos)
            
            # Update GREEN position
            green_phase_match = re.search(r'\n\*\*GREEN Phase:\*\*\n', content[green_phase_pos:])
            green_phase_pos = green_phase_pos + green_phase_match.start() + 1
            
            # Add GREEN todo status
            content = add_green_todo_status(iteration_num, content, green_phase_pos)
            
            # Find next iteration or end of GREEN section
            next_iteration_match = re.search(r'\n#### Iteration\s+', content[green_phase_pos:])
            verification_pos = green_phase_pos + next_iteration_match.start() if next_iteration_match else len(content)
            
            # Look for "---" separator before next iteration
            separator_match = re.search(r'\n---\n\n#### Iteration\s+', content[green_phase_pos:])
            if separator_match:
                verification_pos = green_phase_pos + separator_match.start() + 1
            
            # Add GREEN verification (includes REFACTOR)
            content = add_green_verification(iteration_num, content, verification_pos)
        
        print(f"  Updated Iteration {iteration_num}")
    
    # Write updated content
    with open(file_path, 'w') as f:
        f.write(content)
    
    print(f"\nUpdated {len(iterations)} iterations successfully!")

if __name__ == '__main__':
    file_path = 'plans/file_delegate_plan.md'
    if len(sys.argv) > 1:
        file_path = sys.argv[1]
    update_plan_file(file_path)

