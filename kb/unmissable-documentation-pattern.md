# Unmissable Documentation Pattern

## Learning
Critical requirements can be easily overlooked when buried in lengthy documentation. Strategic placement and visual emphasis at the beginning of documents ensures mandatory steps are noticed and followed.

## Importance
Even well-documented requirements fail if users don't see them. Making requirements unmissable prevents errors, reduces support burden, and ensures consistent execution of critical processes.

## Example
```markdown
# Command Name
## MANDATORY CHECKPOINT TRACKING (REQUIRED)

**CRITICAL**: This command REQUIRES checkpoint tracking. You MUST use checkpoint tracking or the command will fail.

### REQUIRED: Initialize Tracking First
[Clear, unmissable instructions at the very top]

[Rest of documentation follows]
```

## Generalization
Place critical requirements:
1. At the very beginning of documentation
2. With clear visual markers (MANDATORY, CRITICAL, REQUIRED)
3. In dedicated sections before other content
4. With explicit consequences of non-compliance
5. Using multiple reinforcement points throughout

## Implementation
```bash
# Check if documentation makes requirements clear
grep -A 2 "^#" .opencode/command/ar/*.md | grep -i "mandatory\|critical\|required"

# Ensure critical sections appear early
head -20 documentation.md | grep -c "MANDATORY"
```

## Related Patterns
- [Gate Enforcement Exit Codes Pattern](gate-enforcement-exit-codes-pattern.md)
- [Command Output Documentation Pattern](command-output-documentation-pattern.md)
- [Documentation Standards Integration](documentation-standards-integration.md)