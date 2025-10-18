# Documentation Standards Integration

## Learning
Documentation standards must be integrated into core development guidelines and automated workflows to ensure consistent application across all project phases and contributors.

## Importance
Embedded standards become automatic habits, prevent documentation debt, maintain quality during rapid development, and ensure new team members follow established patterns from day one.

## Example
Integration across multiple touchpoints:

**Core Guidelines (AGENTS.md)**
```markdown
### 0. Documentation Protocol
**Always search AGENTS.md first** when asked about procedures.
**When reading TODO.md**: Check [ ] = incomplete, [x] = complete.

### Module Development quality checklist:
✓ Update modules/README.md for new modules
✓ All examples use real types and functions
✓ Documentation validated with make check-docs
```

**Slash Commands (.opencode/command/ar/new-learnings.md)**
```markdown
## Code Requirements
- Use REAL types and functions from the AgeRun codebase
- Tag hypothetical examples with // EXAMPLE: or // BAD:
- Validate with make check-docs before committing
```

**Build Integration (Makefile)**
```makefile
check-docs:
    @python3 scripts/check_docs.py
    
build: check-docs run-tests
    @echo "Build complete with documentation validation"
```

## Generalization
Apply standards integration to:
- Development workflows (pre-commit hooks)
- Code review checklists
- CI/CD pipelines
- Onboarding documentation
- Tool command help text
- Error messages and guidance

**Integration Touchpoints:**
1. **Core guidelines** - make standards discoverable and mandatory
2. **Automated tools** - enforce standards without manual effort
3. **Build processes** - fail fast when standards are violated
4. **Documentation generation** - apply standards to new content automatically
5. **Review processes** - include standards verification in checklists

## Implementation

**Multi-Layer Integration:**
```python
# Layer 1: Automated validation
def validate_all_docs():
    """Enforce standards automatically"""
    check_real_types_only()
    check_proper_tagging()
    
# Layer 2: Tool integration  
def create_new_kb_article():
    """Apply standards to new content"""
    use_real_types_by_default()
    validate_before_saving()
    
# Layer 3: Workflow integration
def pre_commit_hook():
    """Standards as gatekeeper"""
    run_documentation_validation()
    fail_if_violations_found()
```

**Standards Documentation Pattern:**
```markdown
# In AGENTS.md Section 0: Documentation Protocol
**Documentation Standards** (CRITICAL for all .md files):
- Use real types/functions from codebase - no hypotheticals without tags
- Tag intentional examples: // EXAMPLE:, // BAD:, // ERROR:
- Validate with `make check-docs` before committing
- Cross-reference related modules and principles
```

**Tool Enhancement Pattern:**
- Make validation tools give specific guidance on standards
- Include standards reminders in error messages
- Provide discovery tools to find compliant examples
- Auto-fix common violations where safe

## Related Patterns
- [Validated Documentation Examples](validated-documentation-examples.md)
- [Progressive Tool Enhancement](progressive-tool-enhancement.md)
- [Complete Documentation Principle](complete-documentation-principle.md)
- [Markdown Link Resolution Patterns](markdown-link-resolution-patterns.md)