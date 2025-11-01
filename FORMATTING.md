# Code Formatting Setup

This project uses `clang-format` for automatic C/C++ code formatting to ensure consistent code style.

## Prerequisites

1. **Install clang-format**:
   ```bash
   # macOS
   brew install clang-format

   # Ubuntu/Debian
   sudo apt-get install clang-format
   ```

2. **Install pre-commit** (Python tool):
   ```bash
   pip install pre-commit
   # or
   brew install pre-commit
   ```

## Setup

1. **Install the pre-commit hooks**:
   ```bash
   cd /Users/rickdeharder/personal/woordklok/wordclock_platformio
   pre-commit install
   ```

2. **That's it!** Now every time you commit, the code will be automatically formatted.

## Manual Formatting

If you want to format files manually without committing:

```bash
# Format a single file
clang-format -i src/main.cpp

# Format all C/C++ files in the project
find src include -name "*.cpp" -o -name "*.h" | xargs clang-format -i
```

## Running Pre-commit Manually

To run pre-commit checks on all files (not just staged):

```bash
pre-commit run --all-files
```

## Configuration

- `.clang-format` - Defines the code style rules
- `.pre-commit-config.yaml` - Configures pre-commit hooks
- `.gitignore` - Excludes build artifacts and IDE files

## What Gets Formatted?

- All `.cpp` and `.h` files in `src/` and `include/`
- Excludes `.pio/` build directory
- Automatically fixes:
  - Indentation (4 spaces)
  - Brace placement
  - Spacing around operators
  - Line length (max 120 chars)
  - Trailing whitespace
  - End-of-file newlines

## Bypassing Pre-commit (Not Recommended)

If you absolutely need to skip the pre-commit hook:

```bash
git commit --no-verify
```

**Note**: Only use this in emergencies!
