#include "ar_local_completion.h"

#include <spawn.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <dlfcn.h>
#include <llama.h>

#include "ar_data.h"
#include "ar_heap.h"
#include "ar_log.h"

extern char **environ;

typedef struct ar_local_completion_buffer_s {
    char *own_text;
    size_t length;
    size_t capacity;
} ar_local_completion_buffer_t;

typedef struct ar_local_completion_placeholder_items_s {
    size_t count;
    void **own_items;
} ar_local_completion_placeholder_items_t;

typedef struct ar_local_completion_llama_api_s {
    void (*ref_llama_backend_init)(void);
    void (*ref_llama_backend_free)(void);
    struct llama_model_params (*ref_llama_model_default_params)(void);
    struct llama_model *(*ref_llama_model_load_from_file)(const char *ref_path, struct llama_model_params params);
    void (*ref_llama_model_free)(struct llama_model *own_model);
    const struct llama_vocab *(*ref_llama_model_get_vocab)(const struct llama_model *ref_model);
    int32_t (*ref_llama_tokenize)(const struct llama_vocab *ref_vocab,
                                  const char *ref_text,
                                  int32_t text_len,
                                  llama_token *mut_tokens,
                                  int32_t n_tokens_max,
                                  bool add_special,
                                  bool parse_special);
    struct llama_context_params (*ref_llama_context_default_params)(void);
    struct llama_context *(*ref_llama_init_from_model)(struct llama_model *mut_model,
                                                       struct llama_context_params params);
    void (*ref_llama_free)(struct llama_context *own_context);
    struct llama_sampler_chain_params (*ref_llama_sampler_chain_default_params)(void);
    struct llama_sampler *(*ref_llama_sampler_chain_init)(struct llama_sampler_chain_params params);
    void (*ref_llama_sampler_chain_add)(struct llama_sampler *mut_chain, struct llama_sampler *mut_sampler);
    struct llama_sampler *(*ref_llama_sampler_init_greedy)(void);
    struct llama_sampler *(*ref_llama_sampler_init_grammar)(const struct llama_vocab *ref_vocab,
                                                            const char *ref_grammar_str,
                                                            const char *ref_grammar_root);
    void (*ref_llama_sampler_free)(struct llama_sampler *own_sampler);
    struct llama_batch (*ref_llama_batch_get_one)(llama_token *mut_tokens, int32_t n_tokens);
    bool (*ref_llama_model_has_encoder)(const struct llama_model *ref_model);
    int32_t (*ref_llama_encode)(struct llama_context *mut_context, struct llama_batch batch);
    llama_token (*ref_llama_model_decoder_start_token)(const struct llama_model *ref_model);
    llama_token (*ref_llama_vocab_bos)(const struct llama_vocab *ref_vocab);
    int64_t (*ref_llama_time_us)(void);
    int32_t (*ref_llama_decode)(struct llama_context *mut_context, struct llama_batch batch);
    llama_token (*ref_llama_sampler_sample)(struct llama_sampler *mut_sampler,
                                            struct llama_context *mut_context,
                                            int32_t idx);
    bool (*ref_llama_vocab_is_eog)(const struct llama_vocab *ref_vocab, llama_token token);
    int32_t (*ref_llama_token_to_piece)(const struct llama_vocab *ref_vocab,
                                        llama_token token,
                                        char *mut_buffer,
                                        int32_t buffer_length,
                                        int32_t lstrip,
                                        bool special);
    void (*ref_llama_sampler_accept)(struct llama_sampler *mut_sampler, llama_token token);
} ar_local_completion_llama_api_t;

struct ar_local_completion_s {
    ar_log_t *ref_log;
    char *own_model_path;
    char *own_runner_path;
    bool own_backend_initialized;
    void *own_llama_library;
    struct llama_model *own_llama_model;
    ar_local_completion_llama_api_t llama_api;
};

static bool _build_direct_response_grammar(const ar_list_t *ref_placeholders,
                                           ar_local_completion_buffer_t *mut_grammar);
static bool _buffer_append(ar_local_completion_buffer_t *mut_buffer, const char *ref_text);
static void _buffer_destroy(ar_local_completion_buffer_t *mut_buffer);

static void _log_error(const ar_local_completion_t *ref_runtime, const char *ref_message) {
    if (ref_runtime != NULL && ref_runtime->ref_log != NULL && ref_message != NULL) {
        ar_log__error(ref_runtime->ref_log, ref_message);
    }
}

static void _log_failure(const ar_local_completion_t *ref_runtime,
                         const char *ref_base_message,
                         const char *ref_failure_category,
                         const char *ref_cause,
                         const char *ref_recovery_hint) {
    ar_local_completion_buffer_t own_message = {0};
    bool built_message = ref_base_message != NULL &&
                         ref_failure_category != NULL &&
                         ref_cause != NULL &&
                         ref_recovery_hint != NULL &&
                         _buffer_append(&own_message, ref_base_message) &&
                         _buffer_append(&own_message, "; failure_category=") &&
                         _buffer_append(&own_message, ref_failure_category) &&
                         _buffer_append(&own_message, "; cause=") &&
                         _buffer_append(&own_message, ref_cause) &&
                         _buffer_append(&own_message, "; recovery_hint=") &&
                         _buffer_append(&own_message, ref_recovery_hint);
    if (built_message) {
        _log_error(ref_runtime, own_message.own_text);
    } else {
        _log_error(ref_runtime, ref_base_message);
    }
    _buffer_destroy(&own_message);
}

static bool _buffer_reserve(ar_local_completion_buffer_t *mut_buffer, size_t minimum_capacity) {
    if (mut_buffer == NULL) {
        return false;
    }
    if (minimum_capacity <= mut_buffer->capacity) {
        return true;
    }

    size_t own_new_capacity = mut_buffer->capacity == 0U ? 256U : mut_buffer->capacity;
    while (own_new_capacity < minimum_capacity) {
        own_new_capacity *= 2U;
    }

    char *own_new_text = AR__HEAP__REALLOC(mut_buffer->own_text,
                                           own_new_capacity,
                                           "local_completion_buffer_text");
    if (own_new_text == NULL) {
        return false;
    }

    mut_buffer->own_text = own_new_text;
    mut_buffer->capacity = own_new_capacity;
    return true;
}

