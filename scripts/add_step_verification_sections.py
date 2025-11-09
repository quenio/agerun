#!/usr/bin/env python3
"""
Add STEP VERIFICATION ENFORCEMENT and MANDATORY: Initialize All Todo Items sections
to commands that are missing them.

This script:
1. Reads each command file
2. Checks if STEP VERIFICATION ENFORCEMENT section exists
3. Checks if MANDATORY: Initialize All Todo Items section exists
4. Extracts step names from checkpoint-init.sh commands
5. Adds missing sections with proper step names
"""

import re
import sys
from pathlib import Path

# Template for STEP VERIFICATION ENFORCEMENT section
STEP_VERIFICATION_TEMPLATE = """## STEP VERIFICATION ENFORCEMENT

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
Prompt: "Verify Step N: [Step Title] completion for {command_name} command.

Todo Item: [Description of what the step accomplished]
Command File: .opencode/command/ar/{command_name}.md
Step: Step N: [Step Title]

Accomplishment Report:
[Report what was accomplished with evidence: files created/modified, commands executed, outputs produced, etc. The step-verifier will independently verify these claims by reading files, checking git status, etc.]"
```

**CRITICAL**: 
- Report accomplishments with evidence, NOT instructions
- The step-verifier independently verifies by reading command files, checking files, git status/diff, etc.
- If step-verifier reports "⚠️ STOP EXECUTION", you MUST fix issues before proceeding
"""

# Template for MANDATORY: Initialize All Todo Items section
def generate_todo_items_section(command_name, steps):
    """Generate the MANDATORY: Initialize All Todo Items section with step names."""
    todo_items = []
    for i, step in enumerate(steps, 1):
        todo_items.append(f'- Add todo item: "Step {i}: {step}" - Status: pending')
        todo_items.append(f'- Add todo item: "Verify Step {i}: {step}" - Status: pending')
    todo_items.append(f'- Add todo item: "Verify Complete Workflow: {command_name}" - Status: pending')
    
    return f"""## MANDATORY: Initialize All Todo Items

**CRITICAL**: Before executing ANY steps, add ALL step and verification todo items to the session todo list using `todo_write`:

**Step and Verification Todo Items:**
{chr(10).join(todo_items)}

**Important**: All todo items are initialized as `pending` and will be updated to `in_progress` when their respective step/verification begins, then to `completed` after verification passes.
"""

def extract_steps_from_checkpoint_init(content, command_name):
    """Extract step names from checkpoint-init.sh commands in the file or wrapper scripts."""
    steps = []
    
    # First, try to find checkpoint-init commands directly in the file
    # Pattern: checkpoint-init.sh COMMAND_NAME "Step 1" "Step 2" ...
    # Also handles multi-line format with escaped quotes
    for line in content.split('\n'):
        if 'checkpoint-init.sh' in line:
            # Check if this line is for our command (command name appears in the line)
            # Handle both formats: "compact-changes" and compact-changes (without quotes)
            if command_name in line or command_name.replace('-', '_') in line:
                # Extract all double-quoted strings from the line
                step_matches = re.findall(r'"([^"]+)"', line)
                if step_matches:
                    steps = step_matches
                    break
    
    # If not found in file, check wrapper scripts
    if not steps:
        # Try common wrapper script patterns
        script_patterns = [
            f"scripts/run-{command_name}.sh",
            f"scripts/run-{command_name.replace('-', '_')}.sh",
        ]
        
        for script_path_str in script_patterns:
            script_path = Path(script_path_str)
            if script_path.exists():
                try:
                    script_content = script_path.read_text()
                    script_steps = extract_steps_from_script(script_content)
                    if script_steps:
                        steps = script_steps
                        break
                except Exception as e:
                    print(f"  Error reading {script_path_str}: {e}")
                    continue
    
    return steps

