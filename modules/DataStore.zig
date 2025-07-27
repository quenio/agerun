const std = @import("std");

/// A simple key-value data store demonstrating Zig struct module patterns
pub const DataStore = @This();

/// Represents a key-value pair in the store
pub const Entry = struct {
    own_key: []u8,
    own_value: []u8,
};

allocator: std.mem.Allocator,
own_entries: std.ArrayList(Entry),
mut_last_accessed: ?*Entry,

/// Initialize a new DataStore with the given allocator
pub fn init(allocator: std.mem.Allocator) !DataStore {
    return DataStore{
        .allocator = allocator,
        .own_entries = std.ArrayList(Entry).init(allocator),
        .mut_last_accessed = null,
    };
}

/// Clean up all owned resources
pub fn deinit(self: *DataStore) void {
    for (self.own_entries.items) |*entry| {
        self.allocator.free(entry.own_key);
        self.allocator.free(entry.own_value);
    }
    self.own_entries.deinit();
}

/// Store a key-value pair, taking ownership of both
pub fn put(self: *DataStore, own_key: []u8, own_value: []u8) !void {
    // Check if key already exists
    for (self.own_entries.items) |*entry| {
        if (std.mem.eql(u8, entry.own_key, own_key)) {
            // Replace existing value
            self.allocator.free(entry.own_value);
            entry.own_value = own_value;
            self.allocator.free(own_key);
            self.mut_last_accessed = entry;
            return;
        }
    }
    
    // Add new entry
    try self.own_entries.append(.{
        .own_key = own_key,
        .own_value = own_value,
    });
    self.mut_last_accessed = &self.own_entries.items[self.own_entries.items.len - 1];
}

/// Get a reference to a value by key
pub fn get(self: *DataStore, ref_key: []const u8) ?[]const u8 {
    for (self.own_entries.items) |*entry| {
        if (std.mem.eql(u8, entry.own_key, ref_key)) {
            self.mut_last_accessed = entry;
            return entry.own_value;
        }
    }
    return null;
}

/// Remove a key-value pair, destroying the owned resources
pub fn remove(self: *DataStore, ref_key: []const u8) bool {
    for (self.own_entries.items, 0..) |*entry, i| {
        if (std.mem.eql(u8, entry.own_key, ref_key)) {
            self.allocator.free(entry.own_key);
            self.allocator.free(entry.own_value);
            _ = self.own_entries.orderedRemove(i);
            if (self.mut_last_accessed == entry) {
                self.mut_last_accessed = null;
            }
            return true;
        }
    }
    return false;
}

/// Check if the store is empty
pub fn isEmpty(self: *const DataStore) bool {
    return self.own_entries.items.len == 0;
}

/// Get the number of entries in the store
pub fn count(self: *const DataStore) usize {
    return self.own_entries.items.len;
}

/// Clear all entries, destroying owned resources
pub fn clear(self: *DataStore) void {
    for (self.own_entries.items) |*entry| {
        self.allocator.free(entry.own_key);
        self.allocator.free(entry.own_value);
    }
    self.own_entries.clearRetainingCapacity();
    self.mut_last_accessed = null;
}

/// Get all keys as a newly allocated slice (caller owns)
pub fn getAllKeys(self: *const DataStore) ![][]u8 {
    const own_keys = try self.allocator.alloc([]u8, self.own_entries.items.len);
    for (self.own_entries.items, 0..) |*entry, i| {
        own_keys[i] = try self.allocator.dupe(u8, entry.own_key);
    }
    return own_keys;
}