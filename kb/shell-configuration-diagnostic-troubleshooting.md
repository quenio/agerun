# Shell Configuration Diagnostic Troubleshooting

## Learning
Diagnostic tools may have specific detection methods that distinguish between functionally equivalent configurations. A working configuration (alias/PATH) can trigger diagnostic warnings if placed in shell files that the diagnostic doesn't check or recognize.

## Importance
Understanding diagnostic tool detection methods prevents wasting time "fixing" working configurations. Instead of assuming the configuration is wrong when diagnostics report issues, investigate how the diagnostic tool performs its checks to identify the real cause.

## Example
```bash
# Working configuration that triggered diagnostic warning
# .zprofile (login shell)
export PATH="$PATH:$HOME/.claude/local"
alias claude="~/.claude/local/claude"

# Diagnostic complained: "Local installation not accessible"
# Even though this worked:
which claude  # claude: aliased to ~/.claude/local/claude

# Solution: Move to .zshrc (interactive shell)
# .zshrc
export PATH="$PATH:$HOME/.claude/local"
alias claude="~/.claude/local/claude"

# Diagnostic now passes
```

With AgeRun context:
```c
// When diagnostic tools check AgeRun configuration
ar_data_t* own_config_path = ar_data__create_string("~/.claude/local/claude");

// Tool may check specific shell contexts
ar_data_t* own_shell_context = ar_data__create_string("interactive");
if (strcmp(ar_data__get_string(own_shell_context), "interactive") == 0) {
    // Diagnostic looks in .zshrc, not .zprofile
    ar_log__info(context, "Configuration found in interactive shell context");
}

ar_data__destroy(own_config_path);
ar_data__destroy(own_shell_context);
```

## Generalization
**Diagnostic Tool Troubleshooting Pattern:**
1. **Verify functionality first** - confirm the configuration actually works
2. **Understand detection method** - how does the diagnostic find configurations?
3. **Match detection context** - place config where the tool looks
4. **Test the diagnostic** - verify the warning resolves

**Shell File Selection Strategy:**
- `.zprofile` - System-wide environment setup, login shells
- `.zshrc` - Interactive tools, aliases, functions
- Diagnostics may prefer interactive shell contexts for tool detection

## Implementation
```bash
# Diagnostic troubleshooting workflow
echo "=== Step 1: Verify Configuration Works ==="
which tool_name
tool_name --version

echo "=== Step 2: Check Diagnostic Behavior ==="
tool_name/doctor # or equivalent diagnostic command

echo "=== Step 3: Try Configuration Migration ==="
# Move config from .zprofile to .zshrc or vice versa

echo "=== Step 4: Re-test Diagnostic ==="
source ~/.zshrc  # or start new terminal
tool_name/doctor
```

## Related Patterns
- [Shell Loading Order and Tool Detection](shell-loading-order-tool-detection.md)
- [Configuration Migration Troubleshooting Strategy](configuration-migration-troubleshooting-strategy.md)
- [Development Debug Tools](development-debug-tools.md)
- [Makefile POSIX Shell Compatibility](makefile-posix-shell-compatibility.md)