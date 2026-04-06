#!/usr/bin/env python3
"""
MISRA Report Generator - Converts cppcheck output to beautiful HTML
All files are read from and written to the build directory
"""

import xml.etree.ElementTree as ET
import html
import re
import os
import json
from datetime import datetime
from pathlib import Path

SCRIPT_DIR = Path(__file__).resolve().parent
PROJECT_ROOT = SCRIPT_DIR.parent
BUILD_DIR = PROJECT_ROOT / 'build'


def parse_cppcheck_output(xml_file):
    """Parse cppcheck XML output and extract violations"""
    violations = []
    xml_path = Path(xml_file)

    if not xml_path.exists():
        xml_path = BUILD_DIR / xml_file
        if not xml_path.exists():
            return violations

    try:
        tree = ET.parse(xml_path)
        root = tree.getroot()

        for error in root.findall('.//error'):
            location = error.find('location')
            if location is not None:
                file_path = location.get('file', 'Unknown')
                line_num = location.get('line', '?')
                msg = error.get('msg', 'No message')
                severity = error.get('severity', 'style')
                
                # Extract MISRA rule number
                rule = 'N/A'
                misra_match = re.search(r'misra c[:\s]+(\d+\.\d+)', msg.lower())
                if misra_match:
                    rule = misra_match.group(1)
                
                violations.append({
                    'rule': rule,
                    'file': os.path.basename(file_path),
                    'full_path': file_path,
                    'line': line_num,
                    'severity': severity,
                    'message': html.escape(msg[:200])
                })
    except Exception as e:
        print(f"Error parsing cppcheck output: {e}")
    
    return violations

