const std = @import("std");
const testing = std.testing;
const DataStore = @import("DataStore.zig");

test "DataStore init and deinit" {
    var store = try DataStore.init(testing.allocator);
    defer store.deinit();
    
    try testing.expect(store.isEmpty());
    try testing.expectEqual(@as(usize, 0), store.count());
}

test "DataStore put and get" {
    var store = try DataStore.init(testing.allocator);
    defer store.deinit();
    
    // Create owned key and value
    const own_key = try testing.allocator.dupe(u8, "test_key");
    const own_value = try testing.allocator.dupe(u8, "test_value");
    
    // Store the pair
    try store.put(own_key, own_value);
    
    // Verify storage
    try testing.expectEqual(@as(usize, 1), store.count());
    try testing.expect(!store.isEmpty());
    
    // Get the value
    const ref_value = store.get("test_key");
    try testing.expect(ref_value != null);
    try testing.expectEqualStrings("test_value", ref_value.?);
}

test "DataStore put replaces existing value" {
    var store = try DataStore.init(testing.allocator);
    defer store.deinit();
    
    // First value
    const own_key1 = try testing.allocator.dupe(u8, "key");
    const own_value1 = try testing.allocator.dupe(u8, "first_value");
    try store.put(own_key1, own_value1);
    
    // Replace with new value
    const own_key2 = try testing.allocator.dupe(u8, "key");
    const own_value2 = try testing.allocator.dupe(u8, "second_value");
    try store.put(own_key2, own_value2);
    
    // Should still have one entry
    try testing.expectEqual(@as(usize, 1), store.count());
    
    // Should have new value
    const ref_value = store.get("key");
    try testing.expect(ref_value != null);
    try testing.expectEqualStrings("second_value", ref_value.?);
}

test "DataStore remove" {
    var store = try DataStore.init(testing.allocator);
    defer store.deinit();
    
    // Add some entries
    const own_key1 = try testing.allocator.dupe(u8, "key1");
    const own_value1 = try testing.allocator.dupe(u8, "value1");
    try store.put(own_key1, own_value1);
    
    const own_key2 = try testing.allocator.dupe(u8, "key2");
    const own_value2 = try testing.allocator.dupe(u8, "value2");
    try store.put(own_key2, own_value2);
    
    try testing.expectEqual(@as(usize, 2), store.count());
    
    // Remove one
    try testing.expect(store.remove("key1"));
    try testing.expectEqual(@as(usize, 1), store.count());
    
    // Verify it's gone
    try testing.expect(store.get("key1") == null);
    try testing.expect(store.get("key2") != null);
    
    // Try to remove non-existent
    try testing.expect(!store.remove("key1"));
}

test "DataStore clear" {
    var store = try DataStore.init(testing.allocator);
    defer store.deinit();
    
    // Add multiple entries
    for (0..5) |i| {
        const own_key = try std.fmt.allocPrint(testing.allocator, "key{d}", .{i});
        const own_value = try std.fmt.allocPrint(testing.allocator, "value{d}", .{i});
        try store.put(own_key, own_value);
    }
    
    try testing.expectEqual(@as(usize, 5), store.count());
    
    // Clear all
    store.clear();
    
    try testing.expect(store.isEmpty());
    try testing.expectEqual(@as(usize, 0), store.count());
}

test "DataStore getAllKeys" {
    var store = try DataStore.init(testing.allocator);
    defer store.deinit();
    
    // Add entries
    const own_key1 = try testing.allocator.dupe(u8, "alpha");
    const own_value1 = try testing.allocator.dupe(u8, "value_alpha");
    try store.put(own_key1, own_value1);
    
    const own_key2 = try testing.allocator.dupe(u8, "beta");
    const own_value2 = try testing.allocator.dupe(u8, "value_beta");
    try store.put(own_key2, own_value2);
    
    const own_key3 = try testing.allocator.dupe(u8, "gamma");
    const own_value3 = try testing.allocator.dupe(u8, "value_gamma");
    try store.put(own_key3, own_value3);
    
    // Get all keys
    const own_keys = try store.getAllKeys();
    defer {
        for (own_keys) |key| {
            testing.allocator.free(key);
        }
        testing.allocator.free(own_keys);
    }
    
    try testing.expectEqual(@as(usize, 3), own_keys.len);
    
    // Keys should be in insertion order
    try testing.expectEqualStrings("alpha", own_keys[0]);
    try testing.expectEqualStrings("beta", own_keys[1]);
    try testing.expectEqualStrings("gamma", own_keys[2]);
}

test "DataStore last accessed tracking" {
    var store = try DataStore.init(testing.allocator);
    defer store.deinit();
    
    // Initially no last accessed
    try testing.expect(store.mut_last_accessed == null);
    
    // Add entry
    const own_key1 = try testing.allocator.dupe(u8, "key1");
    const own_value1 = try testing.allocator.dupe(u8, "value1");
    try store.put(own_key1, own_value1);
    
    // Should track last accessed
    try testing.expect(store.mut_last_accessed != null);
    try testing.expectEqualStrings("key1", store.mut_last_accessed.?.own_key);
    
    // Add another
    const own_key2 = try testing.allocator.dupe(u8, "key2");
    const own_value2 = try testing.allocator.dupe(u8, "value2");
    try store.put(own_key2, own_value2);
    try testing.expectEqualStrings("key2", store.mut_last_accessed.?.own_key);
    
    // Access first one
    _ = store.get("key1");
    try testing.expectEqualStrings("key1", store.mut_last_accessed.?.own_key);
    
    // Remove the last accessed
    try testing.expect(store.remove("key1"));
    try testing.expect(store.mut_last_accessed == null);
}