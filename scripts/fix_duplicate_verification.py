#!/usr/bin/env python3
"""
Fix duplicate Step 8 verification section in Step 9.
"""

with open('.opencode/command/ar/execute-plan.md', 'r') as f:
    lines = f.readlines()

# Find and remove the duplicate Step 8 verification section in Step 9
# It appears between "All tests passing: 12/12" and the correct Step 9 verification
output = []
i = 0
while i < len(lines):
    # Check if we're at the start of the duplicate section
    if (i < len(lines) - 5 and 
        'All tests passing: 12/12' in lines[i] and
        '**⚠️ MANDATORY STEP VERIFICATION**' in lines[i+2] and
        'Before proceeding to Step 9' in lines[i+5] and
        'Verify Step 8: Execute Iterations' in lines[i+6]):
        # Skip the duplicate section (until we find the correct Step 9 verification)
        # Look for the correct Step 9 verification section
        j = i + 2
        while j < len(lines):
            if ('**⚠️ MANDATORY STEP VERIFICATION**' in lines[j] and
                'Before proceeding to Step 10' in lines[j+3] and
                'Verify Step 9: Run Tests' in lines[j+4]):
                # Found the correct Step 9 verification, keep it
                output.append(lines[i])  # Keep "All tests passing: 12/12"
                output.append(lines[i+1])  # Keep "```"
                output.append(lines[i+2])  # Keep blank line
                # Skip to the correct verification section
                i = j
                break
            j += 1
        else:
            # Didn't find the correct section, keep everything
            output.append(lines[i])
            i += 1
    else:
        output.append(lines[i])
        i += 1

with open('.opencode/command/ar/execute-plan.md', 'w') as f:
    f.writelines(output)

print("Fixed duplicate verification section.")
