# Pull Request Review Workflow

## Context

Automated reviewers such as Cursor Bugbot and Codex can add inline review comments, later replies,
and follow-up findings on already-resolved GitHub review threads. A PR can show zero unresolved
threads while still containing actionable follow-up comments that need documentation or code changes.

## Problem

Relying only on unresolved-thread counts misses important review activity:

- reviewers can add new findings inside an existing thread
- a thread can be resolved before the reviewer confirms the fix
- summary review comments can point to stale documentation even after code is fixed
- multiple findings can require separate commits to preserve review traceability

## Workflow

When asked to resolve PR review comments:

1. Fetch all review threads and comments, not just unresolved threads.
2. Inspect replies from reviewers after the last assistant/developer reply.
3. Treat actionable reviewer replies in resolved threads as pending work.
4. Apply exactly one logical fix per commit when the user requests that granularity.
5. Run the narrowest relevant validation first, then broader validation when appropriate.
6. Commit the fix before replying to the reviewer.
7. Reply directly to the original or follow-up review comment:
   - tag the reviewer (`@cursor`, `@chatgpt-codex-connector`, etc.)
   - cite the fix commit hash
   - explain what changed and why it addresses the comment
   - ask the reviewer to review the fix
8. Push the branch.
9. Run `git status` after pushing.
10. Re-check all review threads and comments for additional reviewer replies.
11. Do **not** mark review threads resolved yourself; leave resolution to the reviewer/bot.

## Useful Commands

Fetch inline review threads with all comments:

```bash
gh api graphql \
  -f query='query($owner:String!, $repo:String!, $number:Int!) { repository(owner:$owner, name:$repo) { pullRequest(number:$number) { reviewThreads(first:100) { nodes { id isResolved comments(first:50) { nodes { databaseId path body author { login } createdAt } } } } } } }' \
  -f owner=OWNER \
  -f repo=REPO \
  -F number=PR_NUMBER
```

Fetch one inline comment by ID:

```bash
gh api /repos/OWNER/REPO/pulls/comments/COMMENT_ID
```

Reply to an inline review comment after committing the fix:

```bash
gh api -X POST /repos/OWNER/REPO/pulls/PR_NUMBER/comments/COMMENT_ID/replies \
  -f body='@reviewer Fixed in `COMMIT`. Explanation of what changed. Please review the fix.'
```

Check unresolved-thread count as a final signal only, not as the sole source of truth:

```bash
gh api graphql \
  -f query='query($owner:String!, $repo:String!, $number:Int!) { repository(owner:$owner, name:$repo) { pullRequest(number:$number) { reviewThreads(first:100) { nodes { isResolved comments(first:50) { nodes { body author { login } createdAt } } } } } } }' \
  -f owner=OWNER \
  -f repo=REPO \
  -F number=PR_NUMBER
```

## Evidence to Report

When summarizing PR review work, include:

- commit hash for each fix
- validation commands and pass/fail result
- whether replies were posted to each review comment
- whether the branch was pushed and `git status` was clean
- count of unresolved threads and any remaining actionable replies

## Related Patterns

- [Git Push Verification](git-push-verification.md)
- [Commit Scope Verification](commit-scope-verification.md)
- [User Feedback as QA](user-feedback-as-qa.md)
- [Evidence-Based Debugging](evidence-based-debugging.md)
