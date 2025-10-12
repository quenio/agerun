# Pre-Compact Hook KB Consultation

## Learning
Claude Code supports a PreCompact hook that runs before conversation context is compacted. This hook can automatically surface relevant KB articles about context preservation and session management to ensure important information is retained during compaction.

## Importance
- **Context preservation**: Reminds about best practices before critical information is compacted
- **Session continuity**: Surfaces patterns for maintaining context across session boundaries
- **Automatic consultation**: No manual KB search needed - happens automatically
- **Proactive guidance**: Provides reminders at exactly the right moment

## Example
When Claude Code's context window fills up and compaction is needed, the PreCompact hook automatically runs and displays:

```bash
========================================
   PRE-CONTEXT-COMPACT KB CONSULTATION
========================================

Context compaction detected. Reading relevant KB articles...

📋 Context & Session Management Articles:

📖 context-preservation-across-sessions
📖 session-resumption-without-prompting
📖 post-session-task-extraction-pattern

========================================
   KEY REMINDERS FOR CONTEXT COMPACT
========================================

✓ Preserve critical context about ongoing work
✓ Summarize completed tasks concisely
✓ Maintain checkpoint tracking state if active
✓ Keep references to KB articles consulted
✓ Document any pending decisions or blockers
```

## Implementation

### 1. Create Hook Script

Create `scripts/pre_compact_kb_reader.sh`:

```bash
#!/bin/bash
# Pre-Compact KB Reader Hook
# Reads relevant KB articles before Claude Code compacts conversation context

set -e

PROJECT_DIR="${CLAUDE_PROJECT_DIR:-$(pwd)}"

# Function to read and display KB article summary
read_kb_article() {
    local article="$1"
    local kb_file="$PROJECT_DIR/kb/${article}.md"

    if [ -f "$kb_file" ]; then
        echo "📖 ${article}"
        # Extract and display key sections
        awk '/^## Learning/,/^## / { print }' "$kb_file" | head -10
    fi
}

# Read session continuity articles
read_kb_article "context-preservation-across-sessions"
read_kb_article "session-resumption-without-prompting"
read_kb_article "post-session-task-extraction-pattern"

# Display key reminders
echo "✓ Preserve critical context about ongoing work"
echo "✓ Summarize completed tasks concisely"
echo "✓ Maintain checkpoint tracking state if active"

exit 0
```

### 2. Configure Hook in Settings

Add to `.claude/settings.json`:

```json
{
  "hooks": {
    "PreCompact": [
      {
        "hooks": [
          {
            "type": "command",
            "command": "bash scripts/pre_compact_kb_reader.sh"
          }
        ]
      }
    ]
  }
}
```

### 3. Make Script Executable

```bash
chmod +x scripts/pre_compact_kb_reader.sh
```

### 4. Test the Hook

```bash
# Test manually to verify output
bash scripts/pre_compact_kb_reader.sh
```

## Generalization

This pattern can be applied to any hook event in Claude Code:

- **PreToolUse**: Read KB articles before specific tool execution
- **UserPromptSubmit**: Surface relevant articles based on prompt keywords
- **SessionStart**: Load project-specific KB articles at session beginning
- **PostToolUse**: Verify actions against KB best practices

### Template for Other Hooks

```json
{
  "hooks": {
    "HookEventName": [
      {
        "matcher": "ToolName",  // Optional: filter by tool
        "hooks": [
          {
            "type": "command",
            "command": "bash scripts/your_hook_script.sh"
          }
        ]
      }
    ]
  }
}
```

## Benefits

1. **Zero friction**: No manual grep commands needed
2. **Perfect timing**: Guidance appears exactly when needed
3. **Consistent application**: Never forget to check KB before critical operations
4. **Scalable**: Easy to add more hooks for other events
5. **Educational**: Reinforces best practices through repetition

## Use Cases

- **Context compaction**: Remind about session continuity patterns
- **File writes**: Verify ownership and naming conventions
- **Commits**: Check pre-commit checklist items
- **Test execution**: Surface TDD and testing patterns
- **Documentation updates**: Remind about validation requirements

## Related Patterns
- [Context Preservation Across Sessions](context-preservation-across-sessions.md)
- [Session Resumption Without Prompting](session-resumption-without-prompting.md)
- [Post-Session Task Extraction Pattern](post-session-task-extraction-pattern.md)
- [Knowledge Base Consultation Protocol](knowledge-base-consultation-protocol.md)
- [Just-In-Time KB Reading](just-in-time-kb-reading.md)