static bool _buffer_append_n(ar_local_completion_buffer_t *mut_buffer,
                             const char *ref_text,
                             size_t text_length) {
    if (mut_buffer == NULL || ref_text == NULL) {
        return false;
    }
    if (!_buffer_reserve(mut_buffer, mut_buffer->length + text_length + 1U)) {
        return false;
    }

    memcpy(mut_buffer->own_text + mut_buffer->length, ref_text, text_length);
    mut_buffer->length += text_length;
    mut_buffer->own_text[mut_buffer->length] = '\0';
    return true;
}

static bool _buffer_append(ar_local_completion_buffer_t *mut_buffer, const char *ref_text) {
    if (ref_text == NULL) {
        return false;
    }
    return _buffer_append_n(mut_buffer, ref_text, strlen(ref_text));
}

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wformat-nonliteral"
#endif
static bool _buffer_append_format(ar_local_completion_buffer_t *mut_buffer,
                                  const char *ref_format,
                                  ...) {
    if (mut_buffer == NULL || ref_format == NULL) {
        return false;
    }

    va_list own_args;
    va_start(own_args, ref_format);
    va_list own_args_copy;
    va_copy(own_args_copy, own_args);
    int required_length = vsnprintf(NULL, 0, ref_format, own_args_copy);
    va_end(own_args_copy);
    if (required_length < 0) {
        va_end(own_args);
        return false;
    }

    size_t own_required_size = (size_t)required_length + 1U;
    if (!_buffer_reserve(mut_buffer, mut_buffer->length + own_required_size)) {
        va_end(own_args);
        return false;
    }

    int written_length = vsnprintf(mut_buffer->own_text + mut_buffer->length,
                                   own_required_size,
                                   ref_format,
                                   own_args);
    va_end(own_args);
    if (written_length < 0) {
        return false;
    }

    mut_buffer->length += (size_t)written_length;
    return true;
}
#if defined(__clang__)
#pragma clang diagnostic pop
#endif

static void _buffer_destroy(ar_local_completion_buffer_t *mut_buffer) {
    if (mut_buffer == NULL) {
        return;
    }
    if (mut_buffer->own_text != NULL) {
        AR__HEAP__FREE(mut_buffer->own_text);
        mut_buffer->own_text = NULL;
    }
    mut_buffer->length = 0U;
    mut_buffer->capacity = 0U;
}

static char *_dup_optional(const char *ref_text, const char *ref_desc) {
    if (ref_text == NULL || ref_text[0] == '\0') {
        return NULL;
    }
    return AR__HEAP__STRDUP(ref_text, ref_desc);
}

static char *_default_model_path(void) {
    return AR__HEAP__STRDUP("models/phi-3-mini-q4.gguf", "local_completion_default_model_path");
}

static char *_resolve_model_path(void) {
    const char *ref_override = getenv("AGERUN_COMPLETE_MODEL");
    char *own_path = _dup_optional(ref_override, "local_completion_model_path");
    if (own_path != NULL) {
        return own_path;
    }
    return _default_model_path();
}

static char *_resolve_runner_path(void) {
    return _dup_optional(getenv("AGERUN_COMPLETE_RUNNER"), "local_completion_runner_path");
}

static const char *_default_llama_library_path(void) {
#if defined(__APPLE__)
    return ".deps/llama.cpp-install/lib/libllama.dylib";
#else
    return ".deps/llama.cpp-install/lib/libllama.so";
#endif
}

static const char *_alternate_llama_library_path(void) {
#if defined(__APPLE__)
    return "../../.deps/llama.cpp-install/lib/libllama.dylib";
#else
    return "../../.deps/llama.cpp-install/lib/libllama.so";
#endif
}

static bool _file_exists(const char *ref_path) {
    return ref_path != NULL && access(ref_path, F_OK) == 0;
}

static bool _placeholder_items_init(const ar_list_t *ref_placeholders,
                                    ar_local_completion_placeholder_items_t *mut_view) {
    if (mut_view == NULL) {
        return false;
    }

    mut_view->count = 0U;
    mut_view->own_items = NULL;
    if (ref_placeholders == NULL) {
        return false;
    }

    mut_view->count = ar_list__count(ref_placeholders);
    mut_view->own_items = ar_list__items(ref_placeholders);
    if (mut_view->count > 0U && mut_view->own_items == NULL) {
        return false;
    }

    return true;
}

static void _placeholder_items_destroy(ar_local_completion_placeholder_items_t *mut_view) {
    if (mut_view == NULL) {
        return;
    }
    if (mut_view->own_items != NULL) {
        AR__HEAP__FREE(mut_view->own_items);
        mut_view->own_items = NULL;
    }
    mut_view->count = 0U;
}

static bool _build_prompt(const char *ref_template,
                          const ar_list_t *ref_placeholders,
                          ar_local_completion_buffer_t *mut_prompt) {
    if (!_buffer_append(mut_prompt,
                        "<|user|>\n"
                        "You are filling an AgeRun complete() template.\n"
                        "Return exactly one line per requested placeholder in the form name=value.\n"
                        "Do not add commentary, numbering, JSON, or markdown.\n"
                        "Do not include braces in values.\n"
                        "Template: ")) {
        return false;
    }
    if (!_buffer_append(mut_prompt, ref_template)) {
        return false;
    }
    if (!_buffer_append(mut_prompt, "\nRequested placeholders:\n")) {
        return false;
    }

    ar_local_completion_placeholder_items_t own_placeholder_view = {0};
    if (!_placeholder_items_init(ref_placeholders, &own_placeholder_view)) {
        return false;
    }

    bool success = true;
    for (size_t index = 0U; index < own_placeholder_view.count; index++) {
        const char *ref_name = own_placeholder_view.own_items[index];
        if (ref_name == NULL ||
            !_buffer_append(mut_prompt, "- ") ||
            !_buffer_append(mut_prompt, ref_name) ||
            !_buffer_append(mut_prompt, "\n")) {
            success = false;
            break;
        }
    }

    if (success) {
        success = _buffer_append(mut_prompt,
                                 "Output only the requested placeholder assignments.\n"
                                 "<|end|>\n"
                                 "<|assistant|>\n");
    }

    _placeholder_items_destroy(&own_placeholder_view);
    return success;
}

