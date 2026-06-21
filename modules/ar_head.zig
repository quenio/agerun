const c = @cImport({
    @cInclude("ar_head.h");
    @cInclude("ar_data.h");
});

fn _createZeroResult() ?*c.ar_data_t {
    return c.ar_data__create_integer(0);
}

pub export fn ar_head__create_result(
    ref_list: ?*const c.ar_data_t
) ?*c.ar_data_t {
    if (ref_list == null or c.ar_data__get_type(ref_list) != c.AR_DATA_TYPE__LIST) {
        return _createZeroResult();
    }

    if (c.ar_data__list_count(ref_list) == 0) {
        return _createZeroResult();
    }

    const ref_first = c.ar_data__list_first(ref_list) orelse return _createZeroResult();
    return c.ar_data__deep_copy(ref_first) orelse _createZeroResult();
}
