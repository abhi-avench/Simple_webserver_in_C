#!/bin/bash
# Local test script for MISRA CI/CD pipeline
# All generated files go into build/ directory

set -e  # Exit on error

echo "========================================="
echo "🔍 Local MISRA CI/CD Pipeline Test"
echo "========================================="

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Create build directory if it doesn't exist
mkdir -p build

# Step 1: Build the project (output already in build/)
echo -e "\n${YELLOW}📦 Step 1: Building project...${NC}"
cd build
cmake ..
make
cd ..
echo -e "${GREEN}✅ Build completed in build/${NC}"

# Step 2: Run cppcheck MISRA analysis (output in build/)
echo -e "\n${YELLOW}🔍 Step 2: Running MISRA analysis with cppcheck...${NC}"
cppcheck --enable=all \
         --inconclusive \
         --suppress=missingIncludeSystem \
         --suppress=unmatchedSuppression \
         --xml \
         --xml-version=2 \
         src/ 2> build/cppcheck_output.xml

if [ -s build/cppcheck_output.xml ]; then
    echo -e "${GREEN}✅ cppcheck analysis completed${NC}"
    # Show summary
    VIOLATION_COUNT=$(grep -c "<error" build/cppcheck_output.xml || echo "0")
    echo "   Total violations found: $VIOLATION_COUNT"
    echo "   Output saved to: build/cppcheck_output.xml"
else
    echo -e "${YELLOW}⚠️ No violations found or empty output${NC}"
    echo '<results><errors/></results>' > build/cppcheck_output.xml
fi

# Step 3: Generate HTML report (output in build/)
echo -e "\n${YELLOW}📄 Step 3: Generating HTML report...${NC}"

# Set mock GitHub environment variables for local testing
export GITHUB_REPOSITORY="local/test-repo"
export GITHUB_SHA="$(git rev-parse HEAD 2>/dev/null || echo 'local-test')"
export GITHUB_REF_NAME="$(git branch --show-current 2>/dev/null || echo 'local')"
export GITHUB_RUN_NUMBER="1"
export GITHUB_ACTOR="$(whoami)"
export GITHUB_RUN_ID="local-$$"
export PROJECT_NAME="Simple Webserver (Local Test)"

# Change to build directory to run report generator
cd build
python3 ../scripts/generate_report.py
cd ..

if [ -f build/misra_report.html ]; then
    echo -e "${GREEN}✅ HTML report generated: build/misra_report.html${NC}"
    # Show file size
    ls -lh build/misra_report.html
else
    echo -e "${RED}❌ Failed to generate report${NC}"
    exit 1
fi

# Step 4: Test EmailJS sending (optional - requires credentials)
echo -e "\n${YELLOW}📧 Step 4: Testing EmailJS email sending...${NC}"

# Check if we have email credentials set locally
if [ -f .env.local ]; then
    source .env.local
    echo "   Using credentials from .env.local"
fi

if [ -n "$EMAILJS_SERVICE_ID" ] && [ -n "$EMAILJS_TEMPLATE_ID" ] && [ -n "$EMAILJS_USER_ID" ]; then
    echo "   Sending test email to: ${EMAILJS_RECIPIENT:-'not set'}"
    
    # Extract top violations for email (using build/cppcheck_output.xml)
    cd build
    python3 << 'EOF' > violations_table.txt
import xml.etree.ElementTree as ET
import html
import re

violations_html = ""
try:
    tree = ET.parse('cppcheck_output.xml')
    root = tree.getroot()
    errors = root.findall('.//error')
    
    if errors:
        for error in errors[:5]:  # Top 5 for email
            location = error.find('location')
            if location is not None:
                file_path = location.get('file', 'Unknown')
                line_num = location.get('line', '?')
                msg = error.get('msg', 'No message')
                
                rule = 'N/A'
                misra_match = re.search(r'misra c[:\s]+(\d+\.\d+)', msg.lower())
                if misra_match:
                    rule = misra_match.group(1)
                
                violations_html += f'<tr><td>{html.escape(rule)}</td><td>{html.escape(file_path)}</td><td>{line_num}</td><td>{html.escape(msg[:100])}</td></tr>'
    
    with open('violations_table.txt', 'w') as f:
        f.write(violations_html if violations_html else '<tr><td colspan="4">No violations found</td></tr>')
except Exception as e:
    with open('violations_table.txt', 'w') as f:
        f.write(f'<tr><td colspan="4">Error: {e}</td></tr>')