static void _copy_symbol(void *mut_target, void *ref_symbol) {
    memcpy(mut_target, &ref_symbol, sizeof(ref_symbol));
}

#define LOAD_LLAMA_SYMBOL(mut_api, own_library, field_name, symbol_name)                              \
    do {                                                                                                 \
        void *own_symbol = dlsym((own_library), (symbol_name));                                          \
        if (own_symbol == NULL) {                                                                        \
            return false;                                                                                \
        }                                                                                                \
        _copy_symbol(&(mut_api)->field_name, own_symbol);                                                \
    } while (0)

static bool _load_llama_api(void *own_library, ar_local_completion_llama_api_t *mut_api) {
    if (own_library == NULL || mut_api == NULL) {
        return false;
    }

    memset(mut_api, 0, sizeof(*mut_api));
    LOAD_LLAMA_SYMBOL(mut_api, own_library, ref_llama_backend_init, "llama_backend_init");
    LOAD_LLAMA_SYMBOL(mut_api, own_library, ref_llama_backend_free, "llama_backend_free");
    LOAD_LLAMA_SYMBOL(mut_api, own_library, ref_llama_model_default_params, "llama_model_default_params");
    LOAD_LLAMA_SYMBOL(mut_api, own_library, ref_llama_model_load_from_file, "llama_model_load_from_file");
    LOAD_LLAMA_SYMBOL(mut_api, own_library, ref_llama_model_free, "llama_model_free");
    LOAD_LLAMA_SYMBOL(mut_api, own_library, ref_llama_model_get_vocab, "llama_model_get_vocab");
    LOAD_LLAMA_SYMBOL(mut_api, own_library, ref_llama_tokenize, "llama_tokenize");
    LOAD_LLAMA_SYMBOL(mut_api, own_library, ref_llama_context_default_params, "llama_context_default_params");
    LOAD_LLAMA_SYMBOL(mut_api, own_library, ref_llama_init_from_model, "llama_init_from_model");
    LOAD_LLAMA_SYMBOL(mut_api, own_library, ref_llama_free, "llama_free");
    LOAD_LLAMA_SYMBOL(mut_api, own_library, ref_llama_sampler_chain_default_params, "llama_sampler_chain_default_params");
    LOAD_LLAMA_SYMBOL(mut_api, own_library, ref_llama_sampler_chain_init, "llama_sampler_chain_init");
    LOAD_LLAMA_SYMBOL(mut_api, own_library, ref_llama_sampler_chain_add, "llama_sampler_chain_add");
    LOAD_LLAMA_SYMBOL(mut_api, own_library, ref_llama_sampler_init_greedy, "llama_sampler_init_greedy");
    LOAD_LLAMA_SYMBOL(mut_api, own_library, ref_llama_sampler_init_grammar, "llama_sampler_init_grammar");
    LOAD_LLAMA_SYMBOL(mut_api, own_library, ref_llama_sampler_free, "llama_sampler_free");
    LOAD_LLAMA_SYMBOL(mut_api, own_library, ref_llama_batch_get_one, "llama_batch_get_one");
    LOAD_LLAMA_SYMBOL(mut_api, own_library, ref_llama_model_has_encoder, "llama_model_has_encoder");
    LOAD_LLAMA_SYMBOL(mut_api, own_library, ref_llama_encode, "llama_encode");
    LOAD_LLAMA_SYMBOL(mut_api, own_library, ref_llama_model_decoder_start_token, "llama_model_decoder_start_token");
    LOAD_LLAMA_SYMBOL(mut_api, own_library, ref_llama_vocab_bos, "llama_vocab_bos");
    LOAD_LLAMA_SYMBOL(mut_api, own_library, ref_llama_time_us, "llama_time_us");
    LOAD_LLAMA_SYMBOL(mut_api, own_library, ref_llama_decode, "llama_decode");
    LOAD_LLAMA_SYMBOL(mut_api, own_library, ref_llama_sampler_sample, "llama_sampler_sample");
    LOAD_LLAMA_SYMBOL(mut_api, own_library, ref_llama_vocab_is_eog, "llama_vocab_is_eog");
    LOAD_LLAMA_SYMBOL(mut_api, own_library, ref_llama_token_to_piece, "llama_token_to_piece");
    LOAD_LLAMA_SYMBOL(mut_api, own_library, ref_llama_sampler_accept, "llama_sampler_accept");
    return true;
}

