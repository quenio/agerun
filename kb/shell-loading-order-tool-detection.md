# Shell Loading Order and Tool Detection

## Learning
Shell configuration files load in specific order and contexts: `.zprofile` for login shells, `.zshrc` for interactive shells. Tools and diagnostics may check different shell contexts, affecting whether configurations are detected even when functionally equivalent.

## Importance
Understanding shell loading order prevents configuration issues and diagnostic false positives. Different shell contexts affect not just functionality, but also tool detection behavior. This knowledge is essential for systematic troubleshooting of shell-related issues.

## Example
```bash
# Shell loading order for zsh:
# 1. /etc/zshenv
# 2. ~/.zshenv
# 3. /etc/zprofile (login shells)
# 4. ~/.zprofile (login shells)
# 5. /etc/zshrc (interactive shells)
# 6. ~/.zshrc (interactive shells)
# 7. /etc/zlogin (login shells)
# 8. ~/.zlogin (login shells)

# Terminal applications are usually BOTH login and interactive
# So they load: ~/.zprofile then ~/.zshrc

# Configuration placement affects tool detection:
# .zprofile - loaded early, good for environment variables
export JAVA_HOME=$(/usr/libexec/java_home -v 21)
export PATH="$PATH:/opt/homebrew/bin"

# .zshrc - loaded for interactive shells, good for interactive tools
alias ll='ls -la'
export PATH="$PATH:$HOME/.local/bin"  # Appends to existing PATH
```

AgeRun context example:
```c
// Tool detection in different shell contexts
ar_data_t* own_shell_type = ar_data__create_string("interactive");
ar_data_t* own_config_file = ar_data__create_string(".zshrc");

// Different shells check different contexts
if (strcmp(ar_data__get_string(own_shell_type), "login") == 0) {
    ar_data__destroy(own_config_file);
    own_config_file = ar_data__create_string(".zprofile");
}

ar_log__info(context, "Checking configuration in shell context");

ar_data__destroy(own_shell_type);
ar_data__destroy(own_config_file);
```

## Generalization
**Shell Context Decision Matrix:**
- **Login Shell**: System login, SSH, terminal app startup
- **Interactive Shell**: Command prompt available for user input
- **Non-interactive**: Scripts, cron jobs, automated processes

**Configuration Placement Strategy:**
- **Environment variables**: `.zprofile` - set once, inherited by all processes
- **Interactive tools**: `.zshrc` - aliases, functions, interactive-specific settings
- **Tool diagnostics**: Often expect interactive shell context (`.zshrc`)

**Tool Detection Patterns:**
- Some diagnostics only check interactive shell configurations
- Others may check login shell environment setup
- Path order matters: later files can override earlier settings

## Implementation
```bash
# Check current shell context
echo "Login shell: $0"  # Shows shell type
echo "Interactive: $-"  # Shows shell options

# Test shell loading order
echo "Testing shell file loading..."
echo "export TEST_ZPROFILE='loaded'" >> ~/.zprofile
echo "export TEST_ZSHRC='loaded'" >> ~/.zshrc

# Start new shell and check
zsh -l -c 'echo "zprofile: $TEST_ZPROFILE, zshrc: $TEST_ZSHRC"'

# Clean up
sed -i.bak '/TEST_ZPROFILE/d' ~/.zprofile
sed -i.bak '/TEST_ZSHRC/d' ~/.zshrc
rm -f ~/.zprofile.bak ~/.zshrc.bak

# Debug shell configuration loading
zsh -x ~/.zprofile  # Show execution trace
zsh -x ~/.zshrc
```

## Related Patterns
- [Shell Configuration Diagnostic Troubleshooting](shell-configuration-diagnostic-troubleshooting.md)
- [Configuration Migration Troubleshooting Strategy](configuration-migration-troubleshooting-strategy.md)
- [Makefile POSIX Shell Compatibility](makefile-posix-shell-compatibility.md)
- [Development Debug Tools](development-debug-tools.md)