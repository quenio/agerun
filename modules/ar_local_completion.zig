const std = @import("std");
const c = @cImport({
    @cInclude("ar_local_completion.h");
    @cInclude("ar_data.h");
    @cInclude("ar_log.h");
});
const have_llama = @hasInclude("llama.h");
const llama_api = if (have_llama)
    @cImport({
        @cInclude("llama.h");
    })
else
    struct {};
const ar_allocator = @import("ar_allocator.zig");
const ProcessChild = std.process.Child;
const ProcessChildRunResult = std.process.Child.RunResult;
const runProcessChild = ProcessChild.run;
const LlamaToken = if (have_llama) @FieldType(llama_api, "llama_token") else i32;

var g_llama_backend_initialized = false;
var g_llama_backend_user_count: usize = 0;

const ar_local_completion_t = struct {
    ref_log: ?*c.ar_log_t,
    own_model_path: ?[*:0]u8,
    own_runner_path: ?[*:0]u8,
    own_backend_initialized: bool,
    own_llama_model: ?*anyopaque,
};

fn _logError(ref_runtime: ?*const ar_local_completion_t, ref_message: [*:0]const u8) void {
    if (ref_runtime != null and ref_runtime.?.ref_log != null) {
        c.ar_log__error(ref_runtime.?.ref_log, ref_message);
    }
}

fn _logOwnedMessage(ref_runtime: ?*const ar_local_completion_t, own_message: *std.ArrayList(u8)) void {
    own_message.append(0) catch {
        _logError(ref_runtime, "complete() failed to construct a detailed runtime error message");
        return;
    };
    _logError(ref_runtime, @ptrCast(own_message.items.ptr));
}

fn _dupOptional(ref_text: ?[:0]const u8, ref_desc: [*:0]const u8) ?[*:0]u8 {
    if (ref_text == null) return null;
    if (ref_text.?.len == 0) return null;
    return ar_allocator.dupe(@as([*:0]const u8, @ptrCast(ref_text.?.ptr)), ref_desc);
}

fn _defaultModelPath() ?[*:0]u8 {
    return ar_allocator.dupe("models/phi-3-mini-q4.gguf", "local_completion_default_model_path");
}

fn _alternateModelPath() ?[*:0]u8 {
    return ar_allocator.dupe("../../models/phi-3-mini-q4.gguf", "local_completion_alternate_model_path");
}

fn _resolveModelPath() ?[*:0]u8 {
    const own_override = _dupOptional(std.posix.getenvZ("AGERUN_COMPLETE_MODEL"), "local_completion_model_path");
    if (own_override != null) return own_override;

    const own_default = _defaultModelPath() orelse return null;
    if (_fileExists(own_default)) return own_default;
    ar_allocator.free(own_default);

    const own_alternate = _alternateModelPath() orelse return null;
    if (_fileExists(own_alternate)) return own_alternate;
    ar_allocator.free(own_alternate);

    return _defaultModelPath();
}

fn _resolveRunnerPath() ?[*:0]u8 {
    return _dupOptional(std.posix.getenvZ("AGERUN_COMPLETE_RUNNER"), "local_completion_runner_path");
}

fn _fileExists(ref_path: [*:0]const u8) bool {
    std.fs.cwd().access(std.mem.span(ref_path), .{}) catch {
        return false;
    };
    return true;
}

