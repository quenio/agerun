# KB Link Fix Pattern

## Learning

When KB links are broken in documentation, search `kb/README.md` using grep to find correct article names rather than assuming paths or guessing article names. This prevents broken links and ensures discoverability.

## Importance

- **Prevents broken links**: Verifying article existence before referencing ensures links work
- **Ensures discoverability**: Searching README.md finds articles that are actually indexed
- **Prevents guesswork**: Don't assume article names or paths—verify they exist
- **Maintains documentation quality**: Broken links reduce trust and usability

## Example

**Broken link scenario:**

```markdown
# In .claude/step-verifier.md

# BAD: Assuming article name without verification
- See [Sub-Agents MCP Command Execution Capabilities](../../kb/sub-agents-mcp-command-execution-capabilities.md)

# Result: Link broken - article doesn't exist
```

**Correct fix pattern:**

```bash
# Step 1: Search kb/README.md for related keywords
grep -i "sub-agent\|mcp\|sub.*agent" kb/README.md

# Output shows:
# - [MCP Sub-Agent Integration Pattern](mcp-sub-agent-integration-pattern.md)
# - [Sub-Agent Verification Pattern](sub-agent-verification-pattern.md)

# Step 2: Verify article exists
ls -la kb/mcp-sub-agent-integration-pattern.md

# Step 3: Update link with correct article name
```

```markdown
# GOOD: Verified article name from README.md
- See [MCP Sub-Agent Integration Pattern](../kb/mcp-sub-agent-integration-pattern.md)

# Result: Link works - article exists and is indexed
```

**Real example from session:**

```bash
# Broken link found in .claude/step-verifier.md line 41
# Original: sub-agents-mcp-command-execution-capabilities.md (doesn't exist)

# Search for correct article:
grep -i "mcp\|sub-agent" kb/README.md

# Found: mcp-sub-agent-integration-pattern.md
# Verified: File exists
# Fixed: Updated link to use correct article name
```

## Generalization

**KB Link Fix Workflow:**

1. **Identify broken link**: Check validation errors or broken link reports
2. **Extract keywords**: From the broken link or context, identify search terms
3. **Search README.md**: Use grep with keywords to find related articles
   ```bash
   grep -i "keyword1\|keyword2" kb/README.md
   ```
4. **Verify article exists**: Check that the file actually exists
   ```bash
   ls -la kb/article-name.md
   ```
5. **Update link**: Use correct article name from README.md
6. **Verify fix**: Run `make check-docs` to confirm link is valid

**Common causes of broken links:**

- **Assumed article names**: Guessing article names without checking
- **Path errors**: Incorrect relative paths (too many `../` or wrong directory)
- **Renamed articles**: Articles renamed but references not updated
- **Non-existent articles**: Referencing articles that were never created

**Prevention strategies:**

- Always search `kb/README.md` before creating new KB references
- Use grep to find related articles by keyword
- Verify article existence before committing links
- Run `make check-docs` to catch broken links automatically

## Implementation

**Automated link verification:**

```bash
# Check for broken KB links in a file
check_kb_links() {
    local file="$1"
    grep -o '\[.*\](.*\.md)' "$file" | while read -r link; do
        # Extract path from markdown link
        path=$(echo "$link" | grep -o '([^)]*\.md)' | tr -d '()')
        
        # Check if it's a KB link
        if [[ "$path" == *"kb/"* ]]; then
            # Extract article name
            article=$(basename "$path")
            
            # Check if article exists
            if [ ! -f "kb/$article" ]; then
                echo "BROKEN LINK: $link in $file"
                echo "  Searching for alternatives..."
                grep -i "$(echo "$article" | sed 's/-/ /g' | cut -d. -f1)" kb/README.md
            fi
        fi
    done
}
```

**Manual verification workflow:**

```bash
# When fixing a broken link:

# 1. Extract keywords from broken link
BROKEN_LINK="sub-agents-mcp-command-execution-capabilities"
KEYWORDS="mcp sub-agent command execution"

# 2. Search README.md
grep -i "$KEYWORDS" kb/README.md

# 3. Verify found article exists
FOUND_ARTICLE="mcp-sub-agent-integration-pattern.md"
ls -la "kb/$FOUND_ARTICLE"

# 4. Update link with correct name
# Edit file, replace broken link with: [MCP Sub-Agent Integration Pattern](../kb/$FOUND_ARTICLE)

# 5. Verify fix
make check-docs
```

## Related Patterns

- [Markdown Link Resolution Patterns](markdown-link-resolution-patterns.md) - How to resolve relative paths correctly
- [Documentation Validation Enhancement Patterns](documentation-validation-enhancement-patterns.md) - Automated link checking
- [Knowledge Base Search Patterns](kb-search-patterns.md) - Finding KB articles by keyword
- [Search Result Completeness Verification](search-result-completeness-verification.md) - Complete searches prevent missing articles

