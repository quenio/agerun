#!/bin/bash
# Build or link the shared vendored llama.cpp CPU cache.
# Usage: make vendor-llama-cpu
#
# Purpose:
# - Reuse a shared cache across AgeRun worktrees.
# - Create a lock file while the shared cache is being built.
# - Make concurrent build instances wait until the lock clears, then re-check cache availability.

set -e
set -o pipefail

CMAKE=${CMAKE:-cmake}
UNAME_S=${UNAME_S:-$(uname -s)}
LLAMA_SOURCE_DIR=${LLAMA_SOURCE_DIR:-llama-cpp}
LLAMA_CACHE_DIR=${LLAMA_CACHE_DIR:-"$HOME/.agerun/build/cache/vendor-llama-cpu"}
LLAMA_CACHE_INSTALL_DIR=${LLAMA_CACHE_INSTALL_DIR:-}
LLAMA_CACHE_LOCK=${LLAMA_CACHE_LOCK:-"$LLAMA_CACHE_DIR/build.lock"}
LLAMA_CACHE_KEY=${LLAMA_CACHE_KEY:-}
LLAMA_BUILD_DIR=${LLAMA_BUILD_DIR:-"$LLAMA_CACHE_DIR/build"}
LLAMA_INSTALL_DIR=${LLAMA_INSTALL_DIR:-.deps/llama.cpp-install}
LLAMA_SHARED_EXT=${LLAMA_SHARED_EXT:-so}
LLAMA_CPU_CMAKE_FLAGS=${LLAMA_CPU_CMAKE_FLAGS:-}
LLAMA_CACHE_LOCK_POLL_SECONDS=${LLAMA_CACHE_LOCK_POLL_SECONDS:-2}
LLAMA_CACHE_LOCK_TIMEOUT_SECONDS=${LLAMA_CACHE_LOCK_TIMEOUT_SECONDS:-3600}

