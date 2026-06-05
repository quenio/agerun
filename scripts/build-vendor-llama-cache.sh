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
LLAMA_CACHE_INSTALL_DIR=${LLAMA_CACHE_INSTALL_DIR:-"$LLAMA_CACHE_DIR/install"}
LLAMA_CACHE_LOCK=${LLAMA_CACHE_LOCK:-"$LLAMA_CACHE_DIR/build.lock"}
LLAMA_BUILD_DIR=${LLAMA_BUILD_DIR:-"$LLAMA_CACHE_DIR/build"}
LLAMA_INSTALL_DIR=${LLAMA_INSTALL_DIR:-.deps/llama.cpp-install}
LLAMA_SHARED_EXT=${LLAMA_SHARED_EXT:-so}
LLAMA_CPU_CMAKE_FLAGS=${LLAMA_CPU_CMAKE_FLAGS:-}
LLAMA_CACHE_LOCK_POLL_SECONDS=${LLAMA_CACHE_LOCK_POLL_SECONDS:-2}

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

SOURCE_DIR=$(_abs_path "$LLAMA_SOURCE_DIR")
CACHE_DIR=$(_abs_path "$LLAMA_CACHE_DIR")
CACHE_INSTALL_DIR=$(_abs_path "$LLAMA_CACHE_INSTALL_DIR")
CACHE_LOCK=$(_abs_path "$LLAMA_CACHE_LOCK")
BUILD_DIR=$(_abs_path "$LLAMA_BUILD_DIR")
LOCAL_INSTALL_DIR=$(_abs_path "$LLAMA_INSTALL_DIR")

_require_safe_path "LLAMA_CACHE_DIR" "$CACHE_DIR"
_require_safe_path "LLAMA_CACHE_INSTALL_DIR" "$CACHE_INSTALL_DIR"
_require_safe_path "LLAMA_CACHE_LOCK" "$CACHE_LOCK"
_require_safe_path "LLAMA_BUILD_DIR" "$BUILD_DIR"
_require_safe_path "LLAMA_INSTALL_DIR" "$LOCAL_INSTALL_DIR"

CACHE_HEADER="$CACHE_INSTALL_DIR/include/llama.h"
CACHE_LIB="$CACHE_INSTALL_DIR/lib/libllama.$LLAMA_SHARED_EXT"
LOCAL_HEADER="$LOCAL_INSTALL_DIR/include/llama.h"
LOCAL_LIB="$LOCAL_INSTALL_DIR/lib/libllama.$LLAMA_SHARED_EXT"

_cache_available() {
    [ -f "$CACHE_HEADER" ] && [ -f "$CACHE_LIB" ]
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

_build_cache() {
    local cmake_flags=()

    _validate_build_inputs
    mkdir -p "$CACHE_DIR"
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

    if ! _cache_available; then
        echo "ERROR: vendored llama.cpp cache build did not produce $CACHE_HEADER and $CACHE_LIB"
        exit 1
    fi
}

_wait_for_lock() {
    while [ -f "$CACHE_LOCK" ]; do
        echo "Waiting for vendored llama.cpp cache lock at $CACHE_LOCK"
        sleep "$LLAMA_CACHE_LOCK_POLL_SECONDS"
    done
}

_acquire_lock() {
    mkdir -p "$CACHE_DIR"

    while true; do
        _wait_for_lock

        if _cache_available; then
            return 1
        fi

        if (
            set -o noclobber
            {
                echo "pid=$$"
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
