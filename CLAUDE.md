# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Overview

HTQL (Hyper-Text Query Language) is a C++-based library for querying and transforming HTML, XML, and text documents. The project provides Python bindings through a compiled extension module.

## Development Commands

### Build and Installation
```bash
python setup.py install
```

### Testing
```bash
python tests/test_basic_regex.py
```

To run a specific test case:
```bash
python tests/test_basic_regex.py --i <test_index>
```

## Architecture

### Core Components

- **C++ Core Library** (`cpp/` directory): Contains the main HTQL implementation
  - `htql.cpp/h`: Core HTQL query engine
  - `qhtql.cpp/h`: Query processing engine (132K+ lines)
  - `RegExParser.cpp/h`: Extended regular expression parser with named sets
  - `htmlbuf.cpp/h`: HTML parsing and buffering
  - `docbase.cpp/h`: Document database functionality
  - `txregex.cpp/h`: Text regex utilities
  - `pyhtql*.cpp/h`: Python interface modules

- **Python Interface**: Python extension module built from C++ sources
  - Primary classes: `htql.RegEx()` for regex operations, `htql.query()` for HTML queries
  - Supports both string and list-based pattern matching

### Key Features
- HTML/XML querying with CSS-like selectors (e.g., `<a>:href,tx`)
- Extended regex with named sets and variables (e.g., `&[s:states]`)
- Document transformation and modification
- HTTP retrieval capabilities
- Support for both Python 2.6+ and Python 3+

### Build System
- Uses `distutils` with extensive C++ compilation
- Platform-specific macros for Windows/Unix
- Compiles 100+ C++ source files into single Python extension
- Cross-platform support with conditional compilation flags

### Testing Strategy
The test suite (`tests/test_basic_regex.py`) contains comprehensive regex pattern tests covering:
- Basic regex patterns and quantifiers
- Extended regex syntax with named variables
- List-based pattern matching for POS-tagged data
- Index-based and overlap-based matching modes
- Group capture functionality

## Important Notes

- The C++ codebase is extensive (100+ files) and mature
- Python interface supports both legacy (Python 2.6+) and modern versions
- Build process requires C++ compiler toolchain
- Tests can be run individually by index for debugging specific patterns

## Lessons Learned

### 64-bit Pointer Truncation Bug (fixed 2026-03-14)

**Symptom**: `import htql` caused an access violation (segfault) in Python on Windows x64.

**Root cause**: `ReferLink::Data` in `cpp/referlink.h` was declared as `long`. On Windows x64 (LLP64 model), `long` is 4 bytes but pointers are 8 bytes. Code throughout the codebase stored heap pointers with `(long) ptr` casts, silently truncating the upper 32 bits. When those values were read back as pointers, they pointed to invalid memory addresses, crashing on `delete`.

**How it manifested**: `PyInit_htql` creates a default `HtmlQL` instance via `PyObject_CallObject`. The `HtmlQL` constructor builds and then destroys an `HTQLParser`, triggering `resetHtqlFunctions()` which called `delete` on a truncated pointer.

**Fix**: Changed `long Data` to `intptr_t Data` in `cpp/referlink.h` (and matching method signatures in `referlink.cpp`), and changed all `(long) ptr` casts to `(intptr_t) ptr` in `docbase.cpp`, `expr.cpp`, `HtNaiveBayes.cpp`, `htscript.cpp`, `qhtql.cpp`.

**Watch out for**: The same bug existed in two more `Data` fields confirmed by later testing (see below): `tStack::Data` in `stack.h` and `ReferLinkNamedHeap::createHeap` in `referlink.cpp`. Also `htbrowser.cpp` stored `HtBrowserCookieItem*` in `ReferLink::Data` via `(long)`. On Windows x64, always use `intptr_t`, `Py_ssize_t`, or `void*` to store pointer values, never `long` or `int`.

### Incomplete Pointer-Truncation Fixes (fixed 2026-03-14)

**Symptom**: After fixing `ReferLink::Data` to `intptr_t` and updating `link->Data = (intptr_t) ptr` assignments, calls to built-in functions like `text_words()` still segfaulted.

