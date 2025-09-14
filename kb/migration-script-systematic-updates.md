# Migration Script Systematic Updates

## Learning

When performing systematic codebase migrations (like API changes), creating specialized Python scripts is more reliable and maintainable than manual edits or one-off sed commands. These scripts document the migration process and can be reused or adapted for similar future migrations.

## Importance

Migration scripts provide:
- Consistency across all affected files
- Documentation of what was changed and why
- Ability to review changes before applying
- Reusability for similar future migrations
- Prevention of manual errors and omissions

## Example

```python
#!/usr/bin/env python3
# Example from update_test_system_calls.py

def update_system_calls(content):
    """Update global system calls to instance-based APIs"""
    replacements = [
        # Basic system operations
        ('ar_system__init()', 'ar_system__init_with_instance(mut_system)'),  // EXAMPLE: Global API no longer exists
        ('ar_system__shutdown()', 'ar_system__shutdown_with_instance(mut_system)'),  // EXAMPLE: Global API no longer exists
        
        # Agency operations through system
        ('ar_agency__create_agent_with_instance(', 
         'ar_agency__create_agent_with_instance(ar_system__get_agency(mut_system), '),
        
        # Methodology operations through agency
        ('ar_methodology__register_method(',
         'ar_methodology__register_method_with_instance('
         'ar_agency__get_methodology(ar_system__get_agency(mut_system)), ')
    ]
    
    for old, new in replacements:
        content = content.replace(old, new)
    
    return content

# Usage with ar_data_t for file handling
def process_file(filepath):
    # Read file content using actual ar_io functions
    FILE *fp = ar_io__open_file(filepath, "r");
    if (!fp) return;
    
    # Read content into string
    char buffer[4096];
    ar_data_t *own_content = ar_data__create_string("");
    while (ar_io__read_line(fp, buffer, sizeof(buffer), filepath)) {
        // Append to content
    }
    ar_io__close_file(fp);
    
    # Process content
    const char *ref_text = ar_data__get_string(own_content);
    // ... process text ...
    
    # Write back using ar_io__write_file
    ar_io__write_file(filepath, updated_text, strlen(updated_text));
    
    // Cleanup
    ar_data__destroy(own_content);
```

## Generalization

For any systematic codebase migration:
1. Identify all patterns that need updating
2. Create a Python script with clear replacement rules
3. Add dry-run mode to preview changes
4. Process files systematically with proper error handling
5. Preserve the script for documentation and future reference
6. Consider creating multiple focused scripts rather than one complex script

## Implementation

```python
#!/usr/bin/env python3
"""Template for migration scripts"""

import os
import sys
import argparse

def update_content(content, verbose=False):
    """Apply systematic updates to content"""
    original = content
    replacements = [
        # Define your replacements here
        ('old_pattern', 'new_pattern'),
    ]
    
    for old, new in replacements:
        if old in content:
            content = content.replace(old, new)
            if verbose:
                print(f"  Replaced: {old} -> {new}")
    
    return content, content != original

def main():
    parser = argparse.ArgumentParser(description='Systematic migration script')
    parser.add_argument('files', nargs='*', help='Files to process')
    parser.add_argument('--dry-run', action='store_true', help='Preview changes')
    parser.add_argument('--verbose', action='store_true', help='Show replacements')
    args = parser.parse_args()
    
    # Find files if none specified
    if not args.files:
        args.files = []
        for root, dirs, files in os.walk('modules'):
            for f in files:
                if f.endswith(('.c', '.h')):
                    args.files.append(os.path.join(root, f))
    
    for filepath in args.files:
        with open(filepath, 'r') as f:
            content = f.read()
        
        updated, changed = update_content(content, args.verbose)
        
        if changed:
            print(f"{'Would update' if args.dry_run else 'Updating'}: {filepath}")
            if not args.dry_run:
                with open(filepath, 'w') as f:
                    f.write(updated)

if __name__ == '__main__':
    main()
```

## Related Patterns
- [Script Enhancement Over One-Off](script-enhancement-over-one-off.md)
- [Progressive Tool Enhancement](progressive-tool-enhancement.md)
- [Systematic Function Renaming Pattern](systematic-function-renaming-pattern.md)
- [Global Function Removal Script Pattern](global-function-removal-script-pattern.md)
- [Compilation-Driven Refactoring Pattern](compilation-driven-refactoring-pattern.md)