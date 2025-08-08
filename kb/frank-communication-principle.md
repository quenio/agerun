# Frank Communication Principle

## Learning
Technical communication should prioritize accuracy and honest assessment over polite agreement. When uncertain about correctness, express doubts clearly rather than providing potentially incorrect confirmation.

## Importance
- Prevents cascading errors from incorrect assumptions
- Builds trust through honest technical assessment
- Saves time by avoiding polite but misleading responses
- Ensures code quality through direct feedback

## Example
```c
// User provides incorrect assumption
// BAD: Polite agreement without verification
"Yes, that looks correct to me."

// GOOD: Frank assessment with evidence
"Actually, that's incorrect. The function ar_data__destroy() already handles null:
if (own_data == null) {
    return;
}
So the null check is redundant."
```

## Generalization
Apply frank communication in:
- Code reviews: Point out actual issues, not hypothetical ones
- Technical discussions: Challenge incorrect assumptions with evidence
- Implementation decisions: Advocate for correct approach over convenient one
- Documentation: Be explicit about limitations and edge cases

## Implementation
When communicating technically:
1. Verify claims with actual code: `grep -r "function_name" modules/`
2. Provide evidence: Show the actual implementation
3. Be direct: "That's incorrect because..." not "Maybe we should consider..."
4. Focus on technical accuracy over social harmony

## Related Patterns
- [API Behavior Verification](api-behavior-verification.md)
- [User Feedback as QA](user-feedback-as-qa.md)
- [Task Authorization Pattern](task-authorization-pattern.md)