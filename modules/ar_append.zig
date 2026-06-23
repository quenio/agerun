const c = @cImport({
    @cInclude("ar_append.h");
    @cInclude("ar_data.h");
});
const ar_allocator = @import("ar_allocator.zig");

fn _createZeroResult() ?*c.ar_data_t {
    return c.ar_data__create_integer(0);
}

pub export fn ar_append__create_result(
    ref_list: ?*const c.ar_data_t,
    ref_value: ?*const c.ar_data_t
) ?*c.ar_data_t {
    if (ref_list == null or ref_value == null or
        c.ar_data__get_type(ref_list) != c.AR_DATA_TYPE__LIST) {
        return _createZeroResult();
    }

    const own_result = c.ar_data__create_list() orelse return _createZeroResult();
    const source_count = c.ar_data__list_count(ref_list);

    if (source_count > 0) {
        const own_items = c.ar_data__list_items(ref_list) orelse {
            c.ar_data__destroy(own_result);
            return _createZeroResult();
        };
        defer ar_allocator.free(own_items);

        var index: usize = 0;
        while (index < source_count) : (index += 1) {
            const own_copy = c.ar_data__deep_copy(own_items[index]) orelse {
                c.ar_data__destroy(own_result);
                return _createZeroResult();
            };

            if (!c.ar_data__list_add_last_data(own_result, own_copy)) {
                c.ar_data__destroy(own_copy);
                c.ar_data__destroy(own_result);
                return _createZeroResult();
            }
        }
    }

    const own_value_copy = c.ar_data__deep_copy(ref_value) orelse {
        c.ar_data__destroy(own_result);
        return _createZeroResult();
    };

    if (!c.ar_data__list_add_last_data(own_result, own_value_copy)) {
        c.ar_data__destroy(own_value_copy);
        c.ar_data__destroy(own_result);
        return _createZeroResult();
    }

    return own_result;
}