def generate_html_report(violations, repo_info, output_file):
    """Generate beautiful HTML report"""
    
    # Ensure output directory exists
    output_dir = os.path.dirname(output_file)
    if output_dir and not os.path.exists(output_dir):
        os.makedirs(output_dir)
    
    # Count violations by severity
    error_count = sum(1 for v in violations if v['severity'] == 'error')
    warning_count = sum(1 for v in violations if v['severity'] == 'warning')
    style_count = sum(1 for v in violations if v['severity'] == 'style')
    total_count = len(violations)
    
    # Build violation table rows
    table_rows = ""
    for v in violations[:50]:  # Limit to 50 for email
        severity_class = f"severity-{v['severity']}"
        table_rows += f"""
            <tr class="violation-{v['severity']}">
                <td><code>{v['rule']}</code></td>
                <td>{v['file']}</td>
                <td>{v['line']}</td>
                <td class="{severity_class}">{v['severity'].upper()}</td>
                <td>{v['message']}</td>
             </tr>
        """
    
    if not table_rows:
        table_rows = '<tr><td colspan="5" style="text-align: center;">✅ No MISRA violations found!</td></tr>'
    
    # Determine status
    status = "PASS - No Violations" if total_count == 0 else f"FAIL - {total_count} Violations Found"
    status_class = "status-pass" if total_count == 0 else "status-fail"
    
    # HTML template
    html_content = f"""<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>MISRA C:2012 Compliance Report - {repo_info['project_name']}</title>
    <style>
        * {{ margin: 0; padding: 0; box-sizing: border-box; }}
        body {{
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, 'Helvetica Neue', Arial, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
            padding: 20px;
        }}
        .container {{
            max-width: 1400px;
            margin: 0 auto;
            background: white;
            border-radius: 16px;
            box-shadow: 0 20px 60px rgba(0,0,0,0.3);
            overflow: hidden;
        }}
        .header {{
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            padding: 40px;
            text-align: center;
        }}
        .header h1 {{ font-size: 2.5em; margin-bottom: 10px; }}
        .header p {{ opacity: 0.9; font-size: 1.1em; }}
        
        .summary {{
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
            gap: 20px;
            padding: 30px;
            background: #f8f9fa;
        }}
        .summary-card {{
            background: white;
            padding: 20px;
            border-radius: 12px;
            text-align: center;
            box-shadow: 0 2px 8px rgba(0,0,0,0.1);
            transition: transform 0.2s;
        }}
        .summary-card:hover {{ transform: translateY(-5px); }}
        .summary-card h3 {{ color: #666; font-size: 0.9em; margin-bottom: 10px; text-transform: uppercase; }}
        .summary-card .number {{ font-size: 2.5em; font-weight: bold; color: #667eea; }}
        
        .status-pass {{ background-color: #4CAF50; color: white; padding: 8px 20px; border-radius: 20px; display: inline-block; }}
        .status-fail {{ background-color: #f44336; color: white; padding: 8px 20px; border-radius: 20px; display: inline-block; }}
        
        .content {{ padding: 30px; }}
        
        .info-section {{
            background: #e7f3fe;
            border-left: 4px solid #2196F3;
            padding: 20px;
            margin-bottom: 30px;
            border-radius: 8px;
        }}
        
        table {{
            width: 100%;
            border-collapse: collapse;
            margin: 20px 0;
            background: white;
            box-shadow: 0 1px 3px rgba(0,0,0,0.1);
        }}
        th {{
            background: #667eea;
            color: white;
            padding: 12px;
            text-align: left;
            font-weight: 600;
            position: sticky;
            top: 0;
        }}
        td {{
            padding: 12px;
            border-bottom: 1px solid #e0e0e0;
        }}
        tr:hover {{ background-color: #f5f5f5; }}
        
        .violation-error {{ border-left: 4px solid #f44336; }}
        .violation-warning {{ border-left: 4px solid #ff9800; }}
        .violation-style {{ border-left: 4px solid #2196f3; }}
        
        .severity-error {{ color: #f44336; font-weight: bold; }}
        .severity-warning {{ color: #ff9800; font-weight: bold; }}
        .severity-style {{ color: #2196f3; }}
        
        code {{
            background: #f4f4f4;
            padding: 2px 6px;
            border-radius: 4px;
            font-family: 'Courier New', monospace;
            font-size: 0.9em;
        }}
        
        .footer {{
            background: #f8f9fa;
            padding: 20px;
            text-align: center;
            color: #666;
            font-size: 0.85em;
            border-top: 1px solid #e0e0e0;
        }}
        
        @media (max-width: 768px) {{
            .summary {{ grid-template-columns: 1fr; }}
            table {{ font-size: 12px; }}
            th, td {{ padding: 8px; }}
            .header h1 {{ font-size: 1.5em; }}
        }}
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>🔍 MISRA C:2012 Compliance Report</h1>
            <p>{repo_info['project_name']}</p>
            <div style="margin-top: 20px;">
                <span class="{status_class}">{status}</span>
            </div>
        </div>
        
        <div class="summary">
            <div class="summary-card">
                <h3>Total Violations</h3>
                <div class="number">{total_count}</div>
            </div>
            <div class="summary-card">
                <h3>Error Severity</h3>
                <div class="number" style="color: #f44336;">{error_count}</div>
            </div>
            <div class="summary-card">
                <h3>Warning Severity</h3>
                <div class="number" style="color: #ff9800;">{warning_count}</div>
            </div>
            <div class="summary-card">
                <h3>Style Issues</h3>
                <div class="number" style="color: #2196f3;">{style_count}</div>
            </div>
        </div>
        
        <div class="content">
            <div class="info-section">
                <h3>📋 Build Information</h3>
                <p><strong>Repository:</strong> {repo_info['repo_full_name']}</p>
                <p><strong>Commit SHA:</strong> <code>{repo_info['commit_sha']}</code></p>
                <p><strong>Branch:</strong> {repo_info['branch']}</p>
                <p><strong>Run Number:</strong> #{repo_info['run_number']}</p>
                <p><strong>Triggered by:</strong> {repo_info['triggered_by']}</p>
                <p><strong>Generated:</strong> {datetime.now().strftime('%Y-%m-%d %H:%M:%S UTC')}</p>
            </div>
            
            <h2>📋 Violation Details</h2>
            <div style="overflow-x: auto;">
                <table>
                    <thead>
                        <tr>
                            <th>MISRA Rule</th>
                            <th>File</th>
                            <th>Line</th>
                            <th>Severity</th>
                            <th>Message</th>
                        </tr>
                    </thead>
                    <tbody>
                        {table_rows}
                    </tbody>
                </table>
            </div>
            
            <div style="margin: 30px 0; padding: 20px; background: #fff3cd; border-left: 4px solid #ffc107; border-radius: 8px;">
                <h3>⚠️ Intentional Violations (For Learning)</h3>
                <p>This project contains intentional MISRA violations for educational purposes:</p>
                <ul>
                    <li><strong>Rule 21.5, 21.8</strong> - main.c (signal handlers, exit())</li>
                    <li><strong>Rule 21.6</strong> - http_server.c (sprintf/printf usage)</li>
                    <li><strong>Rule 15.5, 16.4, 18.4</strong> - http_parser.c (multiple returns, missing default, pointer arithmetic)</li>
                    <li><strong>Rule 13.5</strong> - router.c (side effects in logical operators)</li>
                    <li><strong>Rule 21.10</strong> - logger.c (time.h functions)</li>
                </ul>
            </div>
        </div>
        
        <div class="footer">
            <p>Generated by GitHub Actions CI/CD Pipeline</p>
            <p>MISRA C:2012 compliance check using cppcheck</p>
            <p><a href="{repo_info['workflow_link']}" style="color: #667eea;">View workflow run on GitHub</a></p>
        </div>
    </div>
</body>
</html>
"""
    
    with open(output_file, 'w', encoding='utf-8') as f:
        f.write(html_content)
    
    print(f"✅ Report written to: {output_file}")
    return total_count

