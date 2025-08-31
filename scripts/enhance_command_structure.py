#!/usr/bin/env python3
"""
Enhance all command documentation to follow the comprehensive structure
exemplified by new-learnings.md.

This script adds missing structural elements while preserving existing content.
"""

import os
import json
import re

commands_dir = '.claude/commands'

# Load the fix report
with open('/tmp/structure_fix_report.json', 'r') as f:
    fix_report = json.load(f)

def enhance_simple_command(filepath, filename, missing_elements):
    """Enhance a simple command to have comprehensive structure."""
    with open(filepath, 'r') as f:
        lines = f.readlines()
    
    # Find where to insert new content (after the h1 title)
    h1_line = -1
    for i, line in enumerate(lines):
        if line.startswith('# ') and i > 0:  # Skip first line description
            h1_line = i
            break
    
    if h1_line == -1:
        print(f"  ⚠️ Could not find h1 title in {filename}")
        return False
    
    # Extract command name from filename
    cmd_name = filename.replace('.md', '').replace('-', '_')
    cmd_display = filename.replace('.md', '').replace('-', ' ').title()
    
    # Prepare new sections to add
    new_sections = []
    
    # Add Checkpoint Tracking section if missing
    if "Missing tracking_system section" in missing_elements:
        new_sections.append(f"""## Checkpoint Tracking

This command uses checkpoint tracking to ensure systematic execution and verification.

### Initialize Tracking
```bash
# Start the {cmd_display.lower()} process
make checkpoint-init CMD={cmd_name} STEPS='"Prepare" "Execute" "Verify"'
```

**Expected output:**
```
========================================
   CHECKPOINT TRACKING INITIALIZED
========================================

Command: {cmd_name}
Tracking file: /tmp/{cmd_name}_progress.txt
Total steps: 3

Steps to complete:
  1. Prepare
  2. Execute
  3. Verify

Goal: Complete {cmd_display.lower()} successfully
```

### Check Progress
```bash
make checkpoint-status CMD={cmd_name}
```

**Expected output (example at 33% completion):**
```
========================================
   CHECKPOINT STATUS: {cmd_name}
========================================

Progress: 1/3 steps (33%)

[██████░░░░░░░░░░░░] 33%

Current Status: Preparing...

Next Action:
  → Step 2: Execute
```

## Minimum Requirements

**MANDATORY for successful completion:**
- [ ] Command executes without errors
- [ ] Expected output is produced
- [ ] No unexpected warnings or issues

""")
    
    # Find where existing h2 sections start
    first_h2_line = -1
    for i in range(h1_line + 1, len(lines)):
        if lines[i].startswith('## '):
            first_h2_line = i
            break
    
    # Insert new sections
    if new_sections:
        insertion_point = first_h2_line if first_h2_line != -1 else h1_line + 1
        
        # Add a blank line after h1 if needed
        if insertion_point == h1_line + 1 and not lines[h1_line].endswith('\n\n'):
            new_sections.insert(0, '\n')
        
        # Insert the new content
        for section in new_sections:
            lines.insert(insertion_point, section)
    
    # Add checkpoint markers to existing sections
    # Find Command section and add markers
    for i in range(len(lines)):
        if lines[i].strip() == '## Command':
            # Add checkpoint start marker
            if i + 1 < len(lines):
                lines.insert(i + 1, '\n#### [CHECKPOINT START - EXECUTION]\n\n')
            
            # Find the end of this section and add end marker
            for j in range(i + 2, len(lines)):
                if lines[j].startswith('## '):
                    lines.insert(j, '\n#### [CHECKPOINT END - EXECUTION]\n')
                    break
            break
    
    # Write back the enhanced file
    with open(filepath, 'w') as f:
        f.writelines(lines)
    
    return True

def enhance_checkpoint_command(filepath, filename, missing_elements):
    """Enhance an existing checkpoint command with missing elements."""
    with open(filepath, 'r') as f:
        content = f.read()
    
    # These commands already have good structure, just need minor additions
    modifications = []
    
    if "No end markers" in missing_elements:
        # Find checkpoint starts without corresponding ends
        pattern = r'#### \[CHECKPOINT START[^\]]*\]'
        starts = re.finditer(pattern, content)
        for match in starts:
            start_pos = match.end()
            # Look for the next section or checkpoint
            next_section = re.search(r'\n##|#### \[CHECKPOINT', content[start_pos:])
            if next_section:
                insert_pos = start_pos + next_section.start()
                # Check if there's already an end marker
                check_area = content[start_pos:insert_pos]
                if 'CHECKPOINT END' not in check_area:
                    modifications.append((insert_pos, '\n#### [CHECKPOINT END]\n'))
    
    # Apply modifications in reverse order to maintain positions
    for pos, text in sorted(modifications, reverse=True):
        content = content[:pos] + text + content[pos:]
    
    with open(filepath, 'w') as f:
        f.write(content)
    
    return True

# Process all commands that need fixing
print("=" * 80)
print("ENHANCING COMMAND STRUCTURE")
print("=" * 80)

enhanced_count = 0
failed_count = 0

for filename, missing_elements in fix_report['needs_fixing']:
    filepath = os.path.join(commands_dir, filename)
    
    # Determine command type based on current score
    score = fix_report['scores'][filename]
    
    print(f"\nProcessing {filename} (score: {score}%)...")
    
    try:
        if score < 50:
            # These are simple commands that need major enhancement
            if enhance_simple_command(filepath, filename, missing_elements):
                print(f"  ✅ Enhanced with checkpoint structure")
                enhanced_count += 1
            else:
                print(f"  ❌ Failed to enhance")
                failed_count += 1
        else:
            # These already have structure, just need minor fixes
            if enhance_checkpoint_command(filepath, filename, missing_elements):
                print(f"  ✅ Fixed missing markers")
                enhanced_count += 1
            else:
                print(f"  ❌ Failed to fix")
                failed_count += 1
    except Exception as e:
        print(f"  ❌ Error: {e}")
        failed_count += 1

print("\n" + "=" * 80)
print("ENHANCEMENT SUMMARY")
print("=" * 80)
print(f"\n✅ Successfully enhanced: {enhanced_count} commands")
if failed_count > 0:
    print(f"❌ Failed to enhance: {failed_count} commands")

print("\nRun 'make check-command-structure' to verify improvements")