static bool _ensure_llama_backend_initialized(ar_local_completion_t *mut_runtime) {
    if (mut_runtime == NULL) {
        return false;
    }
    if (mut_runtime->own_model_path == NULL || mut_runtime->own_model_path[0] == '\0') {
        _log_failure(mut_runtime,
                     "complete() local model path is not configured; set AGERUN_COMPLETE_MODEL or provide models/phi-3-mini-q4.gguf",
                     "runtime_unavailable",
                     "local model path is not configured",
                     "set AGERUN_COMPLETE_MODEL or provide models/phi-3-mini-q4.gguf");
        return false;
    }
    if (!_file_exists(mut_runtime->own_model_path)) {
        _log_failure(mut_runtime,
                     "complete() local model file was not found; set AGERUN_COMPLETE_MODEL or provide models/phi-3-mini-q4.gguf",
                     "runtime_unavailable",
                     "local model file was not found",
                     "set AGERUN_COMPLETE_MODEL or provide models/phi-3-mini-q4.gguf");
        return false;
    }

    const char *ref_library_path = getenv("AGERUN_COMPLETE_LIBLLAMA");
    if (ref_library_path == NULL || ref_library_path[0] == '\0' || !_file_exists(ref_library_path)) {
        ref_library_path = _default_llama_library_path();
    }
    if (!_file_exists(ref_library_path)) {
        ref_library_path = _alternate_llama_library_path();
    }
    if (!_file_exists(ref_library_path)) {
        _log_failure(mut_runtime,
                     "complete() direct libllama runtime is unavailable; run make vendor-llama-cpu to build .deps/llama.cpp-install/lib/libllama",
                     "runtime_unavailable",
                     "vendored libllama runtime is unavailable",
                     "run make vendor-llama-cpu to build .deps/llama.cpp-install/lib/libllama");
        return false;
    }

    void *own_library = dlopen(ref_library_path, RTLD_NOW | RTLD_LOCAL);
    if (own_library == NULL) {
        _log_failure(mut_runtime,
                     "complete() could not load the vendored libllama runtime; run make vendor-llama-cpu and verify the installed library dependencies",
                     "runtime_unavailable",
                     "vendored libllama runtime could not be loaded",
                     "run make vendor-llama-cpu and verify the installed library dependencies");
        return false;
    }

    if (!_load_llama_api(own_library, &mut_runtime->llama_api)) {
        dlclose(own_library);
        _log_failure(mut_runtime,
                     "complete() vendored libllama is missing required symbols; rebuild the pinned vendored llama.cpp dependency",
                     "runtime_unavailable",
                     "vendored libllama is missing required symbols",
                     "rebuild the pinned vendored llama.cpp dependency");
        return false;
    }

    mut_runtime->llama_api.ref_llama_backend_init();

    struct llama_model_params own_model_params =
        mut_runtime->llama_api.ref_llama_model_default_params();
    own_model_params.n_gpu_layers = 0;

    struct llama_model *own_model =
        mut_runtime->llama_api.ref_llama_model_load_from_file(mut_runtime->own_model_path,
                                                              own_model_params);
    if (own_model == NULL) {
        mut_runtime->llama_api.ref_llama_backend_free();
        dlclose(own_library);
        _log_failure(mut_runtime,
                     "complete() could not load the configured GGUF model with direct libllama; verify the vendored CPU-only build and the configured model path",
                     "runtime_unavailable",
                     "configured GGUF model could not be loaded with direct libllama",
                     "verify the vendored CPU-only build and the configured model path");
        return false;
    }

    mut_runtime->own_llama_library = own_library;
    mut_runtime->own_llama_model = own_model;
    mut_runtime->own_backend_initialized = true;
    return true;
}

static bool _ensure_backend_initialized(ar_local_completion_t *mut_runtime) {
    if (mut_runtime == NULL) {
        return false;
    }
    if (mut_runtime->own_backend_initialized) {
        return true;
    }
    if (mut_runtime->own_runner_path != NULL) {
        mut_runtime->own_backend_initialized = true;
        return true;
    }
    return _ensure_llama_backend_initialized(mut_runtime);
}

static bool _read_pipe_to_buffer(int ref_fd, ar_local_completion_buffer_t *mut_buffer) {
    char own_chunk[1024];
    ssize_t read_count = 0;
    do {
        read_count = read(ref_fd, own_chunk, sizeof(own_chunk));
        if (read_count > 0) {
            if (!_buffer_append_n(mut_buffer, own_chunk, (size_t)read_count)) {
                return false;
            }
        }
    } while (read_count > 0);

    return read_count == 0;
}

