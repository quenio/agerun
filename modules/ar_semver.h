#ifndef AGERUN_SEMVER_H
#define AGERUN_SEMVER_H

#include <stdbool.h>

/**
 * Parse semantic version string into components
 * @param ref_version Version string in form "X.Y.Z" or partial form like "X" or "X.Y"
 * @param major Pointer to store major version
 * @param minor Pointer to store minor version (set to 0 if not specified)
 * @param patch Pointer to store patch version (set to 0 if not specified)
 * @return true if parsing successful, false otherwise
 * @note Ownership: Function does not take ownership of ref_version.
 *       Pass NULL for any component you don't need.
 */
bool ar_semver__parse(const char *ref_version, int *major, int *minor, int *patch);

/**
 * Compare two version strings according to semver rules
 * @param ref_v1 First version string
 * @param ref_v2 Second version string
 * @return <0 if v1 < v2, 0 if v1 == v2, >0 if v1 > v2
 * @note Ownership: Function does not take ownership of ref_v1 or ref_v2.
 */
int ar_semver__compare(const char *ref_v1, const char *ref_v2);

/**
 * Check if versions are compatible (same major version)
 * @param ref_v1 First version string
 * @param ref_v2 Second version string
 * @return true if compatible, false otherwise
 * @note Ownership: Function does not take ownership of ref_v1 or ref_v2.
 */
bool ar_semver__are_compatible(const char *ref_v1, const char *ref_v2);

/**
 * Check if a version string matches a partial version pattern
 * @param ref_version Full version string (e.g., "1.2.3")
 * @param ref_pattern Partial or full version pattern (e.g., "1" or "1.2")
 * @return true if version matches the pattern, false otherwise
 * @note Ownership: Function does not take ownership of ref_version or ref_pattern.
 */
bool ar_semver__matches_pattern(const char *ref_version, const char *ref_pattern);

/**
 * Find the latest version from an array of version strings that matches a pattern
 * @param ref_versions Array of version strings (borrowed references)
 * @param count Number of versions in the array
 * @param ref_pattern Pattern to match (e.g., "1" for all 1.x.x versions)
 * @return Index of the latest matching version, or -1 if none found
 * @note Ownership: Function does not take ownership of any parameters.
 */
int ar_semver__find_latest_matching(const char **ref_versions, int count, const char *ref_pattern);

#endif /* AGERUN_SEMVER_H */
