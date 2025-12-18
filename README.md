# Secret Detector

![Secret Detector logo](./assets/secret-detector.svg)

[🇷🇺 Читать на русском](README.ru.md)

Secret Detector is a fast C++17 tool for finding secrets in source code: API keys, tokens, passwords, database URLs, private keys and other sensitive data.  
It includes both a command-line scanner and a Qt-based GUI.

---

## Features

- Detects common secret types:
  - AWS access keys  
  - GitHub tokens (classic and fine-grained)  
  - Private keys (RSA, DSA, EC, OpenSSH)  
  - Passwords in plain text assignments  
  - API keys and database connection URLs  
  - JWT tokens
- Recursive scanning of directories with respect to `.gitignore`
- Flexible include/exclude filters (extensions and path patterns)
- Severity levels: `CRITICAL`, `HIGH`, `MEDIUM`, `LOW`
- Configurable detection patterns via `config/patterns.json`
- GUI:
  - Interactive table of findings with sorting and severity coloring
  - Double-click to open file at the exact line in your editor
  - Configurable preferred editor (VS Code, Vim, etc.)
  - Export reports to Text / JSON / CSV / HTML
- CLI:
  - Machine-friendly output (JSON/CSV/HTML) for CI/CD integration
  - Non-zero exit codes suitable for pipelines and pre-commit hooks

---

## Requirements

### Build Dependencies

- C++17 compatible compiler (GCC 11+, Clang 12+)
- CMake 3.16+
- Libraries:
  - `nlohmann-json3-dev`
  - `libpcre2-dev`
  - `libspdlog-dev`
- For GUI:
  - `qtbase5-dev`
  - `qttools5-dev`
  - Qt5 runtime libraries (`libqt5widgets5`, `libqt5gui5`, etc.)

Example (Ubuntu):

```bash
sudo apt update
sudo apt install -y build-essential cmake git nlohmann-json3-dev libpcre2-dev libspdlog-dev qtbase5-dev qttools5-dev
```

---

## Building from Source

Clone the repository and build in a separate `build` directory:

```bash
git clone https://github.com/CHIKENlvl19/SecretDetector.git
cd SecretDetector
mkdir -p build
cd build
# Build CLI + GUI (default)
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j"$(nproc)"
```

Resulting binaries (paths may vary depending on CMake setup):

- `build/secret_detector` – CLI scanner  
- `build/secret_detector_gui` – GUI application  

### Build CLI Only (no Qt dependency)

```bash
mkdir -p build-cli
cd build-cli
cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_GUI=OFF ..
make -j"$(nproc)" secret_detector
```

---

## Configuration: Detection Patterns

All detection rules are defined in `config/patterns.json`.

Example (simplified):

```json
{
  "version": "1.0.0",
  "description": "Secret Detector Patterns",
  "patterns": {
    "aws_key": {
      "regex": "\b(AKIA[0-9A-Z]{16})\b",
      "severity": "CRITICAL",
      "description": "AWS Access Key ID"
    },
    "github_token": {
      "regex": "\b(ghp_[0-9a-zA-Z]{36})\b",
      "severity": "CRITICAL",
      "description": "GitHub Personal Access Token"
    },
    "private_key": {
      "regex": "-----BEGIN (RSA |DSA |EC |OPENSSH )?PRIVATE KEY-----",
      "severity": "CRITICAL",
      "description": "Private key block"
    },
    "password_plain": {
      "regex": "\b(password|passwd)\s*[=:]\s*['"'][^'"']+['"']",
      "severity": "HIGH",
      "description": "Password in plain text assignment"
    },
    "api_key": {
      "regex": "(api[_-]?key|apikey)\s*[=:]\s*['"][^'"]{16,}['"]",
      "severity": "HIGH",
      "description": "API key assignment"
    },
    "database_url": {
      "regex": "(postgres|mysql|mongodb)://[^\s'"]+:[^\s'"]+@[^\s'"]+",
      "severity": "HIGH",
      "description": "Database connection URL"
    }
  }
}
```

You can extend this file with additional regex patterns for your environment.

---

## Running the CLI

