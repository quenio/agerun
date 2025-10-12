# Comprehensive Learning Extraction Pattern

## Learning
The process of extracting learnings from a session requires systematic review and user collaboration. Initial analysis often misses subtle patterns that only become apparent when the user prompts for deeper reflection. The most valuable learnings often emerge from what seems routine or obvious at first glance.

## Importance
Without systematic extraction, valuable patterns remain undocumented and future sessions repeat the same discoveries. User prompting acts as a quality gate, ensuring comprehensive documentation. This meta-pattern improves the knowledge base growth rate and reduces technical debt from undocumented learnings.

## Example
```c
// Session progression showing learning extraction depth:

// INITIAL ANALYSIS (3 patterns identified):
// 1. Error logging pattern
// 2. Module consistency  
// 3. Stateless verification

// USER PROMPT: "You had identified other learnings I didn't see you write about"

// DEEPER ANALYSIS (3 more patterns discovered):
// 4. API suffix cleanup after migration
// 5. NULL instance logging limitation  
// 6. Test-driven documentation validation

// USER PROMPT: "Any learnings on how to find and document new learnings?"

// META-ANALYSIS (pattern about patterns):
// 7. Comprehensive learning extraction pattern itself

// This demonstrates the layered nature of learning extraction
```

## Generalization
**Multi-pass extraction strategy**:

1. **First pass - Obvious patterns**: 
   - Direct technical implementations
   - Explicit bug fixes
   - Clear architectural changes

2. **Second pass - Subtle patterns** (after user prompting):
   - Process improvements discovered during work
   - Limitations accepted as design choices
   - Cleanup patterns after main work

3. **Third pass - Meta patterns** (reflection on reflection):
   - How learnings were discovered
   - Why initial analysis was incomplete
   - Process improvements for future extraction

**Key insights**:
- User prompting reveals blind spots in self-analysis
- Initial extraction typically captures only 50% of learnings
- Meta-patterns about learning extraction are themselves learnings
- Routine work often contains non-obvious patterns

## Implementation
```bash
# Systematic learning extraction workflow:

# 1. Initial extraction - what seems important
grep -r "pattern\|learning\|insight" session_notes.md

# 2. Review user corrections - often reveal missed patterns
grep -r "actually\|should\|why\|but" session_transcript.md

# 3. Analyze routine work - hidden patterns
# Look for:
# - Repeated sequences of commands
# - Similar fixes across multiple files
# - Accepted limitations or trade-offs
# - Process discoveries during debugging

# 4. Meta-analysis - patterns about patterns
# Ask:
# - Why did I miss certain learnings initially?
# - What prompted the user to ask for more?
# - How can extraction be more thorough?

# 5. Document extraction process itself as a learning
echo "Document the meta-pattern of learning extraction"
```

## Extraction Triggers
Questions that reveal hidden learnings:
- "What else did you discover that seems obvious now?"
- "What trade-offs did you accept without documenting?"
- "What would you do differently next time?"
- "What surprised you during implementation?"
- "What patterns emerged across multiple files?"
- "What did the user's feedback reveal?"

## Anti-patterns in Learning Extraction
- **Surface-level documentation**: Only capturing the final fix, not the discovery process
- **Obvious-only bias**: Documenting only "big" discoveries, missing subtle patterns
- **Single-pass extraction**: Stopping after first analysis without deeper reflection
- **Ignoring meta-patterns**: Not documenting patterns about documentation itself

## Quality Indicators
Signs of comprehensive extraction:
- Multiple passes reveal new patterns each time
- User satisfaction with completeness
- Meta-patterns about process are included
- Both successes and accepted limitations documented
- Patterns range from specific to general to meta

## Related Patterns
- [Context Preservation Across Sessions](context-preservation-across-sessions.md) - Preserving learnings across context boundaries
- [Post-Session Task Extraction Pattern](post-session-task-extraction-pattern.md)
- [New Learnings Complete Integration Pattern](new-learnings-complete-integration-pattern.md)
- [User Feedback as Architecture Gate](user-feedback-as-architecture-gate.md)
- [User Feedback as QA](user-feedback-as-qa.md)
- [Documentation Compacting Pattern](documentation-compacting-pattern.md)
- [Self-Documenting Modifications Pattern](self-documenting-modifications-pattern.md)
- [Command Thoroughness Requirements Pattern](command-thoroughness-requirements-pattern.md)
- [Systematic Consistency Verification](systematic-consistency-verification.md)
- [Unmissable Documentation Pattern](unmissable-documentation-pattern.md)
- [Search Result Completeness Verification](search-result-completeness-verification.md) - Complete searches prevent missing patterns