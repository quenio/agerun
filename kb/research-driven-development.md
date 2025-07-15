# Research-Driven Development

## Learning
Technical guidelines and architectural decisions should be grounded in authoritative research sources, then adapted with domain-specific examples rather than invented from scratch.

## Importance
Prevents reinventing established practices, provides theoretical foundation for practical guidelines, ensures comprehensive coverage, and enables team members to understand the rationale behind specific practices.

## Example
Code smells implementation followed research-driven approach:
1. **Identified authoritative source**: Martin Fowler's "Refactoring" book catalog
2. **Mapped to domain**: Object-oriented smells → C module patterns  
3. **Ensured completeness**: Covered all major categories (Bloaters, Couplers, etc.)
4. **Adapted examples**: Each smell illustrated with AgeRun C code
5. **Validated against codebase**: Verified examples matched actual patterns

Similar pattern used for Parnas principles - referenced original research papers, then adapted to AgeRun's modular C architecture.

## Generalization
Apply research-driven pattern to any technical domain:
1. **Research phase**: Identify authoritative sources (academic papers, industry standards, established books)
2. **Analysis phase**: Extract core principles/concepts from sources
3. **Adaptation phase**: Map concepts to domain-specific context
4. **Validation phase**: Verify against actual codebase/practice
5. **Documentation phase**: Include source references for credibility
6. **Evolution phase**: Update when new research emerges

## Implementation
```bash
# Research identification
# 1. Search academic sources: Google Scholar, ACM Digital Library
# 2. Identify industry-standard references (Fowler, Gang of Four, etc.)
# 3. Check for updated editions/recent developments

# Concept extraction
# 4. Create concept map from source material
# 5. Identify relationships between concepts
# 6. Note adaptation requirements for domain

# Domain mapping
# 7. Create domain-specific examples for each concept
# 8. Identify detection/measurement approaches
# 9. Validate against existing codebase patterns

# Documentation pattern
## Source Attribution
Based on [Author Name]'s *Book Title* (Year), Chapter X.

## Domain Adaptation  
Original concept: [OO description]
AgeRun adaptation: [C-specific description]
```

## Related Patterns
- [Systematic Guideline Enhancement](systematic-guideline-enhancement.md)
- [OO to C Adaptation](oo-to-c-adaptation.md)