EOF
    
    VIOLATIONS_TABLE=$(cat violations_table.txt)
    cd ..
    
    # Prepare status
    if [ ! -s build/cppcheck_output.xml ] || [ $(grep -c "<error" build/cppcheck_output.xml) -eq 0 ]; then
        STATUS_TEXT="✅ PASS - No MISRA Violations"
        STATUS_CLASS="status-pass"
        TOTAL_VIOLATIONS="0"
    else
        TOTAL_VIOLATIONS=$(grep -c "<error" build/cppcheck_output.xml)
        STATUS_TEXT="⚠️ FAIL - ${TOTAL_VIOLATIONS} Violations Found"
        STATUS_CLASS="status-fail"
    fi
    
    # Send test email
    RESPONSE=$(curl -s -w "\n%{http_code}" -X POST https://api.emailjs.com/api/v1.0/email/send \
      -H 'Content-Type: application/json' \
      -H 'Origin: http://localhost' \
      -d "{
        \"service_id\": \"${EMAILJS_SERVICE_ID}\",
        \"template_id\": \"${EMAILJS_TEMPLATE_ID}\",
        \"user_id\": \"${EMAILJS_USER_ID}\",
        \"template_params\": {
          \"status\": \"${STATUS_TEXT}\",
          \"status_class\": \"${STATUS_CLASS}\",
          \"project_name\": \"Simple Webserver (Local Test)\",
          \"repo_name\": \"local-test\",
          \"commit_sha\": \"$(git rev-parse HEAD 2>/dev/null | cut -c1-7 || echo 'local')\",
          \"branch\": \"$(git branch --show-current 2>/dev/null || echo 'local')\",
          \"run_number\": \"LOCAL\",
          \"triggered_by\": \"$(whoami)\",
          \"timestamp\": \"$(date '+%Y-%m-%d %H:%M:%S')\",
          \"total_violations\": \"${TOTAL_VIOLATIONS}\",
          \"violations_table\": $(echo "$VIOLATIONS_TABLE" | python3 -c 'import json,sys; print(json.dumps(sys.stdin.read()))'),
          \"artifact_link\": \"file://$(pwd)/build/misra_report.html\",
          \"workflow_link\": \"local-test\",
          \"to_email\": \"${EMAILJS_RECIPIENT}\"
        }
      }")
    
    HTTP_CODE=$(echo "$RESPONSE" | tail -n1)
    if [ "$HTTP_CODE" = "200" ]; then
        echo -e "${GREEN}✅ Test email sent successfully!${NC}"
        echo "   Check your inbox at: ${EMAILJS_RECIPIENT}"
    else
        echo -e "${RED}❌ Failed to send email. HTTP Code: ${HTTP_CODE}${NC}"
        echo "   Response: $(echo "$RESPONSE" | sed '$d')"
    fi
else
    echo -e "${YELLOW}⚠️ EmailJS credentials not set. Skipping email test.${NC}"
    echo "   To test email, create a .env.local file with:"
    echo "   EMAILJS_SERVICE_ID=your_service_id"
    echo "   EMAILJS_TEMPLATE_ID=your_template_id"
    echo "   EMAILJS_USER_ID=your_user_id"
    echo "   EMAILJS_RECIPIENT=your_email@example.com"
fi

# Step 5: Open report in browser (optional)
echo -e "\n${YELLOW}🌐 Step 5: Opening report in browser...${NC}"
if [ -f build/misra_report.html ]; then
    if command -v xdg-open &> /dev/null; then
        xdg-open build/misra_report.html 2>/dev/null || echo "   Report saved as build/misra_report.html"
    elif command -v open &> /dev/null; then
        open build/misra_report.html 2>/dev/null || echo "   Report saved as build/misra_report.html"
    else
        echo "   Report saved as build/misra_report.html"
    fi
fi

# Step 6: Show summary of generated files
echo -e "\n${GREEN}========================================="
echo "✅ Local test completed successfully!"
echo "=========================================${NC}"
echo ""
echo "📊 Generated files in build/ directory:"
ls -lh build/*.html build/*.xml build/*.json 2>/dev/null || echo "   No generated files found"
echo ""
echo "📁 Build directory contents:"
ls -la build/ | head -10
echo ""
echo "Next steps:"
echo "   1. Review the HTML report: open build/misra_report.html"
echo "   2. Check cppcheck output: cat build/cppcheck_output.xml"
echo "   3. If email was sent, check your inbox"
echo "   4. Push to GitHub to test the full CI/CD pipeline"