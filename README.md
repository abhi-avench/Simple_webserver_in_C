# Simple_webserver

A minimal HTTP/1.1 webserver written in pure C, built as a **MISRA C:2012 compliance demo project**.

The codebase contains **intentional MISRA violations** so you can practice running static analysis, fixing violations, and automating the process with CI/CD.

---

## 📁 Directory Structure

```
tiny_webserver/
├── include/                  # Public header files
│   ├── http_server.h         # Server API & types
│   ├── http_parser.h         # Parser API
│   ├── router.h              # Router API
│   └── logger.h              # Logger API
├── src/                      # Source files
│   ├── main.c                # Entry point
│   ├── http_server.c         # Core server (socket, accept loop)
│   ├── http_parser.c         # HTTP request parser & response builder
│   ├── router.c              # URI router
│   └── logger.c              # Logging utility
├── scripts/
│   └── run_analysis.py       # Python CI/CD automation script
├── reports/                  # Generated MISRA reports (gitignored)
├── tests/                    # Unit tests (placeholder)
├── docs/                     # Documentation
├── .github/
│   └── workflows/
│       └── misra_ci.yml      # GitHub Actions CI/CD pipeline
└── CMakeLists.txt
```

---

## 🔧 Build

```bash
mkdir build && cd build
cmake ..
make
./tiny_webserver
```

Then open: http://localhost:8080

Available routes:

- `GET /` — Home page
- `GET /status` — JSON status
- `GET /info` — Server info

---

## 🔍 Run MISRA Check Locally

```bash
# Install cppcheck
sudo apt install cppcheck          # Ubuntu/Debian
brew install cppcheck              # macOS

# Run via CMake target
cd build && make misra_check

# OR via Python script
python3 scripts/run_analysis.py
```

Report is saved to `reports/misra_report_<timestamp>.txt`

---

## 📋 Intentional MISRA Violations

| File            | Rule  | Description                           |
| --------------- | ----- | ------------------------------------- |
| `main.c`        | 21.5  | Use of `signal()` function            |
| `main.c`        | 21.8  | Use of `exit()` function              |
| `http_server.c` | 21.6  | Use of `sprintf` / `printf`           |
| `http_server.c` | 11.8  | Cast discards `const` qualifier       |
| `http_parser.c` | 15.5  | Multiple return points in function    |
| `http_parser.c` | 16.4  | Switch missing `default` clause       |
| `http_parser.c` | 14.3  | Condition always true (unsigned >= 0) |
| `http_parser.c` | 18.4  | Pointer arithmetic                    |
| `router.c`      | 13.5  | Right-hand of `&&` has side effects   |
| `logger.c`      | 7.4   | String literal to non-const pointer   |
| `logger.c`      | 21.10 | Use of `time.h` functions             |

---

## ⚙️ GitHub Actions CI/CD

Push to `main` or `develop` → GitHub Actions will:

1. Install `cmake` + `cppcheck`
2. Build the project
3. Run MISRA analysis
4. Upload report as a pipeline artifact
5. (Optional) Email the report — set `EMAIL_PASS` as a GitHub Secret

---

## 🎯 Learning Goals

- Understand common MISRA C:2012 violations in real C code
- Practice fixing violations and re-running analysis
- Build a CI/CD pipeline for embedded code compliance





```
cmake -S . -B build
cmake --build build
```