# AGENTS.md

## Scope

These instructions apply to all files under `docs/`.

Follow the repository-root `AGENTS.md` by default, except for the docs-only verification override below.

## Docs-Only Verification Override

When **all modified tracked files are under `docs/`**, treat the change as a static documentation/site update.

In that case:

- Do **not** run `make clean build 2>&1`.
- Do **not** run `make check-logs`.
- If any changed file under `docs/` is a Markdown file, run `make check-docs 2>&1` before committing.
- For HTML/CSS/JS-only changes under `docs/`, verify the edited files directly and review the rendered result when practical.

If **any** modified tracked file is outside `docs/`, fall back to the full repository-root pre-commit checklist.

## Site Rules

- `docs/` is the GitHub Pages publishing root for the repository.
- Keep assets static and self-contained.
- Use relative paths between files inside `docs/`.
- Do not place generated build artifacts in `docs/`.

## Walkthrough Subtree

For files under `docs/walkthroughs/`, also follow `docs/walkthroughs/AGENTS.md`.
