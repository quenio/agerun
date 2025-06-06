# AgeRun Development Guide for Claude

This document contains essential instructions for Claude Code to assist with AgeRun development.

## Project Overview

AgeRun is a lightweight, message-driven agent system where each agent is defined by a single method responsible for handling all incoming messages. Agents operate within a persistent runtime environment, allowing dynamic creation, versioning, pausing, resumption, and destruction.

[... rest of the existing content ...]

## Development Memories

When creating new modules, think twice before adding global state to it. Instead, prefer an opaque structure to hold the state.

In the implementation of a module, when using other modules, first read their interface, so you know which functions are available, instead of guessing function names to call.