fn _ensureLlamaBackendInitialized(mut_runtime: *ar_local_completion_t) bool {
    if (!have_llama) {
        _logError(
            mut_runtime,
            "complete() direct libllama support is unavailable; run make vendor-llama-cpu or set AGERUN_COMPLETE_RUNNER for an explicit local runner override",
        );
        return false;
    }

    const ref_model_path = mut_runtime.own_model_path orelse {
        _logError(mut_runtime, "complete() local model path is not configured; set AGERUN_COMPLETE_MODEL or provide models/phi-3-mini-q4.gguf");
        return false;
    };
    if (std.mem.len(ref_model_path) == 0) {
        _logError(mut_runtime, "complete() local model path is empty; set AGERUN_COMPLETE_MODEL or provide models/phi-3-mini-q4.gguf");
        return false;
    }
    if (!_fileExists(ref_model_path)) {
        _logError(mut_runtime, "complete() local model file was not found; set AGERUN_COMPLETE_MODEL or provide models/phi-3-mini-q4.gguf");
        return false;
    }

    if (!g_llama_backend_initialized) {
        @field(llama_api, "llama_backend_init")();
        g_llama_backend_initialized = true;
    }

    var model_params = @field(llama_api, "llama_model_default_params")();
    model_params.n_gpu_layers = 0;

    const own_model = @field(llama_api, "llama_model_load_from_file")(ref_model_path, model_params) orelse {
        _logError(
            mut_runtime,
            "complete() could not load the configured GGUF model with direct libllama; recovery_hint=verify AGERUN_COMPLETE_MODEL, the GGUF file, and the vendored CPU-only build",
        );
        if (g_llama_backend_initialized and g_llama_backend_user_count == 0) {
            @field(llama_api, "llama_backend_free")();
            g_llama_backend_initialized = false;
        }
        return false;
    };

    mut_runtime.own_llama_model = own_model;
    mut_runtime.own_backend_initialized = true;
    g_llama_backend_user_count += 1;
    return true;
}

fn _ensureBackendInitialized(mut_runtime: *ar_local_completion_t) bool {
    if (mut_runtime.own_backend_initialized) return true;
    if (mut_runtime.own_model_path == null or std.mem.len(mut_runtime.own_model_path.?) == 0) {
        _logError(mut_runtime, "complete() local model path is not configured; set AGERUN_COMPLETE_MODEL or provide models/phi-3-mini-q4.gguf");
        return false;
    }

    if (mut_runtime.own_runner_path != null) {
        if (std.mem.len(mut_runtime.own_runner_path.?) == 0) {
            _logError(mut_runtime, "complete() local runner override is empty; set AGERUN_COMPLETE_RUNNER to a llama.cpp-compatible executable path");
            return false;
        }
        mut_runtime.own_backend_initialized = true;
        return true;
    }

    return _ensureLlamaBackendInitialized(mut_runtime);
}

fn _makeCStringFromSlice(ref_text: []const u8, ref_desc: [*:0]const u8) ?[*:0]u8 {
    const own_text = ar_allocator.alloc(u8, ref_text.len + 1, ref_desc) orelse {
        return null;
    };
    @memcpy(own_text[0..ref_text.len], ref_text);
    own_text[ref_text.len] = 0;
    return @ptrCast(own_text);
}

fn _buildPrompt(ref_template: [*:0]const u8, ref_placeholders: *const c.ar_list_t) ?[]u8 {
    var own_prompt = std.ArrayList(u8).init(std.heap.c_allocator);
    errdefer own_prompt.deinit();

    const writer = own_prompt.writer();
    writer.print(
        "You are filling an AgeRun complete() template.\n" ++
            "Return exactly one line per requested placeholder in the form name=value.\n" ++
            "Do not add commentary, numbering, JSON, or markdown.\n" ++
            "Do not include braces in values.\n" ++
            "Template: {s}\nRequested placeholders:\n",
        .{std.mem.span(ref_template)},
    ) catch return null;

    const placeholder_count = c.ar_list__count(ref_placeholders);
    const own_items = c.ar_list__items(ref_placeholders) orelse return null;
    defer ar_allocator.free(own_items);

    var index: usize = 0;
    while (index < placeholder_count) : (index += 1) {
        const ref_name: ?[*:0]const u8 = @ptrCast(own_items[index]);
        if (ref_name == null) return null;
        writer.print("- {s}\n", .{std.mem.span(ref_name.?)}) catch return null;
    }

    writer.writeAll("Output only the requested placeholder assignments.\n") catch return null;
    return own_prompt.toOwnedSlice() catch null;
}

