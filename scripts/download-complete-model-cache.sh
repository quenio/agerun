#!/bin/bash
# Download the shared real complete() model artifacts.
# Usage: make download-complete-model
#
# Purpose:
# - Reuse one model directory across AgeRun worktrees.
# - Create a lock while large model artifacts are being downloaded.
# - Make concurrent instances wait until the lock clears, then re-check availability.

set -e
set -o pipefail

_home_directory() {
    local ref_home

    if [ -n "${HOME:-}" ]; then
        printf '%s\n' "$HOME"
        return
    fi

    if command -v python3 >/dev/null 2>&1; then
        ref_home=$(python3 -c 'import os, pwd; print(pwd.getpwuid(os.getuid()).pw_dir)' 2>/dev/null || true)
        if [ -n "$ref_home" ]; then
            printf '%s\n' "$ref_home"
            return
        fi
    fi

    echo "ERROR: HOME is not set and account home lookup failed"
    exit 1
}

COMPLETE_MODEL_HOME=${COMPLETE_MODEL_HOME:-$(_home_directory)}
COMPLETE_MODEL_DIR=${COMPLETE_MODEL_DIR:-"$COMPLETE_MODEL_HOME/.agerun/models"}
COMPLETE_MODEL_FILE=${COMPLETE_MODEL_FILE:-"$COMPLETE_MODEL_DIR/phi-3-mini-q4.gguf"}
COMPLETE_MODEL_LICENSE=${COMPLETE_MODEL_LICENSE:-"$COMPLETE_MODEL_DIR/phi-3-mini-q4.LICENSE"}
COMPLETE_MODEL_CARD=${COMPLETE_MODEL_CARD:-"$COMPLETE_MODEL_DIR/phi-3-mini-q4.README.md"}
DEFAULT_COMPLETE_MODEL_URL="https://huggingface.co/microsoft/Phi-3-mini-4k-instruct-gguf"
DEFAULT_COMPLETE_MODEL_LICENSE_URL="$DEFAULT_COMPLETE_MODEL_URL/raw/main/LICENSE"
DEFAULT_COMPLETE_MODEL_CARD_URL="$DEFAULT_COMPLETE_MODEL_URL/raw/main/README.md"
DEFAULT_COMPLETE_MODEL_URL="$DEFAULT_COMPLETE_MODEL_URL/resolve/main"
DEFAULT_COMPLETE_MODEL_URL="$DEFAULT_COMPLETE_MODEL_URL/Phi-3-mini-4k-instruct-q4.gguf?download=true"
COMPLETE_MODEL_URL=${COMPLETE_MODEL_URL:-"$DEFAULT_COMPLETE_MODEL_URL"}
COMPLETE_MODEL_LICENSE_URL=${COMPLETE_MODEL_LICENSE_URL:-"$DEFAULT_COMPLETE_MODEL_LICENSE_URL"}
COMPLETE_MODEL_CARD_URL=${COMPLETE_MODEL_CARD_URL:-"$DEFAULT_COMPLETE_MODEL_CARD_URL"}
COMPLETE_MODEL_LOCK=${COMPLETE_MODEL_LOCK:-"$COMPLETE_MODEL_DIR/download.lock"}
COMPLETE_MODEL_LOCK_POLL_SECONDS=${COMPLETE_MODEL_LOCK_POLL_SECONDS:-2}
COMPLETE_MODEL_LOCK_TIMEOUT_SECONDS=${COMPLETE_MODEL_LOCK_TIMEOUT_SECONDS:-3600}

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

    case "$path" in
        /.agerun|/.agerun/*)
            echo "ERROR: unsafe $name path targets the filesystem root: '$path'"
            exit 1
            ;;
    esac
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

MODEL_DIR=$(_abs_path "$COMPLETE_MODEL_DIR")
MODEL_FILE=$(_abs_path "$COMPLETE_MODEL_FILE")
MODEL_LICENSE=$(_abs_path "$COMPLETE_MODEL_LICENSE")
MODEL_CARD=$(_abs_path "$COMPLETE_MODEL_CARD")
MODEL_LOCK=$(_abs_path "$COMPLETE_MODEL_LOCK")
MODEL_LOCK_HOLDER="$MODEL_LOCK/holder"
MODEL_LOCK_RECOVERY="$MODEL_LOCK.recovery"
MODEL_LOCK_RECOVERY_HOLDER="$MODEL_LOCK_RECOVERY/holder"
CURRENT_HOST=$(hostname 2>/dev/null || echo unknown)
DOWNLOAD_LOCK_TOKEN=
DOWNLOAD_LOCK_HEARTBEAT_PID=
DOWNLOAD_LOCK_PID_STARTED_AT=

_require_safe_path "COMPLETE_MODEL_DIR" "$MODEL_DIR"
_require_safe_path "COMPLETE_MODEL_FILE" "$MODEL_FILE"
_require_safe_path "COMPLETE_MODEL_LICENSE" "$MODEL_LICENSE"
_require_safe_path "COMPLETE_MODEL_CARD" "$MODEL_CARD"
_require_safe_path "COMPLETE_MODEL_LOCK" "$MODEL_LOCK"
_require_non_negative_integer "COMPLETE_MODEL_LOCK_POLL_SECONDS" "$COMPLETE_MODEL_LOCK_POLL_SECONDS"
_require_non_negative_integer "COMPLETE_MODEL_LOCK_TIMEOUT_SECONDS" "$COMPLETE_MODEL_LOCK_TIMEOUT_SECONDS"

_path_mtime_epoch() {
    local path="$1"

    stat -c %Y "$path" 2>/dev/null ||
        stat -f %m "$path" 2>/dev/null ||
        echo 0
}

_file_value() {
    local file="$1"
    local key="$2"

    sed -n "s/^$key=//p" "$file" 2>/dev/null | head -n 1 || true
}

_lock_file_value() {
    local key="$1"

    _file_value "$MODEL_LOCK_HOLDER" "$key"
}

_recovery_lock_file_value() {
    local key="$1"

    _file_value "$MODEL_LOCK_RECOVERY_HOLDER" "$key"
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
    printf '%s\n' "$output" |
        awk 'NF { sub(/^[[:space:]]*/, ""); sub(/[[:space:]]*$/, ""); print; exit }'
}