def extract_steps_from_script(script_content):
    """Extract step names from a wrapper script's checkpoint-init.sh call."""
    # Find the line with checkpoint-init.sh
    for line in script_content.split('\n'):
        if 'checkpoint-init.sh' in line:
            # Extract all double-quoted strings from the line
            # Handles both formats:
            #   checkpoint-init.sh COMMAND '"Step 1" "Step 2" ...'
            #   checkpoint-init.sh COMMAND "Step 1" "Step 2" ...
            steps = re.findall(r'"([^"]+)"', line)
            if steps:
                return steps
    
    return []

def update_command_file(file_path, command_name):
    """Update a command file with missing sections."""
    content = file_path.read_text()
    
    # Check if sections already exist
    has_step_verification = "## STEP VERIFICATION ENFORCEMENT" in content
    has_todo_items = "## MANDATORY: Initialize All Todo Items" in content
    
    if has_step_verification and has_todo_items:
        print(f"✅ {command_name}: Both sections already exist")
        return False
    
    # Extract steps
    steps = extract_steps_from_checkpoint_init(content, command_name)
    if not steps:
        print(f"⚠️  {command_name}: Could not extract steps, skipping")
        return False
    
    print(f"📝 {command_name}: Found {len(steps)} steps: {', '.join(steps[:3])}...")
    
    # Find insertion point after CHECKPOINT WORKFLOW ENFORCEMENT
    checkpoint_pattern = r'(## CHECKPOINT WORKFLOW ENFORCEMENT\n\n\*\*CRITICAL\*\*:.*?\n)'
    match = re.search(checkpoint_pattern, content, re.DOTALL)
    
    if not match:
        # Try simpler pattern
        checkpoint_pattern = r'(## CHECKPOINT WORKFLOW ENFORCEMENT\n\n.*?\n)'
        match = re.search(checkpoint_pattern, content, re.DOTALL)
    
    if not match:
        print(f"⚠️  {command_name}: Could not find CHECKPOINT WORKFLOW ENFORCEMENT section")
        return False
    
    insertion_point = match.end()
    
    # Build new content
    new_content = content[:insertion_point]
    
    # Update CHECKPOINT WORKFLOW ENFORCEMENT to mention step-verifier
    if "step-verifier" not in match.group(1):
        new_content = new_content.replace(
            "**CRITICAL**: This command MUST use checkpoint tracking for ALL execution.",
            "**CRITICAL**: This command MUST use checkpoint tracking for ALL execution. All verification is done via step-verifier sub-agent, NOT via checkpoint scripts ([details](../../../kb/checkpoint-tracking-verification-separation.md))."
        )
    
    # Add STEP VERIFICATION ENFORCEMENT if missing
    if not has_step_verification:
        step_verification = STEP_VERIFICATION_TEMPLATE.format(command_name=command_name)
        new_content += "\n" + step_verification + "\n"
    
    # Add MANDATORY: Initialize All Todo Items if missing
    if not has_todo_items:
        todo_section = generate_todo_items_section(command_name, steps)
        new_content += "\n" + todo_section + "\n"
    
    # Add rest of content
    new_content += content[insertion_point:]
    
    # Write back
    file_path.write_text(new_content)
    return True

def main():
    """Main function to update all command files."""
    commands_dir = Path(".opencode/command/ar")
    
    # Commands that need updates
    commands_to_update = [
        "analyze-exec",
        "analyze-tests",
        "build-clean",
        "build",
        "compact-changes",
        "compact-guidelines",
        "check-module-consistency",
        "migrate-module-to-zig-abi",
        "migrate-module-to-zig-struct",
        "review-changes",
        "run-exec",
        "run-tests",
        "sanitize-exec",
        "sanitize-tests",
        "tsan-exec",
        "tsan-tests",
    ]
    
    updated_count = 0
    skipped_count = 0
    
    for command_name in commands_to_update:
        file_path = commands_dir / f"{command_name}.md"
        if not file_path.exists():
            print(f"⚠️  {command_name}: File not found")
            continue
        
        if update_command_file(file_path, command_name):
            updated_count += 1
        else:
            skipped_count += 1
    
    print(f"\n✅ Updated {updated_count} files")
    print(f"⏭️  Skipped {skipped_count} files (already have sections or couldn't extract steps)")

if __name__ == "__main__":
    main()
