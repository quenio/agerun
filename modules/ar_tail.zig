const c = @cImport({
    @cInclude("ar_tail.h");
    @cInclude("ar_data.h");
});
const ar_allocator = @import("ar_allocator.zig");

fn _createZeroResult() ?*c.ar_data_t {
    return c.ar_data__create_integer(0);
}

pub export fn ar_tail__create_result(
    ref_list: ?*const c.ar_data_t
) ?*c.ar_data_t {
    if (ref_list == null or c.ar_data__get_type(ref_list) != c.AR_DATA_TYPE__LIST) {
        return _createZeroResult();
    }

    const own_tail = c.ar_data__create_list() orelse return _createZeroResult();
    const source_count = c.ar_data__list_count(ref_list);
    if (source_count <= 1) {
        return own_tail;
    }

    const own_items = c.ar_data__list_items(ref_list) orelse {
        c.ar_data__destroy(own_tail);
        return _createZeroResult();
    };
    defer ar_allocator.free(own_items);

    var index: usize = 1;
    while (index < source_count) : (index += 1) {
        const ref_item: ?*const c.ar_data_t = own_items[index];
        const own_copy = c.ar_data__deep_copy(ref_item) orelse {
            c.ar_data__destroy(own_tail);
            return _createZeroResult();
        };

        if (!c.ar_data__list_add_last_data(own_tail, own_copy)) {
            c.ar_data__destroy(own_copy);
            c.ar_data__destroy(own_tail);
            return _createZeroResult();
        }
    }

    return own_tail;
}
