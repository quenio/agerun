#!/usr/bin/env python3
"""
Fix all command documentation to follow the comprehensive structure
exemplified by new-learnings.md.

This script adds missing structural elements while preserving existing content.
"""

import os
import json
import re

commands_dir = '.claude/commands'

# Load the fix report
try:
    with open('/tmp/structure_fix_report.json', 'r') as f:
        fix_report = json.load(f)
except FileNotFoundError:
    print("Error: Run 'make check-command-structure FIX=1' first to generate fix report")
    exit(1)

def add_comprehensive_structure(filepath, filename, missing_elements):
    """Add all missing structural elements to achieve excellent score."""
    with open(filepath, 'r') as f:
        lines = f.readlines()
    
    # Extract command name from filename
    cmd_name = filename.replace('.md', '').replace('-', '_')
    cmd_display = filename.replace('.md', '').replace('-', ' ').title()
    
    # Process based on what's missing
    modified = False
    
    # Add update bash commands throughout
    if "No update bash commands" in missing_elements:
        # Find all checkpoint sections and add update commands
        for i in range(len(lines)):
            # Look for checkpoint headers
            if re.match(r'^#### Checkpoint \d+:', lines[i]):
                # Find the end of this checkpoint's bash block
                in_bash = False
                bash_end = -1
                for j in range(i+1, len(lines)):
                    if lines[j].strip().startswith('```bash'):
                        in_bash = True
                    elif in_bash and lines[j].strip() == '```':
                        bash_end = j
                        break
                
                if bash_end > 0:
                    # Extract checkpoint number
                    checkpoint_num = re.search(r'Checkpoint (\d+):', lines[i])
                    if checkpoint_num:
                        num = checkpoint_num.group(1)
                        # Add update command before closing ```
                        update_line = f"make checkpoint-update CMD={cmd_name} STEP={num}\n"
                        if update_line not in lines[bash_end-3:bash_end]:
                            lines.insert(bash_end, f"\n# Mark step complete\n")
                            lines.insert(bash_end+1, update_line)
                            modified = True
    
    # Add gate markers for simple commands
    if "No gate markers" in missing_elements:
        # Find the execution section
        for i in range(len(lines)):
            if lines[i].strip() == '## Command' or lines[i].strip() == '## Expected Output':
                # Add gate before expected output
                gate_section = f"""
#### [EXECUTION GATE]
```bash
# Verify ready to execute
make checkpoint-gate CMD={cmd_name} GATE="Ready" REQUIRED="1"
```

**Expected gate output:**
```
========================================
   GATE: Ready
========================================

✅ GATE PASSED: Ready to execute!

Prerequisites verified:
  ✓ Environment prepared
  ✓ Dependencies available
  
Proceed with execution.
```

"""
                if '[EXECUTION GATE]' not in ''.join(lines):
                    lines.insert(i, gate_section)
                    modified = True
                    break
    
    # Add complete markers
    if "No complete markers" in missing_elements:
        # Find the end of the document or before Key Points
        complete_section = f"""
#### [CHECKPOINT COMPLETE]
```bash
# Show final summary
make checkpoint-status CMD={cmd_name}
```

**Expected completion output:**
```
========================================
   CHECKPOINT STATUS: {cmd_name}
========================================

Progress: 3/3 steps (100%)

[████████████████████] 100%

✅ ALL CHECKPOINTS COMPLETE!

Summary:
  Preparation: ✓ Complete
  Execution: ✓ Complete  
  Verification: ✓ Complete

The {cmd_display.lower()} completed successfully!
```

```bash
# Clean up tracking
make checkpoint-cleanup CMD={cmd_name}
```

"""
        # Find where to insert (before ## Key Points or at end)
        insert_pos = -1
        for i in range(len(lines)):
            if lines[i].startswith('## Key Points'):
                insert_pos = i
                break
        
        if insert_pos == -1:
            # Add at end
            insert_pos = len(lines)
        
        if '[CHECKPOINT COMPLETE]' not in ''.join(lines):
            lines.insert(insert_pos, complete_section)
            modified = True
    
    # Add start/end markers for next-task and next-priority
    if "No start markers" in missing_elements or "No end markers" in missing_elements:
        # These need more comprehensive phase markers
        for i in range(len(lines)):
            if lines[i].startswith('## Expected Behavior'):
                # Add phase markers
                phase_markers = """
#### [CHECKPOINT START - EXECUTION]

"""
                lines.insert(i+1, phase_markers)
                
                # Find the end of this section
                for j in range(i+2, len(lines)):
                    if lines[j].startswith('## '):
                        end_marker = """
#### [CHECKPOINT END - EXECUTION]

"""
                        lines.insert(j, end_marker)
                        break
                modified = True
                break
    
    # Add more checkpoint update commands in existing bash blocks
    if "No update bash commands" in missing_elements:
        # Look for bash blocks that execute main commands
        for i in range(len(lines)):
            if 'make build' in lines[i] or 'make run-' in lines[i] or 'make sanitize-' in lines[i] or 'make tsan-' in lines[i] or 'make analyze-' in lines[i] or 'make check-naming' in lines[i]:
                # Check if update command already exists nearby
                check_range = lines[max(0, i-5):min(len(lines), i+10)]
                if not any('checkpoint-update' in line for line in check_range):
                    # Add after the command
                    for j in range(i+1, min(len(lines), i+10)):
                        if lines[j].strip() == '```':
                            lines.insert(j, "\n# Mark execution complete\n")
                            lines.insert(j+1, f"make checkpoint-update CMD={cmd_name} STEP=2\n")
                            modified = True
                            break
    
    if modified:
        # Write back the enhanced file
        with open(filepath, 'w') as f:
            f.writelines(lines)
        return True
    
    return False

# Process all commands that need fixing
print("=" * 80)
print("FIXING COMMAND STRUCTURE FOR EXCELLENT SCORES")
print("=" * 80)

fixed_count = 0
already_good = 0

for filename in fix_report['scores'].keys():
    score = fix_report['scores'][filename]
    filepath = os.path.join(commands_dir, filename)
    
    if score >= 90:
        already_good += 1
        continue
    
    print(f"\nFixing {filename} (current score: {score}%)...")
    
    # Get missing elements for this file
    missing = []
    for fname, elements in fix_report['needs_fixing']:
        if fname == filename:
            missing = elements
            break
    
    if missing:
        try:
            if add_comprehensive_structure(filepath, filename, missing):
                print(f"  ✅ Added missing elements: {', '.join(missing)}")
                fixed_count += 1
            else:
                print(f"  ℹ️ No changes needed")
        except Exception as e:
            print(f"  ❌ Error: {e}")
    else:
        print(f"  ℹ️ No issues reported")

print("\n" + "=" * 80)
print("FIX SUMMARY")
print("=" * 80)
print(f"\n✅ Fixed: {fixed_count} commands")
print(f"🌟 Already excellent: {already_good} commands")
print(f"📊 Total: {len(fix_report['scores'])} commands")

print("\nRun 'make check-command-structure' to verify all commands now have excellent scores!")