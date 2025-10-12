#!/bin/bash
# Pre-Compact KB Reader Hook
# Reads relevant KB articles before Claude Code compacts conversation context

set -e

# Project directory
PROJECT_DIR="${CLAUDE_PROJECT_DIR:-$(pwd)}"

# Color codes for better visibility
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo ""
echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}   PRE-CONTEXT-COMPACT KB CONSULTATION${NC}"
echo -e "${BLUE}========================================${NC}"
echo ""
echo "Context compaction detected. Reading relevant KB articles..."
echo ""

# Function to read and display KB article summary
read_kb_article() {
    local article="$1"
    local kb_file="$PROJECT_DIR/kb/${article}.md"

    if [ -f "$kb_file" ]; then
        echo -e "${GREEN}📖 ${article}${NC}"
        echo ""
        # Extract Learning and Importance sections
        awk '
            BEGIN { in_learning = 0; in_importance = 0; line_count = 0 }
            /^## Learning$/ { in_learning = 1; next }
            /^## Importance$/ { in_importance = 1; in_learning = 0; next }
            /^## / { in_learning = 0; in_importance = 0 }
            in_learning || in_importance {
                if (line_count < 20) {
                    print "  " $0
                    if (length($0) > 0) line_count++
                }
            }
        ' "$kb_file"
        echo ""
    fi
}

echo "📋 Context & Session Management Articles:"
echo ""

# Read session continuity articles
read_kb_article "context-preservation-across-sessions"
read_kb_article "session-resumption-without-prompting"
read_kb_article "post-session-task-extraction-pattern"

echo ""
echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}   KEY REMINDERS FOR CONTEXT COMPACT${NC}"
echo -e "${BLUE}========================================${NC}"
echo ""
echo "✓ Preserve critical context about ongoing work"
echo "✓ Summarize completed tasks concisely"
echo "✓ Maintain checkpoint tracking state if active"
echo "✓ Keep references to KB articles consulted"
echo "✓ Document any pending decisions or blockers"
echo ""
echo -e "${GREEN}✅ These KB articles were consulted before compaction.${NC}"
echo ""

exit 0
