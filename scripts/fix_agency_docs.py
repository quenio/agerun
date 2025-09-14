#!/usr/bin/env python3
"""Fix ar_agency.md to use correct instance-based function names."""

import re

# Read the file
with open("modules/ar_agency.md", 'r') as f:
    content = f.read()

# Fix the duplicate _with_instance
content = content.replace(
    "ar_agency__create_agent_with_instance_with_instance",
    "ar_agency__create_agent_with_instance"
)

# Remove references to removed functions from Key Functions section
lines_to_remove = [
    "- `ar_agency__set_initialized(bool initialized)` - Initialize or shutdown the agency",
    "- `ar_agency__reset()` - Destroy all agents and clear registry",
    "- `ar_agency__destroy_agent()` - Destroy an agent by ID",
    "- `ar_agency__agent_has_messages()` - Check if agent has pending messages",
    "- `ar_agency__agent_exists()` - Check if an agent exists",
    "- `ar_agency__count_agents()` - Get total number of agents",
    "- `ar_agency__count_active_agents()` - Get number of active agents",
    "- `ar_agency__is_agent_active()` - Check if specific agent is active",
    "- `ar_agency__get_first_agent()` - Get first agent ID",
    "- `ar_agency__get_next_agent()` - Get next agent ID in iteration order",
    "- `ar_agency__get_agent_memory()` - Get agent's memory (read-only)",
    "- `ar_agency__get_agent_method_with_instance()` - Get agent's method reference",
    "- `ar_agency__get_agent_method_with_instance_info()` - Get method name and version",
    "- `ar_agency__update_agent_method()` - Update single agent's method",
    "- `ar_agency__update_agent_methods()` - Bulk update agents using a method",
    "- `ar_agency__set_agent_active()` - Change agent's active status",
    "- `ar_agency__set_agent_id()` - Change agent's ID (for persistence)",
    "- `ar_agency__save_agents_with_instance()` - Save all persistent agents to disk",
    "- `ar_agency__load_agents()` - Load agents from disk",
    "- `ar_agency__get_registry()` - Get registry reference (for internal modules)"
]

for line in lines_to_remove:
    content = content.replace(line + "\n", "")

# Update to use correct instance-based names
replacements = {
    "### Initialization and Reset\n\n": "### Initialization and Reset\n\n- `ar_agency__reset_with_instance()` - Destroy all agents and clear registry\n",

    "### Agent Creation and Destruction\n\n- `ar_agency__create_agent_with_instance()` - Create a new agent with automatic ID allocation\n":
    "### Agent Creation and Destruction\n\n- `ar_agency__create_agent_with_instance()` - Create a new agent with automatic ID allocation\n- `ar_agency__destroy_agent_with_instance()` - Destroy an agent by ID\n",

    "### Agent Communication\n\n- `ar_agency__send_to_agent_with_instance()` - Send a message to an agent\n":
    "### Agent Communication\n\n- `ar_agency__send_to_agent_with_instance()` - Send a message to an agent\n- `ar_agency__agent_has_messages_with_instance()` - Check if agent has pending messages\n",

    "### Agent Information\n\n":
    "### Agent Information\n\n- `ar_agency__agent_exists_with_instance()` - Check if an agent exists\n- `ar_agency__count_agents_with_instance()` - Get total number of agents\n",

    "### Agent Iteration\n\n":
    "### Agent Iteration\n\n- `ar_agency__get_first_agent_with_instance()` - Get first agent ID\n- `ar_agency__get_next_agent_with_instance()` - Get next agent ID in iteration order\n",

    "### Agent Properties\n\n- `ar_agency__get_agent_mutable_memory_with_instance()` - Get agent's memory (mutable)":
    "### Agent Properties\n\n- `ar_agency__get_agent_memory_with_instance()` - Get agent's memory (read-only)\n- `ar_agency__get_agent_mutable_memory_with_instance()` - Get agent's memory (mutable)"
}

for old, new in replacements.items():
    content = content.replace(old, new)

# Add back agent update functions in correct section
content = content.replace(
    "### Agent Updates\n\n",
    "### Agent Updates\n\n- `ar_agency__update_agent_methods_with_instance()` - Bulk update agents using a method\n- `ar_agency__count_agents_using_method_with_instance()` - Count agents using specific method\n"
)

# Fix persistence section
content = content.replace(
    "### Persistence\n\n",
    "### Persistence\n\n- `ar_agency__save_agents_with_instance()` - Save all persistent agents to disk\n- `ar_agency__load_agents_with_instance()` - Load agents from disk\n"
)

# Fix internal access
content = content.replace(
    "### Internal Access\n\n",
    "### Internal Access\n\n- `ar_agency__get_registry_with_instance()` - Get registry reference (for internal modules)\n"
)

# Write the fixed content
with open("modules/ar_agency.md", 'w') as f:
    f.write(content)

print("Fixed ar_agency.md documentation")