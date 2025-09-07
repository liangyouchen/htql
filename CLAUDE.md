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