fn _runRunnerCompletion(mut_runtime: *ar_local_completion_t, ref_prompt: []const u8) ?ProcessChildRunResult {
    const ref_runner = mut_runtime.own_runner_path orelse return null;
    const ref_model = mut_runtime.own_model_path orelse return null;

    const own_prompt_z = _makeCStringFromSlice(ref_prompt, "local_completion_prompt") orelse {
        return null;
    };
    defer ar_allocator.free(own_prompt_z);

    return runProcessChild(.{
        .allocator = std.heap.c_allocator,
        .argv = &[_][]const u8{
            std.mem.span(ref_runner),
            "-m",
            std.mem.span(ref_model),
            "-p",
            std.mem.span(own_prompt_z),
            "-n",
            "128",
            "--temp",
            "0",
        },
    }) catch |err| {
        var own_message = std.ArrayList(u8).init(std.heap.c_allocator);
        defer own_message.deinit();
        own_message.writer().print(
            "complete() local runner failed: cause={s}; recovery_hint=install llama.cpp llama-cli or set AGERUN_COMPLETE_RUNNER",
            .{@errorName(err)},
        ) catch {
            _logError(mut_runtime, "complete() local runner failed to start; recovery_hint=install llama.cpp llama-cli or set AGERUN_COMPLETE_RUNNER");
            return null;
        };
        _logOwnedMessage(mut_runtime, &own_message);
        return null;
    };
}

fn _trimLine(ref_line: []const u8) []const u8 {
    return std.mem.trim(u8, ref_line, "\r\n");
}

fn _responseHasAllRequestedPlaceholders(ref_placeholders: *const c.ar_list_t, ref_output: []const u8) bool {
    const placeholder_count = c.ar_list__count(ref_placeholders);
    const own_items = c.ar_list__items(ref_placeholders) orelse return false;
    defer ar_allocator.free(own_items);

    var index: usize = 0;
    while (index < placeholder_count) : (index += 1) {
        const ref_name: ?[*:0]const u8 = @ptrCast(own_items[index]);
        if (ref_name == null) return false;

        var own_marker = std.ArrayList(u8).init(std.heap.c_allocator);
        defer own_marker.deinit();
        own_marker.writer().print("{s}=", .{std.mem.span(ref_name.?)}) catch return false;
        if (std.mem.indexOf(u8, ref_output, own_marker.items) == null) {
            return false;
        }
    }

    return true;
}

fn _appendTokenPiece(ref_output: *std.ArrayList(u8), ref_vocab: ?*const anyopaque, token: LlamaToken) bool {
    if (!have_llama) return false;
    if (ref_vocab == null) return false;

    const ref_llama_vocab: *const @FieldType(llama_api, "struct_llama_vocab") = @ptrCast(ref_vocab.?);

    var own_stack_buf: [256]u8 = undefined;
    var piece_len = @field(llama_api, "llama_token_to_piece")(
        ref_llama_vocab,
        token,
        @ptrCast(&own_stack_buf),
        @intCast(own_stack_buf.len),
        0,
        true,
    );
    if (piece_len < 0) {
        const needed_len: usize = @intCast(-piece_len);
        const own_buf = std.heap.c_allocator.alloc(u8, needed_len) catch return false;
        defer std.heap.c_allocator.free(own_buf);

        piece_len = @field(llama_api, "llama_token_to_piece")(
            ref_llama_vocab,
            token,
            @ptrCast(own_buf.ptr),
            @intCast(own_buf.len),
            0,
            true,
        );
        if (piece_len < 0) return false;

        ref_output.appendSlice(own_buf[0..@intCast(piece_len)]) catch return false;
        return true;
    }

    ref_output.appendSlice(own_stack_buf[0..@intCast(piece_len)]) catch return false;
    return true;
}

