#!/usr/bin/env python3
"""Fix double _with_instance suffixes and other documentation issues."""

import os
import glob

# Files to fix
files_to_fix = [
    "README.md",
    "kb/*.md",
    "modules/README.md"
]

# Replacements to make
replacements = {
    # Fix double _with_instance
    "ar_agency__send_to_agent_with_instance_with_instance": "ar_agency__send_to_agent_with_instance",
    "ar_agency__create_agent_with_instance_with_instance": "ar_agency__create_agent_with_instance",
    "ar_agency__save_agents_with_instance_with_instance": "ar_agency__save_agents_with_instance",
    "ar_agency__get_agent_method_with_instance_with_instance": "ar_agency__get_agent_method_with_instance",

    # Fix functions that need _with_instance added
    "ar_agency__load_agents": "ar_agency__load_agents_with_instance",
    "ar_agency__get_first_agent": "ar_agency__get_first_agent_with_instance",
    "ar_agency__destroy_agent": "ar_agency__destroy_agent_with_instance",
    "ar_agency__update_agent_methods": "ar_agency__update_agent_methods_with_instance",
    "ar_agency__get_agent_method_with_instance_info": "ar_agency__get_agent_method_with_instance",
    "ar_agency__get_registry": "ar_agency__get_registry_with_instance"
}

# Process files
files_fixed = []
for pattern in files_to_fix:
    for filepath in glob.glob(pattern):
        try:
            with open(filepath, 'r') as f:
                content = f.read()

            original = content
            for old, new in replacements.items():
                content = content.replace(old, new)

            if content != original:
                with open(filepath, 'w') as f:
                    f.write(content)
                files_fixed.append(filepath)
        except Exception as e:
            print(f"Error processing {filepath}: {e}")

print(f"Fixed {len(files_fixed)} files:")
for f in sorted(files_fixed):
    print(f"  - {f}")