_abs_path() {
    case "$1" in
        /*)
            printf '%s\n' "$1"
            ;;
        *)
            printf '%s/%s\n' "$PWD" "$1"
            ;;
    esac
}

_require_safe_path() {
    local name="$1"
    local path="$2"

    if [ -z "$path" ] || [ "$path" = "/" ]; then
        echo "ERROR: unsafe $name path: '$path'"
        exit 1
    fi
}

_require_non_negative_integer() {
    local name="$1"
    local value="$2"

    case "$value" in
        ""|*[!0-9]*)
            echo "ERROR: $name must be a non-negative integer, got '$value'"
            exit 1
            ;;
    esac
}

_hash_stdin() {
    if command -v sha256sum >/dev/null 2>&1; then
        sha256sum | awk '{print $1}'
        return
    fi

    if command -v shasum >/dev/null 2>&1; then
        shasum -a 256 | awk '{print $1}'
        return
    fi

    echo "ERROR: sha256sum or shasum is required to derive the vendored llama.cpp cache key"
    exit 1
}

_hash_file() {
    local file="$1"

    if command -v sha256sum >/dev/null 2>&1; then
        sha256sum "$file" | awk '{print $1}'
        return
    fi

    shasum -a 256 "$file" | awk '{print $1}'
}

_relative_path() {
    local path="$1"

    case "$path" in
        "$PWD"/*)
            printf '%s\n' "${path#"$PWD"/}"
            ;;
        *)
            printf '%s\n' "$path"
            ;;
    esac
}

_emit_file_cache_key_input() {
    local file="$1"

    if [ -f "$file" ]; then
        printf 'file=%s\n' "$(_relative_path "$file")"
        printf 'sha256=%s\n' "$(_hash_file "$file")"
    fi
}

_source_is_repo_relative() {
    case "$LLAMA_SOURCE_DIR" in
        /*)
            return 1
            ;;
        *)
            return 0
            ;;
    esac
}

_git_cache_inputs_are_clean() {
    local status

    status=$(git -C "$PWD" status --porcelain --untracked-files=all -- \
        "$LLAMA_SOURCE_DIR" \
        Makefile \
        scripts/build-vendor-llama-cache.sh 2>/dev/null) ||
        return 1

    [ -z "$status" ]
}

_derive_git_cache_key() {
    local source_oid
    local makefile_oid
    local script_oid

    if ! _source_is_repo_relative; then
        return 1
    fi

    if ! command -v git >/dev/null 2>&1; then
        return 1
    fi

    if ! git -C "$PWD" rev-parse --is-inside-work-tree >/dev/null 2>&1; then
        return 1
    fi

    if ! _git_cache_inputs_are_clean; then
        return 1
    fi

    source_oid=$(git -C "$PWD" rev-parse --verify "HEAD:$LLAMA_SOURCE_DIR" 2>/dev/null) ||
        return 1
    makefile_oid=$(git -C "$PWD" rev-parse --verify "HEAD:Makefile" 2>/dev/null) ||
        return 1
    script_oid=$(git -C "$PWD" rev-parse --verify "HEAD:scripts/build-vendor-llama-cache.sh" 2>/dev/null) ||
        return 1

    {
        printf 'source_oid=%s\n' "$source_oid"
        printf 'makefile_oid=%s\n' "$makefile_oid"
        printf 'script_oid=%s\n' "$script_oid"
    } | _hash_stdin
}

_derive_cache_key() {
    local arch
    local git_cache_key

    if [ ! -d "$SOURCE_DIR" ]; then
        echo "ERROR: vendored llama.cpp source not found at $SOURCE_DIR"
        echo "Cannot derive a cache key for the shared vendored llama.cpp cache"
        exit 1
    fi

    arch=$(uname -m)
    git_cache_key=$(_derive_git_cache_key) || git_cache_key=

    {
        printf 'system=%s\n' "$UNAME_S"
        printf 'arch=%s\n' "$arch"
        printf 'shared_ext=%s\n' "$LLAMA_SHARED_EXT"
        printf 'cmake_flags=%s\n' "$LLAMA_CPU_CMAKE_FLAGS"
        if [ -n "$git_cache_key" ]; then
            printf 'git_key=%s\n' "$git_cache_key"
        else
            find "$SOURCE_DIR" -type f ! -path '*/.git/*' -print | LC_ALL=C sort |
                while IFS= read -r file; do
                    _emit_file_cache_key_input "$file"
                done
            _emit_file_cache_key_input "$PWD/Makefile"
            _emit_file_cache_key_input "$PWD/scripts/build-vendor-llama-cache.sh"
        fi
    } | _hash_stdin
}

_cache_key_directory_name() {
    printf '%s\n' "$LLAMA_CACHE_KEY" | _hash_stdin
}

_resolve_cache_install_dir() {
    local requested_dir="$LLAMA_CACHE_INSTALL_DIR"
    local default_dir

    default_dir=$(_abs_path "$LLAMA_CACHE_DIR/install")

    if [ -z "$requested_dir" ] || [ "$(_abs_path "$requested_dir")" = "$default_dir" ]; then
        printf '%s/installs/%s\n' "$CACHE_DIR" "$(_cache_key_directory_name)"
        return
    fi

    _abs_path "$requested_dir"
}

SOURCE_DIR=$(_abs_path "$LLAMA_SOURCE_DIR")
CACHE_DIR=$(_abs_path "$LLAMA_CACHE_DIR")
CACHE_LOCK=$(_abs_path "$LLAMA_CACHE_LOCK")
STALE_LOCK_RECOVERY_DIR="$CACHE_LOCK.recovery"
STALE_LOCK_RECOVERY_FILE="$STALE_LOCK_RECOVERY_DIR/holder"
BUILD_DIR=$(_abs_path "$LLAMA_BUILD_DIR")
LOCAL_INSTALL_DIR=$(_abs_path "$LLAMA_INSTALL_DIR")

_require_safe_path "LLAMA_CACHE_DIR" "$CACHE_DIR"
_require_safe_path "LLAMA_CACHE_LOCK" "$CACHE_LOCK"
_require_safe_path "LLAMA_BUILD_DIR" "$BUILD_DIR"
_require_safe_path "LLAMA_INSTALL_DIR" "$LOCAL_INSTALL_DIR"
_require_non_negative_integer "LLAMA_CACHE_LOCK_POLL_SECONDS" "$LLAMA_CACHE_LOCK_POLL_SECONDS"
_require_non_negative_integer "LLAMA_CACHE_LOCK_TIMEOUT_SECONDS" "$LLAMA_CACHE_LOCK_TIMEOUT_SECONDS"
LOCAL_HEADER="$LOCAL_INSTALL_DIR/include/llama.h"
LOCAL_LIB="$LOCAL_INSTALL_DIR/lib/libllama.$LLAMA_SHARED_EXT"
CURRENT_HOST=$(hostname 2>/dev/null || echo unknown)

if [ -z "$LLAMA_CACHE_KEY" ]; then
    LLAMA_CACHE_KEY=$(_derive_cache_key)
fi

CACHE_INSTALL_DIR=$(_resolve_cache_install_dir)
_require_safe_path "LLAMA_CACHE_INSTALL_DIR" "$CACHE_INSTALL_DIR"

CACHE_HEADER="$CACHE_INSTALL_DIR/include/llama.h"
CACHE_LIB="$CACHE_INSTALL_DIR/lib/libllama.$LLAMA_SHARED_EXT"
CONTENT_KEY_FILE="$CACHE_INSTALL_DIR/.agerun-cache-key"

_cache_key_matches() {
    [ -f "$CONTENT_KEY_FILE" ] &&
        [ "$(cat "$CONTENT_KEY_FILE")" = "$LLAMA_CACHE_KEY" ]
}

_cache_available() {
    [ -f "$CACHE_HEADER" ] && [ -f "$CACHE_LIB" ] && _cache_key_matches
}

_local_link_current() {
    [ -L "$LOCAL_INSTALL_DIR" ] &&
        [ "$(readlink "$LOCAL_INSTALL_DIR")" = "$CACHE_INSTALL_DIR" ] &&
        [ -f "$LOCAL_HEADER" ] &&
        [ -f "$LOCAL_LIB" ]
}

_link_local_install() {
    if [ "$LOCAL_INSTALL_DIR" = "$CACHE_INSTALL_DIR" ]; then
        return
    fi

    if _local_link_current; then
        return
    fi

    mkdir -p "$(dirname "$LOCAL_INSTALL_DIR")"
    rm -rf "$LOCAL_INSTALL_DIR"
    ln -s "$CACHE_INSTALL_DIR" "$LOCAL_INSTALL_DIR"
}

_validate_build_inputs() {
    if [ ! -f "$SOURCE_DIR/CMakeLists.txt" ]; then
        echo "ERROR: vendored llama.cpp source not found at $SOURCE_DIR"
        echo "Add the pinned upstream source tree under $LLAMA_SOURCE_DIR before running this target"
        exit 1
    fi

    if [ ! -f "$SOURCE_DIR/LICENSE" ]; then
        echo "ERROR: upstream llama.cpp license file missing at $SOURCE_DIR/LICENSE"
        echo "Keep the upstream MIT license text in the vendored source tree"
        exit 1
    fi

    if ! command -v "$CMAKE" >/dev/null 2>&1; then
        echo "ERROR: cmake is required for make vendor-llama-cpu"
        echo "Set CMAKE=/absolute/path/to/cmake or install cmake before building the vendored dependency"
        exit 1
    fi
}

_patch_darwin_rpaths() {
    local dylib
    local exe

    if [ "$UNAME_S" != "Darwin" ]; then
        return
    fi

    for dylib in "$CACHE_INSTALL_DIR"/lib/*.dylib; do
        if [ -f "$dylib" ]; then
            install_name_tool -add_rpath @loader_path "$dylib" 2>/dev/null || true
            install_name_tool -add_rpath @loader_path/../lib "$dylib" 2>/dev/null || true
        fi
    done

    for exe in "$CACHE_INSTALL_DIR"/bin/*; do
        if [ -f "$exe" ]; then
            install_name_tool -add_rpath @loader_path/../lib "$exe" 2>/dev/null || true
        fi
    done
}

_write_cache_key() {
    if [ -n "$LLAMA_CACHE_KEY" ]; then
        printf '%s\n' "$LLAMA_CACHE_KEY" > "$CONTENT_KEY_FILE"
    fi
}

_build_cache() {
    local cmake_flags=()

    _validate_build_inputs
    mkdir -p "$CACHE_DIR"
    rm -rf "$BUILD_DIR"
    rm -rf "$CACHE_INSTALL_DIR"

    if [ -n "$LLAMA_CPU_CMAKE_FLAGS" ]; then
        # Intentionally split the Makefile-provided CMake flags.
        cmake_flags=($LLAMA_CPU_CMAKE_FLAGS)
    fi

    echo "Building shared vendored llama.cpp cache at $CACHE_DIR"
    "$CMAKE" -S "$SOURCE_DIR" -B "$BUILD_DIR" \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX="$CACHE_INSTALL_DIR" \
        -DBUILD_SHARED_LIBS=ON \
        "${cmake_flags[@]}"
    "$CMAKE" --build "$BUILD_DIR" --config Release
    "$CMAKE" --install "$BUILD_DIR" --config Release
    _patch_darwin_rpaths
    _write_cache_key

    if ! _cache_available; then
        echo "ERROR: vendored llama.cpp cache build did not produce $CACHE_HEADER and $CACHE_LIB"
        exit 1
    fi
}

_path_mtime_epoch() {
    local path="$1"

    stat -c %Y "$path" 2>/dev/null ||
        stat -f %m "$path" 2>/dev/null ||
        echo 0
}

_lock_mtime_epoch() {
    _path_mtime_epoch "$CACHE_LOCK"
}

_lock_file_value() {
    local file="$1"
    local key="$2"

    sed -n "s/^$key=//p" "$file" 2>/dev/null | head -n 1 || true
}

_lock_value() {
    _lock_file_value "$CACHE_LOCK" "$1"
}

_recovery_lock_value() {
    _lock_file_value "$STALE_LOCK_RECOVERY_FILE" "$1"
}

_pid_exists_on_current_host() {
    local pid="$1"

    case "$pid" in
        ""|*[!0-9]*)
            return 1
            ;;
    esac

    if kill -0 "$pid" 2>/dev/null; then
        return 0
    fi

    # A process owned by another user can reject kill -0 with EPERM while still
    # being alive. ps keeps that lock holder from being mistaken for a dead PID.
    ps -p "$pid" >/dev/null 2>&1
}

_pid_start_fingerprint() {
    local pid="$1"
    local output

    case "$pid" in
        ""|*[!0-9]*)
            return 1
            ;;
    esac

    output=$(ps -p "$pid" -o lstart= 2>/dev/null) || return 1
    printf '%s\n' "$output" | awk 'NF { sub(/^[[:space:]]*/, ""); sub(/[[:space:]]*$/, ""); print; exit }'
}