fn _runDirectCompletion(
    mut_runtime: *ar_local_completion_t,
    ref_prompt: []const u8,
    ref_placeholders: *const c.ar_list_t,
    timeout_ms: i64,
) ?[]u8 {
    if (!have_llama) {
        _logError(
            mut_runtime,
            "complete() direct libllama support is unavailable; run make vendor-llama-cpu or set AGERUN_COMPLETE_RUNNER for an explicit local runner override",
        );
        return null;
    }

    const ref_model_any = mut_runtime.own_llama_model orelse {
        _logError(mut_runtime, "complete() direct libllama backend is not initialized");
        return null;
    };
    const ref_model: *@FieldType(llama_api, "struct_llama_model") = @ptrCast(ref_model_any);
    const ref_vocab = @field(llama_api, "llama_model_get_vocab")(ref_model);
    if (ref_vocab == null) {
        _logError(mut_runtime, "complete() direct libllama backend could not access the loaded model vocabulary");
        return null;
    }

    const prompt_token_count = -@field(llama_api, "llama_tokenize")(
        ref_vocab,
        @ptrCast(ref_prompt.ptr),
        @intCast(ref_prompt.len),
        null,
        0,
        true,
        true,
    );
    if (prompt_token_count <= 0) {
        _logError(mut_runtime, "complete() direct libllama backend could not tokenize the completion prompt");
        return null;
    }

    const own_prompt_tokens = std.heap.c_allocator.alloc(LlamaToken, @intCast(prompt_token_count)) catch {
        _logError(mut_runtime, "complete() direct libllama backend could not allocate prompt tokens");
        return null;
    };
    defer std.heap.c_allocator.free(own_prompt_tokens);

    const actual_token_count = @field(llama_api, "llama_tokenize")(
        ref_vocab,
        @ptrCast(ref_prompt.ptr),
        @intCast(ref_prompt.len),
        @ptrCast(own_prompt_tokens.ptr),
        @intCast(own_prompt_tokens.len),
        true,
        true,
    );
    if (actual_token_count < 0) {
        _logError(mut_runtime, "complete() direct libllama backend failed to tokenize the completion prompt");
        return null;
    }

    var ctx_params = @field(llama_api, "llama_context_default_params")();
    const context_tokens: usize = own_prompt_tokens.len + 256;
    ctx_params.n_ctx = @intCast(context_tokens);
    ctx_params.n_batch = @intCast(own_prompt_tokens.len);
    ctx_params.no_perf = true;

    const own_ctx = @field(llama_api, "llama_init_from_model")(ref_model, ctx_params) orelse {
        _logError(mut_runtime, "complete() direct libllama backend could not create a generation context");
        return null;
    };
    defer @field(llama_api, "llama_free")(own_ctx);

    var sampler_params = @field(llama_api, "llama_sampler_chain_default_params")();
    sampler_params.no_perf = true;
    const own_sampler = @field(llama_api, "llama_sampler_chain_init")(sampler_params) orelse {
        _logError(mut_runtime, "complete() direct libllama backend could not create a sampler chain");
        return null;
    };
    defer @field(llama_api, "llama_sampler_free")(own_sampler);
    @field(llama_api, "llama_sampler_chain_add")(own_sampler, @field(llama_api, "llama_sampler_init_greedy")());

    var batch = @field(llama_api, "llama_batch_get_one")(@ptrCast(own_prompt_tokens.ptr), @intCast(own_prompt_tokens.len));
    if (@field(llama_api, "llama_model_has_encoder")(ref_model)) {
        if (@field(llama_api, "llama_encode")(own_ctx, batch) != 0) {
            _logError(mut_runtime, "complete() direct libllama backend failed while encoding the prompt");
            return null;
        }

        var decoder_start_token = @field(llama_api, "llama_model_decoder_start_token")(ref_model);
        if (decoder_start_token == @field(llama_api, "LLAMA_TOKEN_NULL")) {
            decoder_start_token = @field(llama_api, "llama_vocab_bos")(ref_vocab);
        }
        batch = @field(llama_api, "llama_batch_get_one")(&decoder_start_token, 1);
    }

    const start_us = @field(llama_api, "llama_time_us")();
    var own_output = std.ArrayList(u8).init(std.heap.c_allocator);
    errdefer own_output.deinit();
    var next_token: LlamaToken = 0;
    var n_pos: i32 = 0;
    const max_predict: i32 = 128;

    while (n_pos + batch.n_tokens < actual_token_count + max_predict) {
        const elapsed_ms = (@field(llama_api, "llama_time_us")() - start_us) / 1000;
        if (elapsed_ms > timeout_ms) {
            _logError(mut_runtime, "complete() direct libllama generation exceeded timeout_ms before producing a complete response");
            return null;
        }

        if (@field(llama_api, "llama_decode")(own_ctx, batch) != 0) {
            _logError(mut_runtime, "complete() direct libllama backend failed during token decoding");
            return null;
        }
        n_pos += batch.n_tokens;

        next_token = @field(llama_api, "llama_sampler_sample")(own_sampler, own_ctx, -1);
        if (@field(llama_api, "llama_vocab_is_eog")(ref_vocab, next_token)) {
            break;
        }
        if (!_appendTokenPiece(&own_output, ref_vocab, next_token)) {
            _logError(mut_runtime, "complete() direct libllama backend could not decode a generated token into text");
            return null;
        }

        @field(llama_api, "llama_sampler_accept")(own_sampler, next_token);
        batch = @field(llama_api, "llama_batch_get_one")(&next_token, 1);

        if (own_output.items.len > 0 and own_output.items[own_output.items.len - 1] == '\n') {
            if (_responseHasAllRequestedPlaceholders(ref_placeholders, own_output.items)) {
                break;
            }
        }
    }

    return own_output.toOwnedSlice() catch null;
}