static bool _run_runner_completion(const ar_local_completion_t *ref_runtime,
                                   const char *ref_prompt,
                                   const ar_list_t *ref_placeholders,
                                   ar_local_completion_buffer_t *mut_stdout) {
    if (ref_runtime == NULL || ref_runtime->own_runner_path == NULL || ref_prompt == NULL ||
        ref_placeholders == NULL || mut_stdout == NULL) {
        return false;
    }

    int own_pipe_fds[2] = {-1, -1};
    if (pipe(own_pipe_fds) != 0) {
        _log_error(ref_runtime,
                   "complete() local runner failed to start; recovery_hint=verify AGERUN_COMPLETE_RUNNER and local process execution support");
        return false;
    }

    posix_spawn_file_actions_t own_actions;
    if (posix_spawn_file_actions_init(&own_actions) != 0) {
        close(own_pipe_fds[0]);
        close(own_pipe_fds[1]);
        _log_error(ref_runtime,
                   "complete() local runner failed to start; recovery_hint=verify AGERUN_COMPLETE_RUNNER and local process execution support");
        return false;
    }

    posix_spawn_file_actions_adddup2(&own_actions, own_pipe_fds[1], STDOUT_FILENO);
    posix_spawn_file_actions_adddup2(&own_actions, own_pipe_fds[1], STDERR_FILENO);
    posix_spawn_file_actions_addclose(&own_actions, own_pipe_fds[0]);
    posix_spawn_file_actions_addclose(&own_actions, own_pipe_fds[1]);

    char *own_prompt_arg = AR__HEAP__STRDUP(ref_prompt, "local_completion_runner_prompt_arg");
    ar_local_completion_buffer_t own_grammar = {0};
    if (own_prompt_arg == NULL || !_build_direct_response_grammar(ref_placeholders, &own_grammar)) {
        if (own_prompt_arg != NULL) {
            AR__HEAP__FREE(own_prompt_arg);
        }
        _buffer_destroy(&own_grammar);
        close(own_pipe_fds[0]);
        own_pipe_fds[0] = -1;
        _log_error(ref_runtime,
                   "complete() local runner failed to start; recovery_hint=verify AGERUN_COMPLETE_RUNNER and local process execution support");
        return false;
    }

    char mut_arg_model[] = "-m";
    char mut_arg_prompt[] = "-p";
    char mut_arg_predict[] = "-n";
    char mut_arg_predict_value[] = "128";
    char mut_arg_temp[] = "--temp";
    char mut_arg_temp_value[] = "0";
    char mut_arg_ctx[] = "--ctx-size";
    char mut_arg_ctx_value[] = "512";
    char mut_arg_no_display[] = "--no-display-prompt";
    char mut_arg_grammar[] = "--grammar";
    char *own_argv[] = {
        ref_runtime->own_runner_path,
        mut_arg_model,
        ref_runtime->own_model_path,
        mut_arg_prompt,
        own_prompt_arg,
        mut_arg_predict,
        mut_arg_predict_value,
        mut_arg_temp,
        mut_arg_temp_value,
        mut_arg_ctx,
        mut_arg_ctx_value,
        mut_arg_no_display,
        mut_arg_grammar,
        own_grammar.own_text,
        NULL,
    };

    pid_t own_pid = 0;
    int spawn_result = posix_spawn(&own_pid,
                                   ref_runtime->own_runner_path,
                                   &own_actions,
                                   NULL,
                                   own_argv,
                                   environ);
    posix_spawn_file_actions_destroy(&own_actions);
    close(own_pipe_fds[1]);
    own_pipe_fds[1] = -1;

    if (spawn_result != 0) {
        close(own_pipe_fds[0]);
        own_pipe_fds[0] = -1;
        AR__HEAP__FREE(own_prompt_arg);
        _buffer_destroy(&own_grammar);
        _log_failure(ref_runtime,
                     "complete() local runner failed: cause=spawn; recovery_hint=install llama.cpp llama-cli or set AGERUN_COMPLETE_RUNNER",
                     "runtime_unavailable",
                     "local runner spawn failed",
                     "install llama.cpp llama-cli or set AGERUN_COMPLETE_RUNNER");
        return false;
    }

    bool read_ok = _read_pipe_to_buffer(own_pipe_fds[0], mut_stdout);
    close(own_pipe_fds[0]);
    own_pipe_fds[0] = -1;
    AR__HEAP__FREE(own_prompt_arg);
    _buffer_destroy(&own_grammar);

    int own_status = 0;
    if (waitpid(own_pid, &own_status, 0) < 0) {
        _log_failure(ref_runtime,
                     "complete() local runner did not exit normally; recovery_hint=verify the explicit AGERUN_COMPLETE_RUNNER override is stable on this platform",
                     "runtime_failure",
                     "local runner did not exit normally",
                     "verify the explicit AGERUN_COMPLETE_RUNNER override is stable on this platform");
        return false;
    }
    if (!read_ok) {
        _log_failure(ref_runtime,
                     "complete() local runner output could not be read; recovery_hint=verify the explicit AGERUN_COMPLETE_RUNNER override is stable on this platform",
                     "runtime_failure",
                     "local runner output could not be read",
                     "verify the explicit AGERUN_COMPLETE_RUNNER override is stable on this platform");
        return false;
    }
    if (!WIFEXITED(own_status) || WEXITSTATUS(own_status) != 0) {
        ar_local_completion_buffer_t own_message = {0};
        bool built_message =
            _buffer_append_format(&own_message,
                                  "complete() local runner exited with code %d: %s; recovery_hint=verify the explicit AGERUN_COMPLETE_RUNNER override, model path, and prompt format",
                                  WIFEXITED(own_status) ? WEXITSTATUS(own_status) : -1,
                                  mut_stdout->own_text != NULL ? mut_stdout->own_text : "");
        if (built_message) {
            _log_error(ref_runtime, own_message.own_text);
        } else {
            _log_failure(ref_runtime,
                         "complete() local runner exited with an error",
                         "runtime_failure",
                         "local runner exited with an error",
                         "verify the explicit AGERUN_COMPLETE_RUNNER override, model path, and prompt format");
        }
        _buffer_destroy(&own_message);
        return false;
    }

    return true;
}

static bool _build_direct_response_grammar(const ar_list_t *ref_placeholders,
                                           ar_local_completion_buffer_t *mut_grammar) {
    ar_local_completion_placeholder_items_t own_placeholder_view = {0};
    if (!_placeholder_items_init(ref_placeholders, &own_placeholder_view)) {
        return false;
    }

    bool success = _buffer_append(mut_grammar, "root ::= ");

    for (size_t index = 0U; success && index < own_placeholder_view.count; index++) {
        success = _buffer_append_format(mut_grammar, "line%zu", index) &&
                  (index + 1U >= own_placeholder_view.count || _buffer_append(mut_grammar, " "));
    }
    if (success) {
        success = _buffer_append(mut_grammar, "\n");
    }

    for (size_t index = 0U; success && index < own_placeholder_view.count; index++) {
        const char *ref_name = own_placeholder_view.own_items[index];
        success = ref_name != NULL &&
                  _buffer_append_format(mut_grammar,
                                        "line%zu ::= \"%s=\" value \"\\n\"\n",
                                        index,
                                        ref_name);
    }

    if (success) {
        success = _buffer_append(mut_grammar,
                                 "value ::= valuechar valuetail\n"
                                 "valuetail ::= valuechar valuetail | \"\"\n"
                                 "valuechar ::= [A-Za-z0-9_] | \" \" | \",\" | \".\" | \"-\" | \"'\" | \"/\" | \":\" | \"(\" | \")\" | \"&\"\n");
    }

    _placeholder_items_destroy(&own_placeholder_view);
    return success;
}

static bool _response_has_all_requested_placeholders(const ar_list_t *ref_placeholders,
                                                     const char *ref_output) {
    ar_local_completion_placeholder_items_t own_placeholder_view = {0};
    if (!_placeholder_items_init(ref_placeholders, &own_placeholder_view)) {
        return false;
    }

    bool found_all = true;
    for (size_t index = 0U; found_all && index < own_placeholder_view.count; index++) {
        const char *ref_name = own_placeholder_view.own_items[index];
        if (ref_name == NULL) {
            found_all = false;
            break;
        }

        ar_local_completion_buffer_t own_marker = {0};
        bool marker_ok = _buffer_append(&own_marker, ref_name) &&
                         _buffer_append(&own_marker, "=");
        found_all = marker_ok && strstr(ref_output, own_marker.own_text) != NULL;
        _buffer_destroy(&own_marker);
    }

    _placeholder_items_destroy(&own_placeholder_view);
    return found_all;
}

