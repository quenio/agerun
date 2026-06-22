const c = @cImport({
    @cInclude("ar_condition.h");
    @cInclude("ar_data.h");
});

pub export fn ar_condition__is_true(
    ref_value: ?*const c.ar_data_t
) bool {
    if (ref_value == null) {
        return false;
    }

    if (c.ar_data__get_type(ref_value) != c.AR_DATA_TYPE__INTEGER) {
        return false;
    }

    return c.ar_data__get_integer(ref_value) != 0;
}