_pid_matches_start_fingerprint() {
    local pid="$1"
    local expected_fingerprint="$2"
    local actual_fingerprint

    if [ -z "$expected_fingerprint" ]; then
        # If ps cannot provide a start-time fingerprint, preserve a live same-host
        # lock instead of risking an overlapping cache build.
        _pid_exists_on_current_host "$pid"
        return
    fi

    actual_fingerprint=$(_pid_start_fingerprint "$pid") || return 1
    [ -n "$actual_fingerprint" ] && [ "$actual_fingerprint" = "$expected_fingerprint" ]
}

_lock_holder_matches_current_process_identity() {
    local lock_pid
    local lock_host
    local lock_pid_started_at

    lock_pid=$(_lock_value "pid")
    lock_host=$(_lock_value "host")
    lock_pid_started_at=$(_lock_value "pid_started_at")

    [ "$lock_host" = "$CURRENT_HOST" ] &&
        _pid_exists_on_current_host "$lock_pid" &&
        _pid_matches_start_fingerprint "$lock_pid" "$lock_pid_started_at"
}

_lock_holder_is_dead_on_current_host() {
    local lock_pid
    local lock_host

    lock_pid=$(_lock_value "pid")
    lock_host=$(_lock_value "host")

    [ "$lock_host" = "$CURRENT_HOST" ] && ! _pid_exists_on_current_host "$lock_pid"
}

