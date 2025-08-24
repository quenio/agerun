# Role Clarification Pattern

## Learning
Slash commands and similar task-oriented documents benefit from explicitly stating the role being played at the very beginning. This immediately sets context and expectations for the task execution.

## Importance
Clear role definition:
- Sets appropriate mindset for task execution
- Clarifies the perspective from which decisions will be made
- Helps maintain consistency in approach and tone
- Reduces ambiguity about the task's purpose and scope

## Example
```markdown
**Role: DevOps Engineer**

Execute build and test operations with focus on CI/CD and system reliability.

**Role: Documentation Specialist**  

Review and validate documentation with focus on accuracy and completeness.

**Role: Software Architect**

Analyze system design with focus on patterns, principles, and maintainability.
```

## Generalization
Apply role clarification to:
- Command definitions that execute specific tasks
- Documentation that guides specific activities
- Scripts that perform specialized operations
- Any context where perspective matters for execution

Format: Always use `**Role: [Role Name]**` as the first line to ensure visibility.

## Implementation
```bash
# Add role to existing command file
echo "**Role: DevOps Engineer**" | cat - command.md > temp.md && mv temp.md command.md

# Verify role is present
head -1 command.md | grep -q "^\*\*Role:" || echo "Missing role statement"
```

## Related Patterns
- [Systematic File Modification Workflow](systematic-file-modification-workflow.md)
- [Batch Update Script Pattern](batch-update-script-pattern.md)