_pid_matches_start_fingerprint() {
    local pid="$1"
    local expected_fingerprint="$2"
    local actual_fingerprint

    if [ -z "$expected_fingerprint" ]; then
        return 1
    fi

    actual_fingerprint=$(_pid_start_fingerprint "$pid") || return 1
    [ -n "$actual_fingerprint" ] && [ "$actual_fingerprint" = "$expected_fingerprint" ]
}

_lock_holder_is_dead_on_current_host() {
    local lock_pid
    local lock_host

    lock_pid=$(_lock_file_value "pid")
    lock_host=$(_lock_file_value "host")

    [ "$lock_host" = "$CURRENT_HOST" ] && ! _pid_exists_on_current_host "$lock_pid"
}

_lock_holder_is_foreign_host() {
    local lock_host

    lock_host=$(_lock_file_value "host")

    [ -n "$lock_host" ] && [ "$lock_host" != "$CURRENT_HOST" ]
}

_lock_holder_matches_current_process_identity() {
    local lock_pid
    local lock_host
    local lock_pid_started_at

    lock_pid=$(_lock_file_value "pid")
    lock_host=$(_lock_file_value "host")
    lock_pid_started_at=$(_lock_file_value "pid_started_at")

    [ "$lock_host" = "$CURRENT_HOST" ] &&
        _pid_exists_on_current_host "$lock_pid" &&
        _pid_matches_start_fingerprint "$lock_pid" "$lock_pid_started_at"
}

_missing_holder_grace_seconds() {
    printf '%s\n' "$COMPLETE_MODEL_LOCK_TIMEOUT_SECONDS"
}

_lock_is_stale() {
    local lock_mtime
    local now
    local lock_age
    local missing_holder_grace

    if [ ! -d "$MODEL_LOCK" ]; then
        return 1
    fi

    lock_mtime=$(_path_mtime_epoch "$MODEL_LOCK")
    now=$(date +%s)
    lock_age=$((now - lock_mtime))

    if [ ! -f "$MODEL_LOCK_HOLDER" ]; then
        missing_holder_grace=$(_missing_holder_grace_seconds)
        [ "$lock_age" -ge "$missing_holder_grace" ]
        return
    fi

    if _lock_holder_is_dead_on_current_host; then
        return 0
    fi

    if [ "$lock_age" -lt "$COMPLETE_MODEL_LOCK_TIMEOUT_SECONDS" ]; then
        return 1
    fi

    if _lock_holder_is_foreign_host; then
        return 0
    fi

    ! _lock_holder_matches_current_process_identity
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
        echo "model_lock=$MODEL_LOCK"
    } > "$MODEL_LOCK_RECOVERY_HOLDER"
}

