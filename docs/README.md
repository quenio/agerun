# AgeRun Docs Site

This directory is the GitHub Pages site root for repository-hosted static content.

## Purpose

Files under `docs/` are intended to be published as static site assets, not as generated build output.

Current primary use:

- host browser-rendered walkthrough slide decks from [`walkthroughs/`](walkthroughs/README.md)

## Structure

```text
docs/
├── README.md
├── index.html
├── site.css
├── .nojekyll
└── walkthroughs/
    ├── index.html
    ├── README.md
    └── <topic-name>/
```

## Publishing Model

This folder is designed to be used as the GitHub Pages publishing root for this repository using the `/docs` publishing option on the `main` branch.

For this repository, published URLs are expected to be rooted at:

```text
https://quenio.github.io/agerun/
```

That means, for example, the walkthrough index and the agent lifecycle walkthrough should be reachable at:

```text
https://quenio.github.io/agerun/walkthroughs/index.html
https://quenio.github.io/agerun/walkthroughs/agent-lifecycle/index.html
```

## Notes

- Keep assets self-contained and static.
- Use relative paths between HTML, JS, CSS, and local assets inside `docs/`.
- Do not place generated build artifacts here.
- `.nojekyll` is included so GitHub Pages serves the files directly without Jekyll processing.
