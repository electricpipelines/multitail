# Multitail

A Windows console application for monitoring multiple log files simultaneously in real-time.

![License](https://img.shields.io/badge/license-MIT-blue.svg)
![Platform](https://img.shields.io/badge/platform-Windows-lightgrey.svg)


## Overview

Multitail allows you to watch multiple log files at once, displaying them in split panes within a single terminal window. Similar to the Unix `tail -f` command, but with support for viewing up to 8 files in parallel.

## Features

- Monitor up to 8 files simultaneously in split panes
- Real-time file following with auto-scroll
- Scroll through history (up to 100,000 lines per file)
- Works with files being actively written to
- Lightweight single executable with no dependencies

## Installation

### Download

Download the latest release from the [Releases](../../releases) page.

### Build from Source

Requirements:
- CMake 3.16+
- MSVC (Visual Studio 2019 or later)

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

The executable will be at `build/Release/multitail.exe`.

## Usage

```bash
multitail.exe file1.log file2.log [file3.log ...]
```

### Keyboard Controls

| Key | Action |
|-----|--------|
| Tab | Switch to next pane |
| Shift+Tab | Switch to previous pane |
| Up/Down | Scroll one line |
| Page Up/Page Down | Scroll one page |
| Home | Jump to beginning of file |
| End | Resume live following |
| Q / Ctrl+C | Exit |

## Examples

Monitor two log files:
```bash
multitail.exe app.log error.log
```

Monitor multiple service logs:
```bash
multitail.exe C:\logs\service1.log C:\logs\service2.log C:\logs\service3.log
```

## License

MIT License - see [LICENSE](LICENSE) for details.

## Contributing

Contributions are welcome! Please feel free to submit issues and pull requests.