_release_recovery_lock() {
    rm -f "$MODEL_LOCK_RECOVERY_HOLDER"
    rmdir "$MODEL_LOCK_RECOVERY" 2>/dev/null || true
}

_recovery_lock_holder_is_dead_on_current_host() {
    local lock_pid
    local lock_host

    lock_pid=$(_recovery_lock_file_value "pid")
    lock_host=$(_recovery_lock_file_value "host")

    [ "$lock_host" = "$CURRENT_HOST" ] && ! _pid_exists_on_current_host "$lock_pid"
}

_recovery_lock_holder_is_foreign_host() {
    local lock_host

    lock_host=$(_recovery_lock_file_value "host")

    [ -n "$lock_host" ] && [ "$lock_host" != "$CURRENT_HOST" ]
}

_recovery_lock_holder_matches_current_process_identity() {
    local lock_pid
    local lock_host
    local lock_pid_started_at

    lock_pid=$(_recovery_lock_file_value "pid")
    lock_host=$(_recovery_lock_file_value "host")
    lock_pid_started_at=$(_recovery_lock_file_value "pid_started_at")

    [ "$lock_host" = "$CURRENT_HOST" ] &&
        _pid_exists_on_current_host "$lock_pid" &&
        _pid_matches_start_fingerprint "$lock_pid" "$lock_pid_started_at"
}

_recovery_lock_is_stale() {
    local lock_mtime
    local now
    local lock_age

    if [ ! -d "$MODEL_LOCK_RECOVERY" ]; then
        return 1
    fi

    if _recovery_lock_holder_is_dead_on_current_host; then
        return 0
    fi

    lock_mtime=$(_path_mtime_epoch "$MODEL_LOCK_RECOVERY")
    now=$(date +%s)
    lock_age=$((now - lock_mtime))

    if [ "$lock_age" -lt "$COMPLETE_MODEL_LOCK_TIMEOUT_SECONDS" ]; then
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

    echo "Removing stale complete model download recovery lock at $MODEL_LOCK_RECOVERY"
    rm -f "$MODEL_LOCK_RECOVERY_HOLDER"
    rmdir "$MODEL_LOCK_RECOVERY" 2>/dev/null || true
}

_acquire_recovery_lock() {
    while true; do
        if mkdir "$MODEL_LOCK_RECOVERY" 2>/dev/null; then
            _write_recovery_lock_metadata
            return 0
        fi

        if _remove_stale_recovery_lock; then
            continue
        fi

        return 1
    done
}

_remove_stale_lock() {
    local remove_status

    if ! _acquire_recovery_lock; then
        return 1
    fi

    if ! _lock_is_stale; then
        _release_recovery_lock
        return 1
    fi

    echo "Removing stale complete model download lock at $MODEL_LOCK"
    rm -rf "$MODEL_LOCK"
    remove_status=$?
    _release_recovery_lock
    return "$remove_status"
}

_all_model_artifacts_available() {
    [ -s "$MODEL_FILE" ] && [ -s "$MODEL_LICENSE" ] && [ -s "$MODEL_CARD" ]
}

_new_download_lock_token() {
    local pid_started_at="$1"

    {
        printf 'host=%s\n' "$CURRENT_HOST"
        printf 'pid=%s\n' "$$"
        printf 'pid_started_at=%s\n' "$pid_started_at"
        printf 'nonce=%s-%s-%s\n' "$(date +%s)" "$RANDOM" "$RANDOM"
    } | (
        if command -v sha256sum >/dev/null 2>&1; then
            sha256sum | awk '{print $1}'
        else
            shasum -a 256 | awk '{print $1}'
        fi
    )
}

_write_lock_metadata() {
    {
        echo "token=$DOWNLOAD_LOCK_TOKEN"
        echo "pid=$$"
        echo "host=$CURRENT_HOST"
        if [ -n "$DOWNLOAD_LOCK_PID_STARTED_AT" ]; then
            echo "pid_started_at=$DOWNLOAD_LOCK_PID_STARTED_AT"
        fi
        echo "started_at=$(date -u '+%Y-%m-%dT%H:%M:%SZ')"
        echo "model_dir=$MODEL_DIR"
    } > "$MODEL_LOCK_HOLDER"
}

