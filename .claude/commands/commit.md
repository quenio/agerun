Create a git commit following the exact workflow specified in CLAUDE.md.

## Pre-Commit Checklist (MANDATORY - ALWAYS CHECK THIS)

Before starting the commit process, ensure you have completed ALL of these steps:

1. **Run Tests**: Use `./full_build.sh` for comprehensive build verification (~20 lines output) to ensure all changes work correctly and no memory leaks. **If full build fails, STOP - do not proceed with commit until all issues are fixed.**
2. **Update Module Documentation**: If you changed a module's interface, update its .md file
3. **Update TODO.md**: Mark completed tasks and add any new tasks identified
4. **Update CHANGELOG.md**: Document completed milestones and achievements (NON-NEGOTIABLE)
5. **Review Changes**: Use `git diff` to verify all changes are intentional
6. **Check for temporary/backup files**: NEVER commit backup files (*.backup, *.bak, *.tmp, etc.)
7. **Then Commit**: Only after completing steps 1-6

**IMPORTANT**: 
- Clean build MUST pass before committing - no exceptions
- CHANGELOG update is MANDATORY for every commit that completes tasks
- Never skip the checklist

## Commit Workflow

After completing the checklist above, follow these steps precisely:

1. **First, run these commands to analyze the current state:**
   - `git status` - to see all untracked files
   - `git diff` - to see both staged and unstaged changes
   - `git log --oneline -10` - to see recent commit messages and follow the repository's commit style

2. **Analyze all changes and draft a commit message:**
   - Summarize the nature of the changes (e.g., new feature, enhancement, bug fix, refactoring, test, docs)
   - Use proper verbs: "add" for new features, "update" for enhancements, "fix" for bug fixes
   - Focus on the "why" rather than the "what" 
   - Keep it concise (1-2 sentences)
   - Check for any sensitive information that shouldn't be committed
   - Ensure the message accurately reflects ALL changes

3. **Execute these commands:**
   - Add all relevant files: `git add -A`
   - Create the commit using HEREDOC format:
   ```bash
   git commit -m "$(cat <<'EOF'
   [Your commit message here]

   🤖 Generated with [Claude Code](https://claude.ai/code)

   Co-Authored-By: Claude <noreply@anthropic.com>
   EOF
   )"
   ```
   - Run `git status` to verify the commit succeeded

4. **If the commit fails due to pre-commit hooks:**
   - Retry the commit ONCE to include any automated changes
   - If it fails again, report the error
   - If it succeeds but files were modified by hooks, amend the commit to include them

5. **Push and verify:**
   - Run `git push`
   - **MANDATORY**: Run `git status` after push to verify it completed successfully
   - Confirm the working tree is clean and branch is up to date

**Important:** Never skip the final `git status` verification - this is a critical step per CLAUDE.md.

## Custom Instructions

If available, the instructions below may override some of the instructions above.

$ARGUMENTS