static bool _append_generated_piece(const ar_local_completion_t *ref_runtime,
                                    const struct llama_vocab *ref_vocab,
                                    const ar_local_completion_llama_api_t *ref_api,
                                    llama_token own_token,
                                    ar_local_completion_buffer_t *mut_output) {
    char own_stack_buffer[256];
    int32_t piece_length = ref_api->ref_llama_token_to_piece(ref_vocab,
                                                             own_token,
                                                             own_stack_buffer,
                                                             (int32_t)sizeof(own_stack_buffer),
                                                             0,
                                                             true);
    if (piece_length < 0) {
        size_t own_required_size = (size_t)(-piece_length);
        char *own_piece_buffer = AR__HEAP__MALLOC(own_required_size,
                                                  "local_completion_piece_buffer");
        if (own_piece_buffer == NULL) {
            _log_error(ref_runtime,
                       "complete() direct libllama backend could not allocate a token text buffer");
            return false;
        }

        piece_length = ref_api->ref_llama_token_to_piece(ref_vocab,
                                                         own_token,
                                                         own_piece_buffer,
                                                         (int32_t)own_required_size,
                                                         0,
                                                         true);
        if (piece_length < 0) {
            AR__HEAP__FREE(own_piece_buffer);
            _log_error(ref_runtime,
                       "complete() direct libllama backend could not decode a generated token into text");
            return false;
        }

        bool append_ok = _buffer_append_n(mut_output, own_piece_buffer, (size_t)piece_length);
        AR__HEAP__FREE(own_piece_buffer);
        return append_ok;
    }

    return _buffer_append_n(mut_output, own_stack_buffer, (size_t)piece_length);
}

static bool _run_direct_completion(const ar_local_completion_t *ref_runtime,
                                   const char *ref_prompt,
                                   const ar_list_t *ref_placeholders,
                                   int64_t timeout_ms,
                                   ar_local_completion_buffer_t *mut_output) {
    if (ref_runtime == NULL || ref_prompt == NULL || ref_placeholders == NULL || mut_output == NULL ||
        ref_runtime->own_llama_model == NULL) {
        return false;
    }

    const ar_local_completion_llama_api_t *ref_api = &ref_runtime->llama_api;
    const struct llama_vocab *ref_vocab = ref_api->ref_llama_model_get_vocab(ref_runtime->own_llama_model);
    if (ref_vocab == NULL) {
        _log_error(ref_runtime,
                   "complete() direct libllama backend could not access the loaded model vocabulary");
        return false;
    }

    int32_t prompt_token_count = -ref_api->ref_llama_tokenize(ref_vocab,
                                                              ref_prompt,
                                                              (int32_t)strlen(ref_prompt),
                                                              NULL,
                                                              0,
                                                              true,
                                                              true);
    if (prompt_token_count <= 0) {
        _log_error(ref_runtime,
                   "complete() direct libllama backend could not tokenize the completion prompt");
        return false;
    }

    llama_token *own_prompt_tokens =
        AR__HEAP__CALLOC((size_t)prompt_token_count, sizeof(llama_token), "local_completion_prompt_tokens");
    if (own_prompt_tokens == NULL) {
        _log_error(ref_runtime,
                   "complete() direct libllama backend could not allocate prompt tokens");
        return false;
    }

    int32_t actual_token_count = ref_api->ref_llama_tokenize(ref_vocab,
                                                             ref_prompt,
                                                             (int32_t)strlen(ref_prompt),
                                                             own_prompt_tokens,
                                                             prompt_token_count,
                                                             true,
                                                             true);
    if (actual_token_count < 0) {
        AR__HEAP__FREE(own_prompt_tokens);
        _log_error(ref_runtime,
                   "complete() direct libllama backend failed to tokenize the completion prompt");
        return false;
    }

    struct llama_context_params own_context_params = ref_api->ref_llama_context_default_params();
    own_context_params.n_ctx = (uint32_t)(actual_token_count + 256);
    own_context_params.n_batch = (uint32_t)actual_token_count;
    own_context_params.no_perf = true;

    struct llama_context *own_context =
        ref_api->ref_llama_init_from_model(ref_runtime->own_llama_model, own_context_params);
    if (own_context == NULL) {
        AR__HEAP__FREE(own_prompt_tokens);
        _log_error(ref_runtime,
                   "complete() direct libllama backend could not create a generation context");
        return false;
    }

    struct llama_sampler_chain_params own_sampler_params =
        ref_api->ref_llama_sampler_chain_default_params();
    own_sampler_params.no_perf = true;
    struct llama_sampler *own_sampler = ref_api->ref_llama_sampler_chain_init(own_sampler_params);
    if (own_sampler == NULL) {
        ref_api->ref_llama_free(own_context);
        AR__HEAP__FREE(own_prompt_tokens);
        _log_error(ref_runtime,
                   "complete() direct libllama backend could not create a sampler chain");
        return false;
    }

    ar_local_completion_buffer_t own_grammar = {0};
    if (!_build_direct_response_grammar(ref_placeholders, &own_grammar)) {
        ref_api->ref_llama_sampler_free(own_sampler);
        ref_api->ref_llama_free(own_context);
        AR__HEAP__FREE(own_prompt_tokens);
        _log_error(ref_runtime,
                   "complete() direct libllama backend could not build an output grammar");
        return false;
    }

    struct llama_sampler *own_grammar_sampler =
        ref_api->ref_llama_sampler_init_grammar(ref_vocab, own_grammar.own_text, "root");
    _buffer_destroy(&own_grammar);
    if (own_grammar_sampler == NULL) {
        ref_api->ref_llama_sampler_free(own_sampler);
        ref_api->ref_llama_free(own_context);
        AR__HEAP__FREE(own_prompt_tokens);
        _log_error(ref_runtime,
                   "complete() direct libllama backend could not initialize an output grammar sampler");
        return false;
    }

    ref_api->ref_llama_sampler_chain_add(own_sampler, own_grammar_sampler);
    ref_api->ref_llama_sampler_chain_add(own_sampler, ref_api->ref_llama_sampler_init_greedy());

    struct llama_batch own_batch = ref_api->ref_llama_batch_get_one(own_prompt_tokens, actual_token_count);
    if (ref_api->ref_llama_model_has_encoder(ref_runtime->own_llama_model)) {
        if (ref_api->ref_llama_encode(own_context, own_batch) != 0) {
            ref_api->ref_llama_sampler_free(own_sampler);
            ref_api->ref_llama_free(own_context);
            AR__HEAP__FREE(own_prompt_tokens);
            _log_error(ref_runtime,
                       "complete() direct libllama backend failed while encoding the prompt");
            return false;
        }

        llama_token own_decoder_start_token =
            ref_api->ref_llama_model_decoder_start_token(ref_runtime->own_llama_model);
        if (own_decoder_start_token == LLAMA_TOKEN_NULL) {
            own_decoder_start_token = ref_api->ref_llama_vocab_bos(ref_vocab);
        }
        own_batch = ref_api->ref_llama_batch_get_one(&own_decoder_start_token, 1);
    }

    int64_t start_time_us = ref_api->ref_llama_time_us();
    int32_t own_position = 0;
    const int32_t max_predict = 256;
    llama_token own_next_token = 0;
    bool success = true;

    while (own_position + own_batch.n_tokens < actual_token_count + max_predict) {
        int64_t elapsed_ms = (ref_api->ref_llama_time_us() - start_time_us) / 1000;
        if (elapsed_ms > timeout_ms) {
            _log_failure(ref_runtime,
                         "complete() direct libllama generation exceeded timeout_ms before producing a complete response",
                         "timeout",
                         "direct libllama generation exceeded timeout_ms before a complete response was produced",
                         "increase the timeout, simplify the template, or verify the local model/runtime configuration");
            success = false;
            break;
        }

        if (ref_api->ref_llama_decode(own_context, own_batch) != 0) {
            _log_error(ref_runtime,
                       "complete() direct libllama backend failed during token decoding");
            success = false;
            break;
        }

        own_position += own_batch.n_tokens;
        own_next_token = ref_api->ref_llama_sampler_sample(own_sampler, own_context, -1);
        if (ref_api->ref_llama_vocab_is_eog(ref_vocab, own_next_token)) {
            break;
        }

        if (!_append_generated_piece(ref_runtime, ref_vocab, ref_api, own_next_token, mut_output)) {
            success = false;
            break;
        }

        own_batch = ref_api->ref_llama_batch_get_one(&own_next_token, 1);

        if (mut_output->length > 0U && mut_output->own_text[mut_output->length - 1U] == '\n' &&
            _response_has_all_requested_placeholders(ref_placeholders, mut_output->own_text)) {
            break;
        }
    }

    ref_api->ref_llama_sampler_free(own_sampler);
    ref_api->ref_llama_free(own_context);
    AR__HEAP__FREE(own_prompt_tokens);
    return success;
}