_heartbeat_download_lock() {
    while [ -n "$DOWNLOAD_LOCK_TOKEN" ] &&
        [ -d "$MODEL_LOCK" ] &&
        [ -f "$MODEL_LOCK_HOLDER" ] &&
        [ "$(_lock_file_value "token")" = "$DOWNLOAD_LOCK_TOKEN" ]; do
        touch "$MODEL_LOCK" "$MODEL_LOCK_HOLDER" 2>/dev/null || true
        sleep "$COMPLETE_MODEL_LOCK_POLL_SECONDS"
    done
}

_start_download_lock_heartbeat() {
    _heartbeat_download_lock &
    DOWNLOAD_LOCK_HEARTBEAT_PID=$!
}

_stop_download_lock_heartbeat() {
    if [ -n "$DOWNLOAD_LOCK_HEARTBEAT_PID" ]; then
        kill "$DOWNLOAD_LOCK_HEARTBEAT_PID" 2>/dev/null || true
        wait "$DOWNLOAD_LOCK_HEARTBEAT_PID" 2>/dev/null || true
        DOWNLOAD_LOCK_HEARTBEAT_PID=
    fi
}

_release_download_lock() {
    _stop_download_lock_heartbeat

    if [ -n "$DOWNLOAD_LOCK_TOKEN" ] &&
        [ -f "$MODEL_LOCK_HOLDER" ] &&
        [ "$(_lock_file_value "token")" = "$DOWNLOAD_LOCK_TOKEN" ]; then
        rm -f "$MODEL_LOCK_HOLDER"
        rmdir "$MODEL_LOCK" 2>/dev/null || true
    fi
}

_wait_for_lock() {
    while [ -d "$MODEL_LOCK" ]; do
        if _remove_stale_lock; then
            continue
        fi

        echo "Waiting for complete model download lock at $MODEL_LOCK"
        sleep "$COMPLETE_MODEL_LOCK_POLL_SECONDS"
    done
}

_acquire_lock() {
    mkdir -p "$MODEL_DIR"

    while true; do
        _wait_for_lock

        if _all_model_artifacts_available; then
            return 1
        fi

        if ! _acquire_recovery_lock; then
            echo "Waiting for complete model download recovery lock at $MODEL_LOCK_RECOVERY"
            sleep "$COMPLETE_MODEL_LOCK_POLL_SECONDS"
            continue
        fi

        if [ -d "$MODEL_LOCK" ]; then
            _release_recovery_lock
            continue
        fi

        DOWNLOAD_LOCK_PID_STARTED_AT=$(_pid_start_fingerprint "$$") || DOWNLOAD_LOCK_PID_STARTED_AT=
        DOWNLOAD_LOCK_TOKEN=$(_new_download_lock_token "$DOWNLOAD_LOCK_PID_STARTED_AT")
        if mkdir "$MODEL_LOCK" 2>/dev/null; then
            if ! _write_lock_metadata; then
                rm -f "$MODEL_LOCK_HOLDER"
                rmdir "$MODEL_LOCK" 2>/dev/null || true
                _release_recovery_lock
                echo "ERROR: unable to write complete model download lock metadata at $MODEL_LOCK_HOLDER"
                exit 1
            fi
            _start_download_lock_heartbeat
            _release_recovery_lock
            trap '_release_download_lock' EXIT INT TERM
            return 0
        fi

        _release_recovery_lock
    done
}

_download_asset() {
    local target="$1"
    local url="$2"
    local label="$3"
    local partial="$target.partial"

    if [ -s "$target" ]; then
        echo "Using existing $label at $target"
        return
    fi

    mkdir -p "$(dirname "$target")"
    echo "Downloading $label to $target"
    curl -L --fail --retry 3 --retry-delay 5 --continue-at - --output "$partial" "$url"
    mv "$partial" "$target"
    test -s "$target"
}

_download_missing_artifacts() {
    _download_asset "$MODEL_FILE" "$COMPLETE_MODEL_URL" "complete model"
    _download_asset "$MODEL_LICENSE" "$COMPLETE_MODEL_LICENSE_URL" "complete model license"
    _download_asset "$MODEL_CARD" "$COMPLETE_MODEL_CARD_URL" "complete model card"
}

_wait_for_lock

if _all_model_artifacts_available; then
    echo "Using shared complete model artifacts at $MODEL_DIR"
    exit 0
fi

if _acquire_lock; then
    if _all_model_artifacts_available; then
        echo "Using shared complete model artifacts at $MODEL_DIR"
    else
        _download_missing_artifacts
    fi
else
    echo "Using shared complete model artifacts at $MODEL_DIR"
fi

test -s "$MODEL_FILE"
test -s "$MODEL_LICENSE"
test -s "$MODEL_CARD"