fn _parseResponse(
    mut_runtime: *ar_local_completion_t,
    ref_placeholders: *const c.ar_list_t,
    ref_stdout: []const u8,
) ?*c.ar_data_t {
    const own_values = c.ar_data__create_map() orelse return null;
    errdefer c.ar_data__destroy(own_values);

    const placeholder_count = c.ar_list__count(ref_placeholders);
    const own_items = c.ar_list__items(ref_placeholders) orelse return null;
    defer ar_allocator.free(own_items);

    var line_it = std.mem.splitScalar(u8, ref_stdout, '\n');
    while (line_it.next()) |ref_line| {
        const ref_trimmed = _trimLine(ref_line);
        if (ref_trimmed.len == 0) continue;
        const equals_index = std.mem.indexOfScalar(u8, ref_trimmed, '=') orelse continue;
        const ref_name = std.mem.trim(u8, ref_trimmed[0..equals_index], " \t");
        const ref_value = _trimLine(ref_trimmed[equals_index + 1 ..]);
        if (ref_name.len == 0) continue;

        var index: usize = 0;
        while (index < placeholder_count) : (index += 1) {
            const ref_requested: ?[*:0]const u8 = @ptrCast(own_items[index]);
            if (ref_requested == null) continue;
            if (std.mem.eql(u8, ref_name, std.mem.span(ref_requested.?))) {
                const own_value = ar_allocator.alloc(u8, ref_value.len + 1, "local_completion_response_value") orelse return null;
                defer ar_allocator.free(own_value);
                @memcpy(own_value[0..ref_value.len], ref_value);
                own_value[ref_value.len] = 0;
                if (!c.ar_data__set_map_string(own_values, ref_requested.?, @ptrCast(own_value))) {
                    return null;
                }
                break;
            }
        }
    }

    var index: usize = 0;
    while (index < placeholder_count) : (index += 1) {
        const ref_requested: ?[*:0]const u8 = @ptrCast(own_items[index]);
        if (ref_requested == null) return null;
        if (c.ar_data__get_map_data(own_values, ref_requested.?) == null) {
            var own_message = std.ArrayList(u8).init(std.heap.c_allocator);
            defer own_message.deinit();
            own_message.writer().print(
                "complete() local backend did not return placeholder '{s}'; recovery_hint=adjust the prompt, model, or direct libllama configuration",
                .{std.mem.span(ref_requested.?)},
            ) catch {
                _logError(mut_runtime, "complete() local backend omitted a requested placeholder");
                return null;
            };
            _logOwnedMessage(mut_runtime, &own_message);
            return null;
        }
    }

    return own_values;
}

export fn ar_local_completion__create(ref_log: ?*c.ar_log_t) ?*ar_local_completion_t {
    const own_runtime = ar_allocator.create(ar_local_completion_t, "local_completion") orelse return null;
    own_runtime.ref_log = ref_log;
    own_runtime.own_model_path = _resolveModelPath();
    own_runtime.own_runner_path = _resolveRunnerPath();
    own_runtime.own_backend_initialized = false;
    own_runtime.own_llama_model = null;
    return own_runtime;
}

