#!/usr/bin/env python3
"""
run_analysis.py
───────────────
Automates the MISRA compliance check pipeline:
  1. Pull latest code (if inside a git repo)
  2. CMake configure + build
  3. Run cppcheck MISRA analysis
  4. Save report to reports/
  5. Send report via email

Usage:
  python3 scripts/run_analysis.py [--email]

Requirements:
  apt install cppcheck cmake
"""

import subprocess
import sys
import os
import datetime
import smtplib
from email.mime.text import MIMEText
from email.mime.multipart import MIMEMultipart

# ─── Configuration ────────────────────────────────────────────────────────────

PROJECT_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
BUILD_DIR    = os.path.join(PROJECT_ROOT, "build")
REPORTS_DIR  = os.path.join(PROJECT_ROOT, "reports")
SRC_DIR      = os.path.join(PROJECT_ROOT, "src")
INCLUDE_DIR  = os.path.join(PROJECT_ROOT, "include")

EMAIL_SENDER   = "you@gmail.com"
EMAIL_RECEIVER = "team@example.com"
EMAIL_SUBJECT  = "MISRA Report — tiny_webserver"


# ─── Helpers ──────────────────────────────────────────────────────────────────

def run(cmd: list, cwd: str = PROJECT_ROOT) -> tuple:
    """Run a shell command, return (returncode, stdout, stderr)."""
    print(f"  ▶ {' '.join(cmd)}")
    result = subprocess.run(cmd, cwd=cwd, capture_output=True, text=True)
    return result.returncode, result.stdout, result.stderr


def banner(title: str) -> None:
    print(f"\n{'='*55}")
    print(f"  {title}")
    print(f"{'='*55}")


# ─── Steps ────────────────────────────────────────────────────────────────────

def step_git_pull() -> bool:
    banner("STEP 1 — Git Pull")
    rc, out, err = run(["git", "pull", "--ff-only"])
    print(out or err)
    if rc != 0:
        print("  [WARN] git pull failed (not a git repo or no remote) — continuing")
    return True  # non-fatal


def step_cmake_configure() -> bool:
    banner("STEP 2 — CMake Configure")
    os.makedirs(BUILD_DIR, exist_ok=True)
    rc, out, err = run(["cmake", "..", "-DCMAKE_BUILD_TYPE=Debug"], cwd=BUILD_DIR)
    print(out)
    if rc != 0:
        print(f"  [ERROR] CMake configure failed:\n{err}")
        return False
    return True


def step_cmake_build() -> bool:
    banner("STEP 3 — CMake Build")
    cpu_count = os.cpu_count() or 1
    rc, out, err = run(["cmake", "--build", ".", f"-j{cpu_count}"], cwd=BUILD_DIR)
    print(out)
    if rc != 0:
        print(f"  [ERROR] Build failed:\n{err}")
        return False
    return True


def step_cppcheck_misra() -> tuple:
    banner("STEP 4 — MISRA Analysis (cppcheck)")

    os.makedirs(REPORTS_DIR, exist_ok=True)
    timestamp   = datetime.datetime.now().strftime("%Y%m%d_%H%M%S")
    report_file = os.path.join(REPORTS_DIR, f"misra_report_{timestamp}.txt")

    cmd = [
        "cppcheck",
        "--enable=all",
        "--addon=misra",
        "--std=c99",
        "--suppress=missingIncludeSystem",
        f"-I{INCLUDE_DIR}",
        "--output-file=" + report_file,
        SRC_DIR,
    ]

    rc, out, err = run(cmd)

    # cppcheck writes violations to stderr by default
    report_content = out + err

    # Also append to the file (output-file captures cppcheck's own output)
    with open(report_file, "a", encoding="utf-8") as f:
        f.write(report_content)

    print(f"\n  Report saved → {report_file}")

    # Count violations
    violation_count = report_content.count("misra-c2012")
    print(f"  MISRA violations found: {violation_count}")

    return report_file, report_content, violation_count


def step_send_email(report_content: str, violation_count: int) -> None:
    banner("STEP 5 — Send Email Report")

    email_pass = os.environ.get("EMAIL_PASS")
    if not email_pass:
        print("  [SKIP] EMAIL_PASS env var not set — skipping email")
        return

    subject = f"{EMAIL_SUBJECT} — {violation_count} violation(s) found"

    body = f"""MISRA Compliance Report
========================
Project  : tiny_webserver
Date     : {datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')}
Violations: {violation_count}

─── Full Report ───────────────────────────────────
{report_content[:4000]}
{'... (truncated)' if len(report_content) > 4000 else ''}
"""

    msg = MIMEMultipart()
    msg["From"]    = EMAIL_SENDER
    msg["To"]      = EMAIL_RECEIVER
    msg["Subject"] = subject
    msg.attach(MIMEText(body, "plain"))

    try:
        with smtplib.SMTP_SSL("smtp.gmail.com", 465) as server:
            server.login(EMAIL_SENDER, email_pass)
            server.send_message(msg)
        print(f"  Email sent to {EMAIL_RECEIVER}")
    except Exception as exc:
        print(f"  [ERROR] Failed to send email: {exc}")


# ─── Main ─────────────────────────────────────────────────────────────────────

def main() -> int:
    send_email_flag = "--email" in sys.argv

    banner("MISRA CI/CD Pipeline — tiny_webserver")
    print(f"  Project root: {PROJECT_ROOT}")

    step_git_pull()

    if not step_cmake_configure():
        return 1

    if not step_cmake_build():
        return 1

    report_file, report_content, violation_count = step_cppcheck_misra()

    if send_email_flag:
        step_send_email(report_content, violation_count)

    banner("DONE")
    print(f"  Report  : {report_file}")
    print(f"  Violations: {violation_count}")
    print()

    return 0 if violation_count == 0 else 2  # exit 2 = violations found


if __name__ == "__main__":
    sys.exit(main())