static char *_trim_line(char *mut_line) {
    if (mut_line == NULL) {
        return NULL;
    }

    while (*mut_line == '\r' || *mut_line == '\n') {
        mut_line++;
    }

    size_t own_length = strlen(mut_line);
    while (own_length > 0U &&
           (mut_line[own_length - 1U] == '\r' || mut_line[own_length - 1U] == '\n')) {
        mut_line[own_length - 1U] = '\0';
        own_length--;
    }

    return mut_line;
}

static ar_data_t *_parse_response(const ar_local_completion_t *ref_runtime,
                                  const ar_list_t *ref_placeholders,
                                  const char *ref_stdout) {
    if (ref_placeholders == NULL || ref_stdout == NULL) {
        return NULL;
    }

    ar_data_t *own_values = ar_data__create_map();
    if (own_values == NULL) {
        return NULL;
    }

    char *own_stdout_copy = AR__HEAP__STRDUP(ref_stdout, "local_completion_stdout_copy");
    if (own_stdout_copy == NULL) {
        ar_data__destroy(own_values);
        return NULL;
    }

    ar_local_completion_placeholder_items_t own_placeholder_view = {0};
    if (!_placeholder_items_init(ref_placeholders, &own_placeholder_view)) {
        AR__HEAP__FREE(own_stdout_copy);
        ar_data__destroy(own_values);
        return NULL;
    }

    for (char *ref_line = strtok(own_stdout_copy, "\n"); ref_line != NULL; ref_line = strtok(NULL, "\n")) {
        char *ref_trimmed = _trim_line(ref_line);
        if (ref_trimmed == NULL || ref_trimmed[0] == '\0') {
            continue;
        }

        char *mut_equals = strchr(ref_trimmed, '=');
        if (mut_equals == NULL) {
            continue;
        }
        *mut_equals = '\0';
        char *ref_name = ref_trimmed;
        char *ref_value = _trim_line(mut_equals + 1);
        if (ref_name[0] == '\0') {
            continue;
        }

        for (size_t index = 0U; index < own_placeholder_view.count; index++) {
            const char *ref_requested = own_placeholder_view.own_items[index];
            if (ref_requested != NULL && strcmp(ref_name, ref_requested) == 0) {
                if (!ar_data__set_map_string(own_values, ref_requested, ref_value)) {
                    _placeholder_items_destroy(&own_placeholder_view);
                    AR__HEAP__FREE(own_stdout_copy);
                    ar_data__destroy(own_values);
                    return NULL;
                }
                break;
            }
        }
    }

    for (size_t index = 0U; index < own_placeholder_view.count; index++) {
        const char *ref_requested = own_placeholder_view.own_items[index];
        if (ref_requested == NULL || ar_data__get_map_data(own_values, ref_requested) == NULL) {
            ar_local_completion_buffer_t own_message = {0};
            bool built_message =
                ref_requested != NULL &&
                _buffer_append_format(&own_message,
                                      "complete() local backend did not return placeholder '%s'; recovery_hint=adjust the prompt, model, or direct libllama configuration",
                                      ref_requested);
            if (built_message) {
                _log_failure(ref_runtime,
                             own_message.own_text,
                             "incomplete_placeholder",
                             "local backend did not return every requested placeholder",
                             "adjust the prompt, model, or direct libllama configuration");
            } else {
                _log_failure(ref_runtime,
                             "complete() local backend omitted a requested placeholder",
                             "incomplete_placeholder",
                             "local backend did not return every requested placeholder",
                             "adjust the prompt, model, or direct libllama configuration");
            }
            _buffer_destroy(&own_message);
            _placeholder_items_destroy(&own_placeholder_view);
            AR__HEAP__FREE(own_stdout_copy);
            ar_data__destroy(own_values);
            return NULL;
        }
    }

    _placeholder_items_destroy(&own_placeholder_view);
    AR__HEAP__FREE(own_stdout_copy);
    return own_values;
}

