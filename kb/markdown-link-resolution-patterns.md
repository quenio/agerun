# Markdown Link Resolution Patterns

## Learning
Markdown file links must be validated according to how they are actually resolved by GitHub, VS Code, and other markdown viewers. Understanding the distinction between relative and absolute paths is crucial for creating portable documentation that works consistently across different platforms.

## Importance
Incorrect link resolution leads to broken documentation, frustrated users, and maintenance overhead. By following GitHub's path resolution behavior, documentation remains accessible whether viewed on GitHub.com, in local editors, or through documentation tools like Claude Code.

## Example
Consider a file at `/docs/api/endpoints.md` with these links:
```markdown
<!-- Correct relative paths -->
[Authentication](../guides/auth.md)        # → /docs/guides/auth.md ✓  // EXAMPLE: Outside KB directory
[Rate Limits](./rate-limits.md)           # → /docs/api/rate-limits.md ✓  // EXAMPLE: Same directory
[Examples](examples/rest.md)              # → /docs/api/examples/rest.md ✓  // EXAMPLE: Subdirectory

<!-- Incorrect absolute path -->
[Overview](/docs/overview.md)             # → GitHub 404 error ✗  // EXAMPLE: Absolute path (invalid)
```

The absolute path fails because GitHub constructs URLs differently than local filesystem paths.

## Generalization

### Path Resolution Rules

**Relative Paths** - Resolved from the containing file's directory:
- Same directory: `file.md` or `./file.md`
- Parent directory: `../file.md` (can chain: `../../file.md`)
- Subdirectory: `subfolder/file.md`

**Absolute Paths** - Start with `/` and break on GitHub:
- GitHub interprets `/path` as `github.com/user/repo/path` (404 error)
- Local tools may interpret as filesystem root or repo root
- Should be avoided for cross-platform compatibility

### Validation Requirements

**File Existence Checks**:
- Target file must exist at the resolved path
- Path resolution must respect case sensitivity
- Warn if file exists with different casing

**Path Format Rules**:
- Reject backslashes `\` (use forward slashes)
- Handle URL-encoded characters (`%20` for spaces)
- Warn about spaces in filenames
- Prevent traversal outside repository root

### Implementation Pattern

```python
def resolve_markdown_link(markdown_file, link_path):
    """Resolve a link path as GitHub would"""
    if link_path.startswith('/'):
        # Absolute paths are problematic - report as error
        return None, "Absolute paths break on GitHub"
    
    # Relative path - resolve from markdown file's directory
    markdown_dir = Path(markdown_file).parent
    resolved = (markdown_dir / link_path).resolve()
    
    # Verify within repository bounds
    if not resolved.is_relative_to(repo_root):
        return None, "Path escapes repository"
    
    return resolved, None
```

### Best Practices

1. **Always use relative paths** for internal documentation links
2. **Test links on GitHub** to ensure they work in the web interface
3. **Validate during CI/CD** to catch broken links early
4. **Use consistent patterns** across the documentation:
   - From repo root: `docs/guide.md`, `kb/article.md`
   - From subdirectories: `../README.md`, `../../kb/article.md`
   - Within same directory: `related-doc.md`

## Related Patterns
- [Documentation Validation Enhancement Patterns](documentation-validation-enhancement-patterns.md)
- [Cross-referencing Knowledge Base](cross-referencing-knowledge-base.md)
- [Documentation Standards Integration](documentation-standards-integration.md)