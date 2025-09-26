# Configuration Migration Troubleshooting Strategy

## Learning
Moving working configurations between shell files can resolve tool detection issues without changing functionality. This systematic migration approach helps isolate whether the problem is configuration content or configuration location.

## Importance
Configuration migration is a powerful troubleshooting technique when tools report issues with working configurations. It separates location-dependent problems from content problems, leading to faster resolution and better understanding of tool behavior.

## Example
```bash
# Before migration - working but diagnostic complains
# ~/.zprofile
export PATH="$PATH:$HOME/.claude/local"
alias claude="~/.claude/local/claude"

# Verify it works
which claude  # claude: aliased to ~/.claude/local/claude
claude --version  # Works fine

# But diagnostic reports: "Local installation not accessible"

# Migration strategy - move to ~/.zshrc
# Step 1: Remove from original location
sed -i.bak '/Claude Code/,/^$/d' ~/.zprofile

# Step 2: Add to new location
cat >> ~/.zshrc << 'EOF'

# Claude Code
export PATH="$PATH:$HOME/.claude/local"
alias claude="~/.claude/local/claude"
EOF

# Step 3: Test in new shell
zsh -c 'which claude'  # Verify functionality preserved

# Step 4: Test diagnostic
claude/doctor  # Warning resolved
```

AgeRun context example:
```c
// Configuration migration tracking
ar_data_t* own_config_content = ar_data__create_string("export PATH=\"$PATH:$HOME/.claude/local\"");
ar_data_t* own_source_file = ar_data__create_string("~/.zprofile");
ar_data_t* own_target_file = ar_data__create_string("~/.zshrc");

// Track migration steps
ar_log__info(context, "Migrating configuration from source to target");

// Verify content preservation
ar_data_t* own_verification_cmd = ar_data__create_string("which claude");
ar_log__info(context, "Configuration migrated successfully");

ar_data__destroy(own_config_content);
ar_data__destroy(own_source_file);
ar_data__destroy(own_target_file);
ar_data__destroy(own_verification_cmd);
```

## Generalization
**Migration Troubleshooting Process:**
1. **Document current state** - capture working configuration
2. **Backup original** - create .bak files before changes
3. **Move systematically** - remove from source, add to target
4. **Test functionality** - ensure original behavior preserved
5. **Test diagnostic** - verify the issue is resolved
6. **Clean up backups** - remove .bak files after verification

**Common Migration Scenarios:**
- `.zprofile` → `.zshrc` for interactive tool diagnostics
- `.zshrc` → `.zprofile` for system-wide environment setup
- Global `/etc/` → user-specific `~/` for permission issues
- Shell-specific → universal shell compatibility

**Migration Decision Criteria:**
- **Diagnostic behavior**: Where does the tool look?
- **Tool usage context**: Login vs interactive shells
- **Persistence needs**: Per-session vs permanent
- **User vs system scope**: Local vs global configuration

## Implementation
```bash
# Generic configuration migration script
migrate_shell_config() {
    local content="$1"
    local source_file="$2"
    local target_file="$3"
    local marker="$4"

    echo "=== Configuration Migration ==="
    echo "From: $source_file"
    echo "To: $target_file"
    echo "Content: $content"

    # Step 1: Backup source
    cp "$source_file" "${source_file}.bak"

    # Step 2: Remove from source (using marker if provided)
    if [ -n "$marker" ]; then
        sed -i.tmp "/$marker/,/^$/d" "$source_file"
    fi

    # Step 3: Add to target
    echo "" >> "$target_file"
    echo "$content" >> "$target_file"

    # Step 4: Verify migration
    echo "=== Verification ==="
    echo "Source file after removal:"
    tail -5 "$source_file"
    echo "Target file after addition:"
    tail -5 "$target_file"

    # Clean up temp files
    rm -f "${source_file}.tmp"
}

# Usage example
migrate_shell_config \
    "# Claude Code\nexport PATH=\"\$PATH:\$HOME/.claude/local\"" \
    "$HOME/.zprofile" \
    "$HOME/.zshrc" \
    "Claude Code"
```

## Related Patterns
- [Shell Configuration Diagnostic Troubleshooting](shell-configuration-diagnostic-troubleshooting.md)
- [Shell Loading Order and Tool Detection](shell-loading-order-tool-detection.md)
- [Development Debug Tools](development-debug-tools.md)
- [Systematic Guideline Enhancement](systematic-guideline-enhancement.md)