**Root cause**: Each registration function (e.g., `addInternalFunction`, `registerFunction`, `registerInterface`) had two branches — an "update existing" branch that used `link->Data = (intptr_t) ptr` (fixed) and an "insert new" branch that called `.add(..., (long) ptr)` (not fixed). On the **first** call for any function name, the `else` (insert) branch runs and the pointer is still truncated at the call site before being passed to `.add()`, even though `.add()` now accepts `intptr_t`.

**Fix**: Change every explicit `(long) ptr` cast at `.add()` call sites to `(intptr_t) ptr` — not just the `link->Data = ...` assignments. Affected locations:
- `expr.cpp`: `RegisteredFunctions.add(..., (long)fun)` and `InternalFunctions.add(..., (long)fun)`
- `htscript.cpp`: `ScriptInterfaces.add(..., (long)interf)`
- `qhtql.cpp`: `HtqlFunctions.add(..., (long)qlfun)`

**Lesson**: When fixing pointer-truncation, search for **all** `(long) ptr` casts that feed into `ReferLink::Data` — both direct assignments (`link->Data = ...`) and indirect ones via `.add()` arguments. The compiler gives no warning because `intptr_t` is implicitly widened from the already-truncated `long`.

**How to search**: `grep -rn "(long)" cpp/*.cpp | grep "\.add("` — inspect every result where the argument is a pointer, not an arithmetic value.

### `tStack::Data` and `ReferLinkNamedHeap` Pointer Truncation (fixed 2026-03-14)

**Symptom**: `htql.RegEx().setNameSet(name, list)` caused an access violation for any non-empty list.

**Root cause**: Two more `long` pointer-storage sites were found:
1. `referlink.cpp` — `ReferLinkNamedHeap::createHeap` stored a newly allocated `ReferLinkHeap*` with `link->Data=(long) set`. When retrieved via `(ReferLinkHeap*) link->Data`, the upper 32 bits were zero, giving an invalid address.
2. `stack.h` — `tStack::Data` was declared `long`. `qhtmlql.cpp` stored `HTQLScope*` pointers in it via `tt->Data=(long) tag`. `htbrowser.cpp` stored `HtBrowserCookieItem*` via `link->Data=(long) item` (where `link` is a `ReferLink`).

**Fix**:
- `referlink.cpp:376`: `link->Data=(long) set` → `(intptr_t) set`
- `stack.h`: `long Data` → `intptr_t Data`; updated `set()`, `search()`, and constructor signatures
- `stack.cpp`: updated 3 matching method implementations to use `intptr_t TheData`
- `htbrowser.cpp`: 3 occurrences of `link->Data=(long) item` → `(intptr_t) item`
- `qhtmlql.cpp:1034`: `tt->Data=(long) tag` → `(intptr_t) tag`

**How to search for remaining occurrences**: `grep -rn "Data=(long)" cpp/*.cpp` — any result where the RHS is a pointer (not an arithmetic literal) is a bug.

### Installed Package Shadows Inplace Build

**Symptom**: After rebuilding with `python setup.py build_ext --inplace`, tests still crashed with the old bug when run as `python tests/test_foo.py`.

**Root cause**: Running a script from a subdirectory (`tests/`) means Python adds that subdirectory — not the project root — to `sys.path[0]`. The project root `.pyd` file is therefore not on the path, and Python falls back to the installed version in `site-packages` (which was the old, unfixed build).

**Fix**: After fixing and rebuilding, also reinstall: `python setup.py install --user`. Verify with `python -c "import htql; print(htql.__file__)"` that the correct `.pyd` is loaded.

### Debugging C Extension Segfaults

To isolate a crash in `PyInit_*`, add `fprintf(stderr, ...)` checkpoints before each `PyType_Ready` call and before `PyObject_CallObject` to identify the crashing call. Then drill into C++ constructors with the same technique.

Enable Python's built-in fault handler to get a Python-level stack trace on access violations: `PYTHONFAULTHANDLER=1 python script.py` or `import faulthandler; faulthandler.enable()` at the top of the script.