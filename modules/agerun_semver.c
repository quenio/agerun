#include "agerun_semver.h"
#include "agerun_debug.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

bool ar_semver_parse(const char *ref_version, int *major, int *minor, int *patch) {
    if (ref_version == NULL) {
        return false;
    }
    
    // Initialize output parameters if provided
    if (major) *major = 0;
    if (minor) *minor = 0;
    if (patch) *patch = 0;
    
    // Parse major version
    char *end;
    int maj = (int)strtol(ref_version, &end, 10);
    if (maj < 0 || end == ref_version) {
        return false; // Invalid major version
    }
    
    if (major) *major = maj;
    
    // If we have a partial version (just major), we're done
    if (*end == '\0') {
        return true;
    }
    
    // Expect a dot
    if (*end != '.') {
        return false;
    }
    
    // Parse minor version
    const char *minor_start = end + 1;
    int min = (int)strtol(minor_start, &end, 10);
    if (min < 0 || end == minor_start) {
        return false; // Invalid minor version
    }
    
    if (minor) *minor = min;
    
    // If we have a partial version (major.minor), we're done
    if (*end == '\0') {
        return true;
    }
    
    // Expect a dot
    if (*end != '.') {
        return false;
    }
    
    // Parse patch version
    const char *patch_start = end + 1;
    int pat = (int)strtol(patch_start, &end, 10);
    if (pat < 0 || end == patch_start) {
        return false; // Invalid patch version
    }
    
    if (patch) *patch = pat;
    
    // We should be at the end of the string now (ignore any pre-release or build metadata)
    // In a full semver implementation, we would handle pre-release and build metadata
    
    // Check if we've reached the end of the string
    if (*end != '\0' && *end != '-' && *end != '+') {
        // If there are more components (like in 1.2.3.4), consider it invalid
        return false;
    }
    
    return true;
}

int ar_semver_compare(const char *ref_v1, const char *ref_v2) {
    if (ref_v1 == NULL && ref_v2 == NULL) {
        return 0; // Both null, consider equal
    }
    if (ref_v1 == NULL) {
        return -1; // null < any version
    }
    if (ref_v2 == NULL) {
        return 1; // any version > null
    }
    
    // Parse both versions
    int maj1, min1, pat1;
    int maj2, min2, pat2;
    
    bool valid1 = ar_semver_parse(ref_v1, &maj1, &min1, &pat1);
    bool valid2 = ar_semver_parse(ref_v2, &maj2, &min2, &pat2);
    
    if (!valid1 && !valid2) {
        return strcmp(ref_v1, ref_v2); // Both invalid, fall back to string comparison
    }
    
    if (!valid1) {
        return -1; // invalid < valid
    }
    
    if (!valid2) {
        return 1; // valid > invalid
    }
    
    // Compare major versions
    if (maj1 != maj2) {
        return maj1 - maj2;
    }
    
    // Major versions are equal, compare minor versions
    if (min1 != min2) {
        return min1 - min2;
    }
    
    // Minor versions are equal, compare patch versions
    return pat1 - pat2;
}

bool ar_semver_are_compatible(const char *ref_v1, const char *ref_v2) {
    if (ref_v1 == NULL || ref_v2 == NULL) {
        return false;
    }
    
    int maj1, maj2;
    
    if (!ar_semver_parse(ref_v1, &maj1, NULL, NULL) ||
        !ar_semver_parse(ref_v2, &maj2, NULL, NULL)) {
        return false; // Invalid version strings
    }
    
    // Versions are compatible if they have the same major version
    return maj1 == maj2;
}

bool ar_semver_matches_pattern(const char *ref_version, const char *ref_pattern) {
    if (ref_version == NULL || ref_pattern == NULL) {
        return false;
    }
    
    int v_maj, v_min, v_pat;
    int p_maj, p_min, p_pat;
    
    // Parse version components
    if (!ar_semver_parse(ref_version, &v_maj, &v_min, &v_pat)) {
        return false; // Invalid version
    }
    
    // Parse pattern components
    if (!ar_semver_parse(ref_pattern, &p_maj, &p_min, &p_pat)) {
        return false; // Invalid pattern
    }
    
    // Count components in pattern
    int pattern_components = 1; // At least major
    const char *dot = strchr(ref_pattern, '.');
    if (dot) {
        pattern_components++;
        dot = strchr(dot + 1, '.');
        if (dot) {
            pattern_components++;
        }
    }
    
    // Check if pattern components match version
    if (pattern_components >= 1 && v_maj != p_maj) {
        return false;
    }
    
    if (pattern_components >= 2 && v_min != p_min) {
        return false;
    }
    
    if (pattern_components >= 3 && v_pat != p_pat) {
        return false;
    }
    
    return true;
}

int ar_semver_find_latest_matching(const char **ref_versions, int count, const char *ref_pattern) {
    if (ref_versions == NULL || count <= 0 || ref_pattern == NULL) {
        return -1;
    }
    
    int latest_idx = -1;
    
    for (int i = 0; i < count; i++) {
        if (ref_versions[i] == NULL) {
            continue;
        }
        
        // Check if this version matches the pattern
        if (ar_semver_matches_pattern(ref_versions[i], ref_pattern)) {
            // If this is the first match or it's newer than our current latest
            if (latest_idx == -1 || 
                ar_semver_compare(ref_versions[i], ref_versions[latest_idx]) > 0) {
                latest_idx = i;
            }
        }
    }
    
    return latest_idx;
}