From the build directory:

```bash
./secret_detector <path> [options]
```

### Scan current directory recursively

```bash
./secret_detector . --recursive
```

### Respect .gitignore and export JSON report

```bash
./secret_detector . --recursive --respect-gitignore --output report.json --format json
```

### Scan a specific project directory

```bash
./secret_detector /path/to/project --recursive --include-ext cpp,h,py --exclude build,.git,node_modules
```

Common options (actual list may differ slightly):

- `--recursive` – scan directories recursively  
- `--respect-gitignore` – skip files ignored by `.gitignore`  
- `--exclude <patterns>` – comma-separated path patterns to skip  
- `--include-ext <exts>` – comma-separated list of file extensions to include  
- `--output <file>` – path to save report  
- `--format <text|json|csv|html>` – report format  
- `--strict` – non-zero exit code if *any* secret is found  
- `--threads <N>` – number of worker threads (0 = auto)

Exit codes (intended for CI):

- `0` – no secrets found, scan completed successfully  
- `1` – secrets found  
- `2+` – runtime or configuration error  

---

## Using the GUI

After building with GUI support:

```bash
cd build
./secret_detector_gui
```
Or just open Secret Detector directly from your app menu.

### Main workflow

1. **Scan Target**  
   - Choose a folder to scan.  
   - Enable "Recursive scan" and "Respect .gitignore" if needed.  

2. **Scan Options**  
   - `Exclude`: path patterns to ignore (comma-separated).  
   - `Include ext`: file extensions to include (`cpp, h, py, go, js, ts, ...`).  

3. **Output Options**  
   - `Show`: filter results by severity (All / CRITICAL / HIGH / …).  
   - `Output`: optional directory to save exported reports.  
   - `Format`: Text / JSON / CSV / HTML.  

4. **Actions**  
   - `Start Scan` – run the scan in a background thread.  
   - `Stop` – abort current scan.  
   - `Clear Results` – clear the table and log.  
   - `Export Report` – save last scan results in the chosen format.  

5. **Inspecting Results**  
   - Results table shows: File, Line, Severity, Pattern, Match, Preview.  
   - Columns are sortable; severity is color-coded.  
   - **Double-click a row** to open the corresponding file at the exact line in your editor.

### Editor Integration

On first double-click, the GUI will show a dialog to select a code editor:

- Supports: VS Code (`code`), Gedit, Kate, Sublime (`subl`), Vim/Neovim, Nano, Emacs, etc.
- You can browse to a custom editor executable.
- Option "Remember my choice" stores the selection in application settings.
- If the chosen editor is later removed, the GUI will prompt for a new one.

You can always change the default editor via menu:  
**Settings → Change Default Editor…**

---

## CI/CD Integration (GitHub Actions)

```yaml
name: Secret Scan

on: [push, pull_request]

jobs:
  scan:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4

      - name: Configure build
        run: |
          mkdir -p build
          cd build
          cmake -DCMAKE_BUILD_TYPE=Release ..

      - name: Build CLI
        run: |
          cd build
          make -j"$(nproc)" secret_detector

      - name: Run Secret Detector
        run: |
          cd build
          ./secret_detector .. --recursive --respect-gitignore --strict --format json --output secret_report.json
```

---

## Limitations and Notes

- The tool focuses on **pattern-based detection** using regexes defined in `patterns.json`.  
- High-entropy detection is implemented in the core but disabled in the default configuration due to a high rate of false positives in typical code.  
- No network access is performed; all analysis is done locally on your files.

---

## Roadmap / Ideas

- Optional high-entropy detector with adjustable thresholds  
- Language-aware heuristics for fewer false positives  
- Built-in integration with pre-commit hooks  
- More predefined patterns (cloud providers, CI tokens, OAuth secrets, etc.)

---

## Contributing

Contributions, issues and feature requests are welcome.

- Report bugs and suggest features via the issue tracker  
- Fork the repo, create a feature branch, and open a Pull Request  

Please keep PRs focused and include a brief description and test steps.

---

## License

This project is licensed under the MIT License.  
See the `LICENSE` file for details.
