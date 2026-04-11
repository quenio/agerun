# Contract: Shell Session Protocol

## Purpose

Define the runtime message contract between the stdio shell delegate, the built-in shell method,
and the shell session module.

## 1. Delegate -> Receiving Agent

### Input envelope

For each accepted line of stdin input, the delegate forwards a map with exactly one field:

```text
text = <input string>
```

The delegate does not interpret the shell syntax beyond creating this envelope.

## 2. Built-in Shell Method Responsibilities

The built-in shell method:
- interprets one shell input line at a time
- supports the restricted instruction subset documented in [arsh-cli.md](./arsh-cli.md)
- launches agents and sends runtime messages when requested
- exchanges shell-session state with the shell session module only through messages

## 3. Built-in Shell Method <-> Shell Session Module

The exact implementation may evolve, but the protocol must support these logical operations:

### Set a shell-session value

```text
action = set
path = <session path>
value = <data>
```

### Get a shell-session value

```text
action = get
path = <session path>
request_id = <correlation value>
```

### Return a resolved shell-session value

```text
action = resolved
request_id = <correlation value>
value = <data>
```

### Report a state-operation failure

```text
action = error
request_id = <correlation value>
reason = <string>
```

## 4. Shell-mode assignment redirection

In shell mode, `memory... := ...` syntax is preserved for the user, but the assigned value is stored
in the shell session module's memory map rather than in the receiving agent's memory map.

## 5. Acknowledgement Stages

### Normal mode

Minimum required stage:
- input envelope handed from delegate to receiving agent

### Verbose mode

Optional additional stages:
- receiving agent accepted shell input for processing
- runtime action succeeded or failed

## 6. Replies to the delegate

Runtime replies sent back to the shell delegate must:
- remain attributable to the correct sender
- be displayable asynchronously
- not corrupt the active shell session when delayed