_lock_holder_is_foreign_host() {
    local lock_host

    lock_host=$(_lock_value "host")

    [ -n "$lock_host" ] && [ "$lock_host" != "$CURRENT_HOST" ]
}

_write_recovery_lock_metadata() {
    local pid_started_at

    pid_started_at=$(_pid_start_fingerprint "$$") || pid_started_at=

    {
        echo "pid=$$"
        echo "host=$CURRENT_HOST"
        if [ -n "$pid_started_at" ]; then
            echo "pid_started_at=$pid_started_at"
        fi
        echo "started_at=$(date -u '+%Y-%m-%dT%H:%M:%SZ')"
        echo "cache_lock=$CACHE_LOCK"
    } > "$STALE_LOCK_RECOVERY_FILE"
}

_release_recovery_lock() {
    rm -f "$STALE_LOCK_RECOVERY_FILE"
    rmdir "$STALE_LOCK_RECOVERY_DIR" 2>/dev/null || true
}

_recovery_lock_holder_matches_current_process_identity() {
    local lock_pid
    local lock_host
    local lock_pid_started_at

    lock_pid=$(_recovery_lock_value "pid")
    lock_host=$(_recovery_lock_value "host")
    lock_pid_started_at=$(_recovery_lock_value "pid_started_at")

    [ "$lock_host" = "$CURRENT_HOST" ] &&
        _pid_exists_on_current_host "$lock_pid" &&
        _pid_matches_start_fingerprint "$lock_pid" "$lock_pid_started_at"
}

_recovery_lock_holder_is_dead_on_current_host() {
    local lock_pid
    local lock_host

    lock_pid=$(_recovery_lock_value "pid")
    lock_host=$(_recovery_lock_value "host")

    [ "$lock_host" = "$CURRENT_HOST" ] && ! _pid_exists_on_current_host "$lock_pid"
}