ar_local_completion_t *ar_local_completion__create(ar_log_t *ref_log) {
    ar_local_completion_t *own_runtime =
        AR__HEAP__CALLOC(1U, sizeof(ar_local_completion_t), "local_completion_runtime");
    if (own_runtime == NULL) {
        return NULL;
    }

    own_runtime->ref_log = ref_log;
    own_runtime->own_model_path = _resolve_model_path();
    own_runtime->own_runner_path = _resolve_runner_path();
    own_runtime->own_backend_initialized = false;
    own_runtime->own_llama_library = NULL;
    own_runtime->own_llama_model = NULL;
    memset(&own_runtime->llama_api, 0, sizeof(own_runtime->llama_api));
    return own_runtime;
}

void ar_local_completion__destroy(ar_local_completion_t *own_runtime) {
    if (own_runtime == NULL) {
        return;
    }

    if (own_runtime->own_llama_model != NULL) {
        own_runtime->llama_api.ref_llama_model_free(own_runtime->own_llama_model);
        own_runtime->own_llama_model = NULL;
    }
    if (own_runtime->own_llama_library != NULL) {
        own_runtime->llama_api.ref_llama_backend_free();
        dlclose(own_runtime->own_llama_library);
        own_runtime->own_llama_library = NULL;
    }
    if (own_runtime->own_model_path != NULL) {
        AR__HEAP__FREE(own_runtime->own_model_path);
        own_runtime->own_model_path = NULL;
    }
    if (own_runtime->own_runner_path != NULL) {
        AR__HEAP__FREE(own_runtime->own_runner_path);
        own_runtime->own_runner_path = NULL;
    }
    AR__HEAP__FREE(own_runtime);
}

const char *ar_local_completion__get_model_path(const ar_local_completion_t *ref_runtime) {
    if (ref_runtime == NULL) {
        return NULL;
    }
    return ref_runtime->own_model_path;
}

bool ar_local_completion__is_ready(const ar_local_completion_t *ref_runtime) {
    return ref_runtime != NULL && ref_runtime->own_backend_initialized;
}

ar_data_t *ar_local_completion__complete(ar_local_completion_t *mut_runtime,
                                         const char *ref_template,
                                         const ar_list_t *ref_placeholders,
                                         int64_t timeout_ms) {
    if (mut_runtime == NULL) {
        return NULL;
    }
    if (ref_template == NULL || ref_template[0] == '\0') {
        _log_failure(mut_runtime,
                     "complete() template is required for local completion",
                     "invalid_request",
                     "template is missing or empty",
                     "provide a non-empty template string with one or more placeholders");
        return NULL;
    }
    if (ref_placeholders == NULL) {
        _log_failure(mut_runtime,
                     "complete() placeholder list is required for local completion",
                     "invalid_request",
                     "placeholder list is missing",
                     "provide every required placeholder name before invoking local completion");
        return NULL;
    }
    if (timeout_ms <= 0) {
        _log_failure(mut_runtime,
                     "complete() timeout_ms must be positive for local completion",
                     "invalid_request",
                     "timeout_ms must be positive",
                     "pass a positive timeout before invoking local completion");
        return NULL;
    }
    if (ar_list__count(ref_placeholders) == 0U) {
        _log_failure(mut_runtime,
                     "complete() requires at least one placeholder",
                     "invalid_request",
                     "placeholder list is empty",
                     "provide one or more placeholder names before invoking local completion");
        return NULL;
    }
    if (!_ensure_backend_initialized(mut_runtime)) {
        return NULL;
    }

    ar_local_completion_buffer_t own_prompt = {0};
    if (!_build_prompt(ref_template, ref_placeholders, &own_prompt)) {
        _buffer_destroy(&own_prompt);
        _log_failure(mut_runtime,
                     "complete() could not build a local completion prompt",
                     "runtime_failure",
                     "local completion prompt could not be built",
                     "verify the template text, placeholder names, and available memory for prompt construction");
        return NULL;
    }

    ar_local_completion_buffer_t own_output = {0};
    bool run_ok = false;
    if (mut_runtime->own_runner_path != NULL) {
        run_ok = _run_runner_completion(mut_runtime, own_prompt.own_text, ref_placeholders, &own_output);
    } else {
        run_ok = _run_direct_completion(mut_runtime,
                                        own_prompt.own_text,
                                        ref_placeholders,
                                        timeout_ms,
                                        &own_output);
    }
    _buffer_destroy(&own_prompt);
    if (!run_ok) {
        _buffer_destroy(&own_output);
        return NULL;
    }

    ar_data_t *own_values = _parse_response(mut_runtime, ref_placeholders, own_output.own_text);
    _buffer_destroy(&own_output);
    return own_values;
}
