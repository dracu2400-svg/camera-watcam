#!/bin/bash

##
# PDF Generation Script for Complete Project Guide
#
# This script converts COMPLETE_PROJECT_GUIDE.md to PDF
##

echo "========================================"
echo "  Generating PDF from Markdown"
echo "========================================"
echo ""

# Check for required tools
if ! command -v pandoc &> /dev/null; then
    echo "ERROR: pandoc is not installed."
    echo ""
    echo "Please install pandoc:"
    echo "  Ubuntu/Debian: sudo apt-get install pandoc"
    echo "  macOS: brew install pandoc"
    echo "  Windows: choco install pandoc"
    echo ""
    exit 1
fi

# Check for PDF engine
HAS_XELATEX=0
HAS_LUALATEX=0

if command -v xelatex &> /dev/null; then
    HAS_XELATEX=1
    echo "✓ XeLaTeX found"
fi

if command -v lualatex &> /dev/null; then
    HAS_LUALATEX=1
    echo "✓ LuaLaTeX found"
fi

if [ $HAS_XELATEX -eq 0 ] && [ $HAS_LUALATEX -eq 0 ]; then
    echo "ERROR: No suitable PDF engine found."
    echo ""
    echo "Please install one of:"
    echo "  - XeLaTeX (recommended): sudo apt-get install texlive-xetex"
    echo "  - LuaLaTeX: sudo apt-get install texlive-luatex"
    echo ""
    echo "For full LaTeX: sudo apt-get install texlive-full"
    echo ""
    exit 1
fi

# Select PDF engine
if [ $HAS_XELATEX -eq 1 ]; then
    PDF_ENGINE="xelatex"
    echo "Using XeLaTeX"
elif [ $HAS_LUALATEX -eq 1 ]; then
    PDF_ENGINE="lualatex"
    echo "Using LuaLaTeX"
fi

echo ""
echo "Converting COMPLETE_PROJECT_GUIDE.md to PDF..."
echo ""

# Convert to PDF
pandoc COMPLETE_PROJECT_GUIDE.md \
    -o COMPLETE_PROJECT_GUIDE.pdf \
    --pdf-engine=$PDF_ENGINE \
    -V geometry:margin=1in \
    -V fontsize=11pt \
    -V documentclass=report \
    -V papersize=a4 \
    --toc \
    --toc-depth=3 \
    --number-sections \
    -V colorlinks=true \
    -V linkcolor=blue \
    -V urlcolor=blue \
    -V toccolor=black \
    2>&1

if [ $? -eq 0 ]; then
    echo ""
    echo "========================================"
    echo "✓ PDF generated successfully!"
    echo "========================================"
    echo ""
    echo "Output: COMPLETE_PROJECT_GUIDE.pdf"

    # Show file size
    if [ -f "COMPLETE_PROJECT_GUIDE.pdf" ]; then
        SIZE=$(du -h COMPLETE_PROJECT_GUIDE.pdf | cut -f1)
        PAGES=$(pdfinfo COMPLETE_PROJECT_GUIDE.pdf 2>/dev/null | grep "Pages:" | awk '{print $2}')

        echo "Size: $SIZE"
        if [ -n "$PAGES" ]; then
            echo "Pages: $PAGES"
        fi
    fi
    echo ""
else
    echo ""
    echo "========================================"
    echo "✗ PDF generation failed"
    echo "========================================"
    echo ""
    echo "Alternative methods:"
    echo ""
    echo "1. Online Converters:"
    echo "   - https://www.markdowntopdf.com/"
    echo "   - https://md2pdf.netlify.app/"
    echo "   - https://dillinger.io/ (Markdown editor with PDF export)"
    echo ""
    echo "2. Desktop Tools:"
    echo "   - Typora (https://typora.io/)"
    echo "   - VS Code with Markdown PDF extension"
    echo "   - Obsidian (https://obsidian.md/)"
    echo ""
    echo "3. Command Line (alternative):"
    echo "   - grip (GitHub-flavored Markdown):"
    echo "     pip install grip"
    echo "     grip COMPLETE_PROJECT_GUIDE.md --export COMPLETE_PROJECT_GUIDE.html"
    echo "     # Then print HTML to PDF from browser"
    echo ""
    exit 1
fi
