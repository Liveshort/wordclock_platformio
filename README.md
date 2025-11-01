# Wordclock 2.0

A Dutch word clock with WiFi connectivity, web interface, and LED drawing board.

## Quick Start

1. Open project in VSCode
2. Install PlatformIO extension
3. Build & upload to ESP32

## Development Setup

### Prerequisites

- **VSCode** with PlatformIO extension
- **clang-format** for code formatting
- **pre-commit** for git hooks (optional but recommended)

### Installing Tools

#### macOS
```bash
brew install clang-format
pip install pre-commit
```

#### Ubuntu/Debian
```bash
sudo apt-get install clang-format
pip install pre-commit
```

#### Windows

**Option 1: Using Chocolatey (Recommended)**
```powershell
# Install Chocolatey first if you don't have it: https://chocolatey.org/install
choco install llvm  # Includes clang-format
pip install pre-commit
```

**Option 2: Using Scoop**
```powershell
# Install Scoop first if you don't have it: https://scoop.sh
scoop install llvm  # Includes clang-format
pip install pre-commit
```

**Option 3: Manual Installation**
1. Download LLVM from https://releases.llvm.org/ (includes clang-format)
2. Add LLVM bin directory to your PATH
3. Install Python from https://www.python.org/
4. Run: `pip install pre-commit`

### Setting Up Pre-commit Hooks

```bash
cd /path/to/wordclock_platformio
pre-commit install
```

This will automatically format your code before each commit.

## Code Formatting

### Automatic Formatting (Recommended)

The project uses `clang-format` for consistent C/C++ code style:

- **4 spaces** for indentation
- **120 characters** max line length
- **Attach braces** style
- Based on Google style guide

#### VSCode Integration

The `.vscode/settings.json` file is configured to:

- ✅ **Auto-format C/C++ files on save** using clang-format
- ✅ Use the `.clang-format` configuration file
- ✅ Trim trailing whitespace
- ✅ Insert final newline
- ❌ **Disable auto-format for Python, JSON, YAML, Markdown** (to prevent unwanted changes)

**What gets formatted on save:**
- `.cpp` and `.h` files only

**What doesn't get formatted on save:**
- Python, JSON, YAML, Markdown files (prevents weird formatting)

#### Git Pre-commit Hook

When you commit, the pre-commit hook will:

1. **Format all C/C++ files** (`.cpp`, `.h`) with clang-format
2. **Clean up all files:**
   - Remove trailing whitespace
   - Fix line endings (LF/Unix style)
   - Ensure final newline
3. **Validate YAML files**
4. **Block large files** (>500KB)

### Manual Formatting

Format a single file:
```bash
clang-format -i src/main.cpp
```

Format all C/C++ files:
```bash
find src include -name "*.cpp" -o -name "*.h" | xargs clang-format -i
```

Run pre-commit checks manually:
```bash
pre-commit run --all-files
```

### Bypassing Pre-commit (Emergency Only)

```bash
git commit --no-verify
```

⚠️ **Only use this in emergencies!**

## Project Structure

```
wordclock_platformio/
├── src/              # Source files (.cpp)
├── include/          # Header files (.h)
├── .pio/             # PlatformIO build files (ignored)
├── .vscode/          # VSCode settings
│   └── settings.json # Editor configuration
├── .clang-format     # Code style rules
├── .pre-commit-config.yaml  # Git hook configuration
└── platformio.ini    # PlatformIO configuration
```

## Features

- Dutch word clock display
- WiFi connectivity with web interface
- Network configuration via access point
- NTP time synchronization
- LED drawing board with color picker
- Minute dots for precise time display
- Button controls for various functions

## Configuration Files

- **`.clang-format`** - Defines code formatting rules
- **`.pre-commit-config.yaml`** - Configures git hooks
- **`.vscode/settings.json`** - VSCode editor settings
- **`.gitignore`** - Excludes build artifacts and IDE files
- **`platformio.ini`** - PlatformIO project configuration
