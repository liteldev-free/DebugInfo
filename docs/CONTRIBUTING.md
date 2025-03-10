---
icon: octicons/repo-push-16
---

# Contributing Guide

We welcome contributions of any kind, including but not limited to

- Open a new issue and tell us what you think.
- Contribute directly to the _DebugInfo_ toolchain.
- Correct documentation errors, including translation errors.
- Write a new tutorial.
- Any others may not be listed.

## Coding style

- All code must be formatted before submission (follow `ruff.toml` and `.clang-format`)
- Except for the class name which uses CamelCase, all others use snake_style.
- Private member names begin with `m_` (C++ code only).

## Interface Stability

- Great, you don't need to make ABI stability guarantees!
- Tool usage should not be significantly modified (as appropriate)
