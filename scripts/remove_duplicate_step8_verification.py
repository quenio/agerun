#!/usr/bin/env python3
"""Remove duplicate Step 8 verification section that appears in Step 9."""

with open('.opencode/command/ar/execute-plan.md', 'r') as f:
    content = f.read()

# Pattern to match: from "All tests passing: 12/12" through duplicate Step 8 verification
# until the correct Step 9 verification (which mentions "Step 10")
import re

pattern = r'(All tests passing: 12/12\n```\n\n)(\*\*⚠️ MANDATORY STEP VERIFICATION\*\*\n\n\*\*MANDATORY: Add verification to session todo list\*\*\n\nBefore proceeding to Step 9.*?\./scripts/checkpoint-update\.sh execute-plan STEP=8\n```\n\n\n\n)(\*\*⚠️ MANDATORY STEP VERIFICATION\*\*\n\n\*\*MANDATORY: Add verification to session todo list\*\*\n\nBefore proceeding to Step 10)'

replacement = r'\1\3'

content = re.sub(pattern, replacement, content, flags=re.DOTALL)

with open('.opencode/command/ar/execute-plan.md', 'w') as f:
    f.write(content)

print('Removed duplicate Step 8 verification section from Step 9')
