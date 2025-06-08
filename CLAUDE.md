# AgeRun Development Guide for Claude

This document contains essential instructions for Claude Code to assist with AgeRun development.

## Project Overview

AgeRun is a lightweight, message-driven agent system where each agent is defined by a single method responsible for handling all incoming messages. Agents operate within a persistent runtime environment, allowing dynamic creation, versioning, pausing, resumption, and destruction.

[... rest of the existing content ...]

## Development Memories

When creating new modules, think twice before adding global state to it. Instead, prefer an opaque structure to hold the state.

In the implementation of a module, when using other modules, first read their interface, so you know which functions are available, instead of guessing function names to call.

You must check what is the current dir before you try run a command with a relative path.

When running tests, ALWAYS check the current working directory first. Tests that use AGERUN_MEMORY_REPORT will create memory report log files in the current directory. To avoid polluting the project root:
- Either change to the bin/ directory before running tests: `cd bin && ./agerun_string_tests`
- Or specify an appropriate path for the memory report: `AGERUN_MEMORY_REPORT=bin/test.memory_report.log ./bin/agerun_string_tests`
- Never run tests with relative paths from the project root without considering where output files will be created

Pause before executing build commands like `make` to check if the project has custom scripts or procedures for these tasks.