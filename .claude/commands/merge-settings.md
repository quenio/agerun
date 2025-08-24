Merge ./.claude/settings.local.json into ./.claude/settings.json and remove the local file.
Follow these steps:
1. Check if ./.claude/settings.local.json exists
2. If it exists, read both settings files
3. Merge the permissions from local into main settings
4. Write the merged settings back to ./.claude/settings.json
5. Remove the local settings file
6. Commit and push the updated settings.json:
   - `git add .claude/settings.json`
   - `git commit -m "chore: merge local settings"`
   - `git push`
7. If local file doesn't exist, inform that no merge is needed

See [Settings File Merging Pattern](../../kb/settings-file-merging-pattern.md) for details.