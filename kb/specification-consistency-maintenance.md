# Specification Consistency Maintenance

## Learning
Formal specifications (SPEC.md) can become outdated during rapid development and must be systematically updated after language changes to maintain consistency with implementation.

## Importance
The specification defines the language contract and is used by developers to understand AgeRun's capabilities. Outdated specifications can mislead users and create confusion between documented behavior and actual implementation.

## Example
```bnf
<!-- Before: Outdated BNF grammar -->
<function-call> ::= <send-function>
                 | <parse-function>
                 | <build-function>
                 | <method-function>
                 | <agent-function>        <!-- Outdated -->
                 | <destroy-function>      <!-- Outdated -->
                 | <if-function>

<agent-function> ::= 'agent' '(' <expression> ',' <expression> [',' <expression>] ')'
<destroy-function> ::= 'destroy' '(' <expression> ')'

<!-- After: Updated BNF grammar -->
<function-call> ::= <send-function>
                 | <parse-function>
                 | <build-function>
                 | <method-function>
                 | <spawn-function>        <!-- Updated -->
                 | <exit-function>         <!-- Updated -->
                 | <deprecate-function>    <!-- Added missing -->
                 | <if-function>

<spawn-function> ::= 'spawn' '(' <expression> ',' <expression> [',' <expression>] ')'
<exit-function> ::= 'exit' '(' <expression> ')'
<deprecate-function> ::= 'deprecate' '(' <expression> ',' <expression> ')'
```

## Generalization
Check specifications BEFORE implementation to avoid unnecessary features, and after major changes to maintain consistency:

### Before Implementation
1. **Feature Verification** - Check if feature is actually specified
2. **Syntax Confirmation** - Verify exact syntax requirements  
3. **Behavior Clarification** - Understand specified behavior vs assumptions

Example: Version resolution - SPEC.md specifies partial versions ("1", "1.2") but NOT wildcards ("*", "1.*")

### After Language Changes

1. **BNF Grammar Updates** - Function names, production rules, terminals
2. **Function Descriptions** - Parameter lists, return values, behavior
3. **Example Code** - Sample instructions using new terminology
4. **Cross-References** - Any mentions of old instruction names
5. **Missing Elements** - New instructions not yet documented

Check these sections specifically:
- Instruction Syntax (BNF grammar)
- Function call descriptions
- System functions list
- Agent Management section
- Example usage throughout

## Implementation
```bash
# 1. Search for outdated references
grep -n "agent\|create\|destroy" SPEC.md

# 2. Update systematically
# - BNF grammar productions
# - Function definitions
# - Descriptions and examples
# - Cross-references

# 3. Validate changes don't break documentation system
make check-docs

# 4. Commit specification updates separately
git add SPEC.md
git commit -m "docs: update SPEC.md to reflect terminology changes"
```

## Related Patterns
- [Language Instruction Renaming Pattern](language-instruction-renaming-pattern.md)
- [Documentation Standards Integration](documentation-standards-integration.md)
- [Validated Documentation Examples](validated-documentation-examples.md)
- [User Feedback as QA](user-feedback-as-qa.md)