if __name__ == '__main__':
    try:
        xml_file_name = 'cppcheck_output.xml'
        violations = parse_cppcheck_output(xml_file_name)

        if not violations and not (PROJECT_ROOT / xml_file_name).exists() and not (BUILD_DIR / xml_file_name).exists():
            print(f"⚠️ Warning: Could not find cppcheck output at {PROJECT_ROOT / xml_file_name} or {BUILD_DIR / xml_file_name}")

        total = len(violations)
        
        # Get GitHub context from environment variables
        repo_info = {
            'project_name': os.environ.get('PROJECT_NAME', 'Simple Webserver'),
            'repo_full_name': os.environ.get('GITHUB_REPOSITORY', 'unknown/repo'),
            'commit_sha': os.environ.get('GITHUB_SHA', 'unknown')[:7],
            'branch': os.environ.get('GITHUB_REF_NAME', 'unknown'),
            'run_number': os.environ.get('GITHUB_RUN_NUMBER', '0'),
            'triggered_by': os.environ.get('GITHUB_ACTOR', 'unknown'),
            'workflow_link': f"https://github.com/{os.environ.get('GITHUB_REPOSITORY', '')}/actions/runs/{os.environ.get('GITHUB_RUN_ID', '')}"
        }
        
        # Generate report in build directory
        output_file = BUILD_DIR / 'misra_report.html'
        total = generate_html_report(violations, repo_info, str(output_file))
        
        # Save summary in build directory
        summary_file = BUILD_DIR / 'report_summary.json'
        with open(summary_file, 'w') as f:
            json.dump({
                'total_violations': total,
                'status': 'pass' if total == 0 else 'fail'
            }, f)
        
        print(f"✅ Report generated with {total} violations")
        print(f"   HTML Report: {output_file}")
        print(f"   Summary: {summary_file}")
        
        # Explicitly exit with success
        exit(0)
        
    except Exception as e:
        print(f"❌ Error generating report: {e}")
        import traceback
        traceback.print_exc()
        exit(1)