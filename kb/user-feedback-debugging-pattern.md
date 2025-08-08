# User Feedback Debugging Pattern

## Learning
User questions that challenge assumptions are invaluable debugging tools. Questions like "Why do you think X?" or "How did you verify Y?" force critical examination of reasoning and often reveal incorrect assumptions.

## Importance
Debugging complex issues often involves forming hypotheses that seem reasonable but are actually wrong. User feedback acts as quality assurance, preventing wasted time on incorrect paths and forcing evidence-based debugging.

## Example
In the ar_method_ast memory leak debugging:

**Initial assumption**: "This is a Zig-C interface compatibility issue"
**User question**: "Why do you think the issue is with interface compatibility?"
**Result**: Forced to reconsider and look at the specific implementation

**Later assumption**: "AR__HEAP__FREE is translating to free()"  
**User question**: "How did you check that?"
**Result**: Realized I hadn't actually verified this, leading to proper investigation

This pattern led to discovering the real issue: missing `-DDEBUG` flag in Zig compilation.

## Generalization
When debugging:
1. State assumptions explicitly
2. Welcome challenges to those assumptions
3. Verify claims with actual evidence, not reasoning
4. Use user questions as debugging checkpoints
5. Re-examine the problem when assumptions are questioned

Best practices:
- Run actual tests instead of theorizing
- Show concrete output to support claims
- Admit when something wasn't verified
- Thank users for catching incorrect reasoning

## Implementation
```bash
# Instead of assuming, verify:
echo "Checking how macro expands..."
zig translate-c -I modules modules/ar_heap.h | grep "AR__HEAP__FREE"

# Test with different flags:
zig translate-c -DDEBUG -I modules modules/ar_heap.h | grep "AR__HEAP__FREE"

# Show actual evidence:
grep -n "Actual memory leaks:" memory_report.log
```

## Real-Time Course Correction Pattern

Beyond challenging formed assumptions, user feedback can provide immediate course correction to prevent going down wrong paths entirely:

**Pattern**: User intervenes before wrong approach is fully pursued
- "You know we cannot use the default build dir because of the race condition"
- "It does not make sense to run the full build if you broke the tests"  
- "No, man! That change was good. You should not have reverted it"

**Benefits**:
- Prevents wasted time on incorrect approaches
- Provides immediate redirection when logic goes astray
- Acts as real-time quality assurance during problem-solving

**Implementation**: Welcome immediate corrections and treat them as valuable course corrections rather than interruptions. Each intervention reveals a gap in systematic thinking.

```bash
# When user provides course correction:
# 1. Stop current approach immediately
# 2. Understand the reasoning behind the correction
# 3. Adjust methodology based on the feedback
# 4. Resume with corrected approach
```

## Related Patterns