_recovery_lock_holder_is_foreign_host() {
    local lock_host

    lock_host=$(_recovery_lock_value "host")

    [ -n "$lock_host" ] && [ "$lock_host" != "$CURRENT_HOST" ]
}

_recovery_lock_is_stale() {
    local lock_mtime
    local now
    local lock_age

    if [ ! -d "$STALE_LOCK_RECOVERY_DIR" ]; then
        return 1
    fi

    if _recovery_lock_holder_is_dead_on_current_host; then
        return 0
    fi

    lock_mtime=$(_path_mtime_epoch "$STALE_LOCK_RECOVERY_DIR")
    now=$(date +%s)
    lock_age=$((now - lock_mtime))

    if [ "$lock_age" -lt "$LLAMA_CACHE_LOCK_TIMEOUT_SECONDS" ]; then
        return 1
    fi

    if _recovery_lock_holder_is_foreign_host; then
        return 0
    fi

    ! _recovery_lock_holder_matches_current_process_identity
}

_remove_stale_recovery_lock() {
    if ! _recovery_lock_is_stale; then
        return 1
    fi

    echo "Removing stale vendored llama.cpp cache recovery lock at $STALE_LOCK_RECOVERY_DIR"
    rm -f "$STALE_LOCK_RECOVERY_FILE"
    rmdir "$STALE_LOCK_RECOVERY_DIR" 2>/dev/null
}

_acquire_recovery_lock() {
    while true; do
        if mkdir "$STALE_LOCK_RECOVERY_DIR" 2>/dev/null; then
            _write_recovery_lock_metadata
            return 0
        fi

        if _remove_stale_recovery_lock; then
            continue
        fi

        return 1
    done
}

_lock_is_stale() {
    local lock_mtime
    local now
    local lock_age

    if _lock_holder_is_dead_on_current_host; then
        return 0
    fi

    lock_mtime=$(_lock_mtime_epoch)
    now=$(date +%s)
    lock_age=$((now - lock_mtime))

    if [ "$lock_age" -lt "$LLAMA_CACHE_LOCK_TIMEOUT_SECONDS" ]; then
        return 1
    fi

    if _lock_holder_is_foreign_host; then
        return 0
    fi

    ! _lock_holder_matches_current_process_identity
}

_remove_stale_lock() {
    if ! _acquire_recovery_lock; then
        return 1
    fi

    (
        trap '_release_recovery_lock' EXIT INT TERM

        if [ -f "$CACHE_LOCK" ] && _lock_is_stale; then
            echo "Removing stale vendored llama.cpp cache lock at $CACHE_LOCK"
            rm -f "$CACHE_LOCK"
            exit 0
        fi

        exit 1
    )
}

_wait_for_lock() {
    while [ -f "$CACHE_LOCK" ]; do
        if _lock_is_stale && _remove_stale_lock; then
            continue
        fi

        echo "Waiting for vendored llama.cpp cache lock at $CACHE_LOCK"
        sleep "$LLAMA_CACHE_LOCK_POLL_SECONDS"
    done
}

_acquire_lock() {
    local pid_started_at

    mkdir -p "$CACHE_DIR"

    while true; do
        _wait_for_lock

        if _cache_available; then
            return 1
        fi

        pid_started_at=$(_pid_start_fingerprint "$$") || pid_started_at=

        if (
            set -o noclobber
            {
                echo "pid=$$"
                echo "host=$CURRENT_HOST"
                if [ -n "$pid_started_at" ]; then
                    echo "pid_started_at=$pid_started_at"
                fi
                echo "started_at=$(date -u '+%Y-%m-%dT%H:%M:%SZ')"
                echo "cache_dir=$CACHE_DIR"
            } > "$CACHE_LOCK"
        ) 2>/dev/null; then
            trap 'rm -f "$CACHE_LOCK"' EXIT INT TERM
            return 0
        fi
    done
}

_wait_for_lock

if _cache_available; then
    echo "Using shared vendored llama.cpp cache at $CACHE_DIR"
    _link_local_install
    exit 0
fi

if _acquire_lock; then
    if _cache_available; then
        echo "Using shared vendored llama.cpp cache at $CACHE_DIR"
    else
        _build_cache
    fi
else
    echo "Using shared vendored llama.cpp cache at $CACHE_DIR"
fi

_link_local_install
test -f "$LOCAL_HEADER"
test -f "$LOCAL_LIB"
