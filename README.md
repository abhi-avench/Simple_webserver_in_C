
---

# A Industry-Grade Static Analysis Pipeline for Embedded Systems - MISRA C:2012 Static Analysis Demo

<div align="center">

**A minimal HTTP/1.1 webserver in C | Embedded Safety-Critical Code Compliance Demo**

[![MISRA Check](https://github.com/abhi-avench/Simple_webserver_in_C/actions/workflows/misra_ci.yml/badge.svg)](https://github.com/abhi-avench/Simple_webserver_in_C/actions/workflows/misra_ci.yml)
[![cppcheck](https://img.shields.io/badge/static%20analysis-cppcheck-blue)](https://cppcheck.sourceforge.io/)
[![MISRA](https://img.shields.io/badge/standard-MISRA%20C%3A2012-red)](https://www.misra.org.uk/)

</div>

---

## What is This Project?

This is an **educational demonstration** of **static code analysis** using **cppcheck** for **MISRA C:2012 compliance** in embedded/safety-critical systems.

The project contains a functional HTTP web server written in C with **intentional MISRA violations** - perfect for:

- Learning how static analyzers detect violations
- Practicing MISRA rule enforcement
- Building CI/CD pipelines for compliance checking
- Understanding embedded software quality gates

---

## Why Static Analysis for Embedded Industry?

```
┌─────────────────────────────────────────────────────────────┐
│  EMBEDDED / SAFETY-CRITICAL SYSTEMS REQUIREMENTS           │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  ✓ Zero undefined behavior                                 │
│  ✓ No memory leaks                                          │
│  ✓ Deterministic execution                                 │
│  ✓ Compiler-independent code                               │
│  ✓ MISRA C:2012 compliance                                 │
│                                                             │
│  🔍 STATIC ANALYSIS (cppcheck) FINDS:                      │
│  • Dead code & unreachable paths                           │
│  • Buffer overflows                                         │
│  • Null pointer dereferences                                │
│  • MISRA rule violations                                    │
│  • Implementation-defined behavior                          │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

---

## 📦 Project Structure

```
simple_webserver/
│
├── 📁 src/                    # Source code for analysis
│   ├── main.c                 # Entry point (has MISRA violations)
│   ├── http_server.c          # Socket server (has violations)
│   ├── http_parser.c          # HTTP parser (has violations)
│   ├── router.c               # URL router (has violations)
│   └── logger.c               # Logger (has violations)
│
├── 📁 include/                # Header files
│
├── 📁 reports/                # cppcheck analysis reports
│   ├── misra_report.xml       # XML format for CI/CD
│   └── misra_report.html      # Human-readable HTML
│
├── 📁 .github/workflows/      # CI/CD automation
│   └── misra_ci.yml           # GitHub Actions pipeline
│
├── 📄 CMakeLists.txt          # Build configuration
└── 📄 cppcheck_output.xml     # Static analysis results
```

---

## 🔧 Quick Start

### Prerequisites

```bash
# Install cppcheck (static analysis tool)
sudo apt install cppcheck    # Ubuntu/Debian
brew install cppcheck         # macOS
winget install cppcheck       # Windows

# Install build tools
sudo apt install cmake gcc    # Ubuntu/Debian
```

### Build & Run

```bash
# Configure and build
cmake -S . -B build
cmake --build build

# Run the webserver
./build/tiny_webserver

# Test in browser: http://localhost:8080
```

---

## Static Analysis with cppcheck

### Run MISRA Analysis Locally

```bash
# Basic MISRA check
cppcheck --enable=all --addon=misra src/

# Generate XML report (for CI/CD)
cppcheck --enable=all --addon=misra \
         --xml --xml-version=2 \
         src/ 2> misra_report.xml

# Generate HTML report
cppcheck --enable=all --addon=misra \
         --output-file=misra_report.txt \
         src/

# Run via CMake (if configured)
cd build && make misra_check
```

### cppcheck Command Line Options Explained

| Option                            | Purpose                                                   |
| --------------------------------- | --------------------------------------------------------- |
| `--enable=all`                    | Enable all checks (style, performance, portability, etc.) |
| `--addon=misra`                   | Load MISRA C:2012 ruleset                                 |
| `--std=c99`                       | Set C standard (important for MISRA)                      |
| `--suppress=missingIncludeSystem` | Ignore system header warnings                             |
| `--xml --xml-version=2`           | Machine-readable output for CI/CD                         |
| `--output-file=<file>`            | Save report to file                                       |
| `--inconclusive`                  | Show uncertain/warnings                                   |
| `-I <dir>`                        | Add include directory                                     |

---

## MISRA C:2012 Violations (Intentional)

| Rule      | Severity | File            | Description              | Embedded Impact                         |
| --------- | -------- | --------------- | ------------------------ | --------------------------------------- |
| **21.5**  | Warning  | `main.c`        | Use of `signal()`        | Undefined behavior in real-time systems |
| **21.8**  | Warning  | `main.c`        | Use of `exit()`          | No graceful shutdown in safety systems  |
| **21.6**  | Style    | `http_server.c` | `printf` usage           | No heap/stdio in bare-metal             |
| **11.8**  | Style    | `http_server.c` | `const` cast discarding  | Memory corruption risk                  |
| **15.5**  | Style    | `http_parser.c` | Multiple return points   | Poor testability, complex control flow  |
| **16.4**  | Warning  | `http_parser.c` | Switch missing `default` | Unhandled cases cause bugs              |
| **14.3**  | Style    | `http_parser.c` | Always-true condition    | Dead code, confusion                    |
| **18.4**  | Warning  | `http_parser.c` | Pointer arithmetic       | Platform-dependent behavior             |
| **13.5**  | Warning  | `router.c`      | Side effects in `&&`     | Order-of-evaluation issues              |
| **7.4**   | Style    | `logger.c`      | Non-const string literal | Memory corruption                       |
| **21.10** | Warning  | `logger.c`      | `<time.h>` usage         | Non-deterministic in embedded           |

### Severity Levels Explained

```
┌─────────────────────────────────────────────────────────────┐
│  ERROR    → Must fix (crash, undefined behavior)           │
│  WARNING  → Should fix (potential bug, MISRA violation)    │
│  STYLE    → Consider fixing (code clarity, maintainability)│
│  PORTABILITY → Fix for cross-platform compatibility        │
│  PERFORMANCE → Optimization opportunity                    │
└─────────────────────────────────────────────────────────────┘
```

---

## CI/CD Pipeline (GitHub Actions)

### What the Pipeline Does

```yaml
Workflow: MISRA Compliance CI
├── Trigger: push/pull_request to main/develop
│
├── Job 1: Build + Static Analysis
│   ├── 📥 Checkout code
│   ├── 📦 Install cppcheck, cmake, gcc
│   ├── 🔨 Build project
│   ├── 🔍 Run cppcheck MISRA analysis
│   ├── 📄 Generate HTML report
│   └── 📤 Upload report as artifact
│
└── Job 2: Email Notification (optional)
    ├── 📥 Download report
    ├── 📊 Parse violations
    └── 📧 Send email via EmailJS
```

### View Reports in GitHub Actions

1. Go to **Actions** tab
2. Click on workflow run
3. Scroll to **Artifacts** section
4. Download `misra-report-*.zip`
5. Extract and open `misra_report.html`

---

## Email Notifications via EmailJS

When violations are found, an automated email is sent with:

- Total violation count
- Severity breakdown (Error/Warning/Style)
- Top violations table
- Link to download full report
- Link to GitHub Actions run

### Setup Email Notifications

Add these secrets in **GitHub Settings → Secrets → Actions**:

| Secret                | Description                            |
| --------------------- | -------------------------------------- |
| `EMAILJS_SERVICE_ID`  | Your EmailJS service ID                |
| `EMAILJS_TEMPLATE_ID` | Your email template ID                 |
| `EMAILJS_USER_ID`     | Your EmailJS public key                |
| `EMAILJS_PRIVATE_KEY` | Your EmailJS private key (strict mode) |
| `EMAILJS_RECIPIENT`   | Email address to receive reports       |

---

## cppcheck Integration Examples

### Example 1: Basic MISRA Check

```bash
$ cppcheck --enable=all --addon=misra src/main.c
$ rm -rf build && mkdir build && cd build && cmake .. && make

Checking src/main.c ...
src/main.c:12: warning: Misra rule 21.5 - The function 'signal' shall not be used
src/main.c:47: warning: Misra rule 21.8 - The function 'exit' shall not be used
```

### Example 2: Suppressing Warnings

```bash
# Suppress specific rule for a line
cppcheck --suppress=misra:21.5 src/main.c

# Suppress via inline comment in code
// cppcheck-suppress misra-c2012-21.5
(void)signal(SIGINT, sig_handler);
```

### Example 3: Custom MISRA Configuration

Create `misra_addon.json`:

```json
{
  "script": "misra.py",
  "args": [
    "--rule-texts=misra_rules.txt",
    "--rule-file=",
    "--certainty=1,2,3"
  ]
}
```

Run with custom config:

```bash
cppcheck --addon=misra_addon.json src/
```

---

## Static Analysis in CI/CD Pipeline

### Adding cppcheck to Your Workflow

```yaml
- name: Run MISRA Analysis
  run: |
    cppcheck --enable=all \
             --addon=misra \
             --xml --xml-version=2 \
             src/ 2> misra_report.xml

    # Fail build if violations found (optional)
    if grep -q "<error" misra_report.xml; then
      echo "MISRA violations found!"
      exit 1
    fi
```

### Quality Gates

```bash
# Count violations
VIOLATIONS=$(grep -c "<error" misra_report.xml)

# Set threshold (e.g., fail if > 0 violations)
if [ $VIOLATIONS -gt 0 ]; then
    echo "Quality gate failed: $VIOLATIONS violations"
    exit 1
fi
```

---

## Learning Outcomes

After completing this project, you will understand:

```
┌─────────────────────────────────────────────────────────────┐
│  STATIC ANALYSIS CONCEPTS                                   │
│  ✓ How to run cppcheck with MISRA rules                    │
│  ✓ Understanding violation severity levels                  │
│  ✓ Suppressing false positives                             │
│  ✓ Integrating static analysis into CI/CD                  │
│                                                             │
│  MISRA C:2012 RULES                                         │
│  ✓ Which rules are critical for embedded                   │
│  ✓ Why these rules exist in safety standards               │
│  ✓ How to fix common violations                            │
│                                                             │
│  CI/CD FOR EMBEDDED                                         │
│  ✓ Automated compliance checking                           │
│  ✓ Report generation in multiple formats                   │
│  ✓ Email notifications for violations                      │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

---

## Tools Used

| Tool               | Purpose                         | Industry Standard       |
| ------------------ | ------------------------------- | ----------------------- |
| **cppcheck**       | Static analysis, MISRA checking | ✅ ISO 26262, IEC 61508 |
| **CMake**          | Build system                    | Cross-platform builds   |
| **GitHub Actions** | CI/CD automation                | DevOps compliance       |
| **EmailJS**        | Notifications                   | Automated reporting     |
| **GCC**            | Compiler                        | Embedded target         |

---

## 📚 Resources

- [cppcheck Manual](https://cppcheck.sourceforge.io/manual.pdf)
- [MISRA C:2012 Guidelines](https://www.misra.org.uk/)
- [cppcheck MISRA Addon](https://sourceforge.net/projects/cppcheck/files/Articles/MISRA.pdf)
- [GitHub Actions Documentation](https://docs.github.com/en/actions)

---

## 🔄 Common cppcheck Commands Reference

```bash
# Basic usage
cppcheck src/

# Enable all checks
cppcheck --enable=all src/

# MISRA compliance
cppcheck --addon=misra src/

# Save report
cppcheck src/ 2> report.txt

# HTML report (using cppcheck-htmlreport)
cppcheck --xml --xml-version=2 src/ 2> report.xml
cppcheck-htmlreport --file=report.xml --report-dir=html_report

# Check specific files
cppcheck src/main.c src/http_server.c

# Exclude directories
cppcheck -isrc/test src/

# Show progress
cppcheck --progress src/

# Platform specific (embedded)
cppcheck --platform=arm32-wince src/
```

---

## License

This project is for **educational purposes** - demonstrating static analysis techniques for embedded safety-critical software development.

---

##  Contributing

Found a new MISRA violation? Want to add more static analysis checks? Feel free to:

1. Fork the repository
2. Add your violations or fixes
3. Run cppcheck to verify
4. Submit a pull request

---

<div align="center">

**Built for learning embedded static analysis | MISRA C:2012 | cppcheck | GitHub Actions**

[⬆ Back to Top](#-simple_webserver---misra-c2012-static-analysis-demo)

</div>

---
