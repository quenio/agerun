# DataStore Module

A simple key-value data store demonstrating Zig struct module patterns.

## Overview

The DataStore module provides an in-memory key-value store with ownership tracking. It demonstrates the proper use of Zig struct modules while maintaining AgeRun's ownership conventions.

## API Reference

### Types

#### `DataStore`
The main struct representing a key-value store.

```zig
const DataStore = @import("DataStore.zig");
```

#### `Entry`
Represents a key-value pair in the store.

```zig
pub const Entry = struct {
    own_key: []u8,
    own_value: []u8,
};
```

### Functions

#### `init`
Initialize a new DataStore with the given allocator.

```zig
pub fn init(allocator: std.mem.Allocator) !DataStore
```

**Parameters:**
- `allocator`: The allocator to use for all memory operations

**Returns:** A new initialized DataStore

**Errors:** May return allocation errors

#### `deinit`
Clean up all owned resources.

```zig
pub fn deinit(self: *DataStore) void
```

**Note:** This destroys all stored keys and values.

#### `put`
Store a key-value pair, taking ownership of both.

```zig
pub fn put(self: *DataStore, own_key: []u8, own_value: []u8) !void
```

**Parameters:**
- `own_key`: The key to store (ownership transferred)
- `own_value`: The value to store (ownership transferred)

**Behavior:** If the key already exists, the old value is destroyed and replaced.

**Errors:** May return allocation errors

#### `get`
Get a reference to a value by key.

```zig
pub fn get(self: *DataStore, ref_key: []const u8) ?[]const u8
```

**Parameters:**
- `ref_key`: The key to look up (borrowed)

**Returns:** A reference to the value if found, null otherwise

**Note:** The returned reference is valid until the value is removed or replaced.

#### `remove`
Remove a key-value pair, destroying the owned resources.

```zig
pub fn remove(self: *DataStore, ref_key: []const u8) bool
```

**Parameters:**
- `ref_key`: The key to remove (borrowed)

**Returns:** true if the key was found and removed, false otherwise

#### `isEmpty`
Check if the store is empty.

```zig
pub fn isEmpty(self: *const DataStore) bool
```

**Returns:** true if the store contains no entries

#### `count`
Get the number of entries in the store.

```zig
pub fn count(self: *const DataStore) usize
```

**Returns:** The number of key-value pairs stored

#### `clear`
Clear all entries, destroying owned resources.

```zig
pub fn clear(self: *DataStore) void
```

**Note:** This destroys all stored keys and values but retains allocated capacity.

#### `getAllKeys`
Get all keys as a newly allocated slice.

```zig
pub fn getAllKeys(self: *const DataStore) ![][]u8
```

**Returns:** A newly allocated array of key copies (caller owns)

**Errors:** May return allocation errors

**Note:** The caller is responsible for freeing both the array and each key string.

## Usage Example

```zig
const std = @import("std");
const DataStore = @import("DataStore.zig");

// Initialize store
var store = try DataStore.init(allocator);
defer store.deinit();

// Store a value
const own_key = try allocator.dupe(u8, "config");
const own_value = try allocator.dupe(u8, "debug mode");
try store.put(own_key, own_value);

// Retrieve value
if (store.get("config")) |value| {
    std.debug.print("Config: {s}\n", .{value});
}

// Remove value
_ = store.remove("config");
```

## Design Notes

1. **Ownership Model**: The store takes ownership of both keys and values, ensuring proper cleanup on removal or destruction.

2. **Last Accessed Tracking**: The store maintains a `mut_last_accessed` pointer for potential cache optimization patterns.

3. **Ordered Storage**: Entries are stored in insertion order using `ArrayList`, making `getAllKeys` predictable.

4. **Simplicity**: Uses simple string values to demonstrate the pattern without external dependencies.

## Testing

Run tests with:
```bash
zig test modules/DataStoreTests.zig
```

The test suite covers:
- Initialization and cleanup
- Basic put/get operations
- Key replacement behavior
- Removal operations
- Clear functionality
- Key enumeration
- Last accessed tracking