export fn ar_local_completion__destroy(own_runtime: ?*ar_local_completion_t) void {
    if (own_runtime == null) return;
    if (have_llama and own_runtime.?.own_llama_model != null) {
        @field(llama_api, "llama_model_free")(@ptrCast(own_runtime.?.own_llama_model.?));
        own_runtime.?.own_llama_model = null;
        if (g_llama_backend_user_count > 0) {
            g_llama_backend_user_count -= 1;
        }
        if (g_llama_backend_user_count == 0 and g_llama_backend_initialized) {
            @field(llama_api, "llama_backend_free")();
            g_llama_backend_initialized = false;
        }
    }
    if (own_runtime.?.own_model_path != null) ar_allocator.free(own_runtime.?.own_model_path);
    if (own_runtime.?.own_runner_path != null) ar_allocator.free(own_runtime.?.own_runner_path);
    ar_allocator.free(own_runtime);
}

export fn ar_local_completion__get_model_path(ref_runtime: ?*const ar_local_completion_t) ?[*:0]const u8 {
    if (ref_runtime == null) return null;
    return ref_runtime.?.own_model_path;
}

export fn ar_local_completion__is_ready(ref_runtime: ?*const ar_local_completion_t) bool {
    return ref_runtime != null and ref_runtime.?.own_backend_initialized;
}

export fn ar_local_completion__complete(
    mut_runtime: ?*ar_local_completion_t,
    ref_template: ?[*:0]const u8,
    ref_placeholders: ?*const c.ar_list_t,
    timeout_ms: i64,
) ?*c.ar_data_t {
    if (mut_runtime == null) {
        return null;
    }
    if (ref_template == null or std.mem.len(ref_template.?) == 0) {
        _logError(mut_runtime, "complete() template is required for local completion");
        return null;
    }
    if (ref_placeholders == null) {
        _logError(mut_runtime, "complete() placeholder list is required for local completion");
        return null;
    }
    if (timeout_ms <= 0) {
        _logError(mut_runtime, "complete() timeout_ms must be positive for local completion");
        return null;
    }
    if (c.ar_list__count(ref_placeholders) == 0) {
        _logError(mut_runtime, "complete() requires at least one placeholder");
        return null;
    }
    if (!_ensureBackendInitialized(mut_runtime.?)) {
        return null;
    }

    const own_prompt = _buildPrompt(ref_template.?, ref_placeholders.?) orelse {
        _logError(mut_runtime, "complete() could not build a local completion prompt");
        return null;
    };
    defer std.heap.c_allocator.free(own_prompt);

    if (mut_runtime.?.own_runner_path != null) {
        const result = _runRunnerCompletion(mut_runtime.?, own_prompt) orelse return null;
        defer std.heap.c_allocator.free(result.stdout);
        defer std.heap.c_allocator.free(result.stderr);

        switch (result.term) {
            .Exited => |code| {
                if (code != 0) {
                    var own_message = std.ArrayList(u8).init(std.heap.c_allocator);
                    defer own_message.deinit();
                    own_message.writer().print(
                        "complete() local runner exited with code {d}: {s}; recovery_hint=verify the explicit AGERUN_COMPLETE_RUNNER override, model path, and prompt format",
                        .{ code, _trimLine(result.stderr) },
                    ) catch {
                        _logError(mut_runtime, "complete() local runner exited with an error");
                        return null;
                    };
                    _logOwnedMessage(mut_runtime, &own_message);
                    return null;
                }
            },
            else => {
                _logError(mut_runtime, "complete() local runner did not exit normally; recovery_hint=verify the explicit AGERUN_COMPLETE_RUNNER override is stable on this platform");
                return null;
            },
        }

        return _parseResponse(mut_runtime.?, ref_placeholders.?, result.stdout);
    }

    const own_output = _runDirectCompletion(mut_runtime.?, own_prompt, ref_placeholders.?, timeout_ms) orelse return null;
    defer std.heap.c_allocator.free(own_output);
    return _parseResponse(mut_runtime.?, ref_placeholders.?, own_output);
}
