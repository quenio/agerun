# AgeRun Semantic Versioning Module

## Overview

The `ar_semver` module provides functions for parsing, comparing, and managing semantic version strings according to the Semantic Versioning specification. This module is crucial for the AgeRun runtime system's method versioning and agent version management.

## Key Features

- Semantic version string parsing into major, minor, and patch components
- Version comparison according to semver rules
- Version compatibility checking (same major version)
- Pattern matching for partial version specifications
- Finding the latest version that matches a pattern

## API Reference

### Parsing

```c
bool ar__semver__parse(const char *ref_version, int *major, int *minor, int *patch);
```

Parses a semantic version string into its numeric components. The version string can be in the full form "X.Y.Z" or partial forms like "X" or "X.Y". Missing components default to 0.

- **Parameters:**
  - `ref_version`: Version string to parse (e.g., "1.2.3", "1.2", or "1")
  - `major`: Pointer to store the major version number
  - `minor`: Pointer to store the minor version number (set to 0 if not in string)
  - `patch`: Pointer to store the patch version number (set to 0 if not in string)
- **Returns:** `true` if parsing was successful, `false` otherwise
- **Note:** You can pass NULL for any component you don't need to capture

### Comparison

```c
int ar__semver__compare(const char *ref_v1, const char *ref_v2);
```

Compares two version strings according to semantic versioning rules.

- **Parameters:**
  - `ref_v1`: First version string
  - `ref_v2`: Second version string
- **Returns:**
  - Less than zero if v1 < v2
  - Zero if v1 = v2
  - Greater than zero if v1 > v2
- **Note:** NULL is considered less than any valid version; Invalid versions are compared lexicographically

### Compatibility Check

```c
bool ar__semver__are_compatible(const char *ref_v1, const char *ref_v2);
```

Checks if two versions are compatible (have the same major version).

- **Parameters:**
  - `ref_v1`: First version string
  - `ref_v2`: Second version string
- **Returns:** `true` if versions are compatible, `false` otherwise
- **Note:** Compatible versions can be automatically swapped without breaking changes

### Pattern Matching

```c
bool ar__semver__matches_pattern(const char *ref_version, const char *ref_pattern);
```

Checks if a version string matches a version pattern. A pattern can be a partial version like "1" (matches all 1.x.x versions) or "1.2" (matches all 1.2.x versions).

- **Parameters:**
  - `ref_version`: Full version string (e.g., "1.2.3")
  - `ref_pattern`: Pattern to match (e.g., "1" or "1.2")
- **Returns:** `true` if the version matches the pattern, `false` otherwise

### Finding the Latest Matching Version

```c
int ar__semver__find_latest_matching(const char **ref_versions, int count, const char *ref_pattern);
```

Finds the latest version in an array that matches a specified pattern.

- **Parameters:**
  - `ref_versions`: Array of version strings
  - `count`: Number of versions in the array
  - `ref_pattern`: Pattern to match (e.g., "1" for all 1.x.x versions)
- **Returns:** Index of the latest matching version, or -1 if none found
- **Note:** Version comparison follows semantic versioning rules

## Version Transition Rules

When a new method version is registered, AgeRun automatically transitions running agents to the new version if:

1. The new version is **compatible** with the agent's current version (same major version)
2. The new version is **newer** than the agent's current version

The transition process follows these steps:

1. The agent finishes processing its current message
2. The system sends a `__sleep__` message to the agent
3. The agent's method reference is updated to the newer version
4. The system sends a `__wake__` message to the agent
5. The agent resumes operation with the new version

This process ensures a clean transition between compatible versions while preserving the agent's state.

## Usage Examples

### Basic Version Parsing

```c
int major, minor, patch;
if (ar__semver__parse("1.2.3", &major, &minor, &patch)) {
    printf("Version: %d.%d.%d\n", major, minor, patch);
}
```

### Version Comparison

```c
// Check if v2 is newer than v1
if (ar__semver__compare("1.2.3", "1.3.0") < 0) {
    printf("Version 1.3.0 is newer than 1.2.3\n");
}
```

### Finding Compatible Versions

```c
const char *versions[] = {"1.0.0", "1.1.0", "1.2.0", "2.0.0"};
int count = 4;
int latest_idx = ar__semver__find_latest_matching(versions, count, "1");
if (latest_idx >= 0) {
    printf("Latest 1.x.x version: %s\n", versions[latest_idx]); // 1.2.0
}
```

## Implementation Notes

- The module ignores pre-release identifiers and build metadata (e.g., "1.0.0-alpha+001")
- Version components must be non-negative integers
- The module handles NULL and invalid version strings gracefully