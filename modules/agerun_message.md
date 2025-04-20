# AgeRun Message Module

## Overview

The AgeRun Message module provides constants and utilities for handling messages in the AgeRun agent system. It centralizes message-related definitions to ensure consistency across the codebase.

## Key Features

- Defines the maximum message length for all agent communication
- Provides a single source of truth for message-related constants

## API Reference

### Constants

- `MAX_MESSAGE_LENGTH`: Maximum length of a message (1024 bytes) including null terminator.

## Usage Example

```c
#include "agerun_message.h"

// Create a buffer to hold a message
char message[MAX_MESSAGE_LENGTH];
```

## Implementation Notes

This is a lightweight module primarily focused on centralizing constants to avoid duplication across the codebase. Having these constants defined in a single location makes maintenance easier when changes are needed.