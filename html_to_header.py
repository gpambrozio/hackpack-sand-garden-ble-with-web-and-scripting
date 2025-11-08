#!/usr/bin/env python3
"""Convert HTML file to C++ header with PROGMEM string

Usage: python3 html_to_header.py web-client.html WebClientHTML.h [--no-minify]
"""

import sys
import re

def minify_html(html):
    """Minify HTML by removing unnecessary whitespace and comments.

    This is a simple minifier that:
    - Removes HTML comments
    - Removes whitespace between tags
    - Collapses multiple spaces to single space
    - Preserves content within <script> and <style> tags
    """
    original_size = len(html)

    # Remove HTML comments (but not conditional comments)
    html = re.sub(r'<!--(?!\[if\s).*?-->', '', html, flags=re.DOTALL)

    # Split by script and style tags to preserve their content
    parts = []
    last_end = 0

    # Find all script and style blocks
    preserve_pattern = r'(<(?:script|style)[^>]*>)(.*?)(</(?:script|style)>)'

    for match in re.finditer(preserve_pattern, html, re.DOTALL | re.IGNORECASE):
        # Add the HTML before this block (minified)
        before = html[last_end:match.start()]
        parts.append(minify_html_content(before))

        # Add the script/style block (preserved but with minimal processing)
        opening_tag = match.group(1)
        content = match.group(2)
        closing_tag = match.group(3)

        # For script/style content, just remove leading/trailing whitespace from lines
        content = '\n'.join(line.strip() for line in content.split('\n'))
        parts.append(opening_tag + content + closing_tag)

        last_end = match.end()

    # Add remaining HTML after last script/style block
    if last_end < len(html):
        parts.append(minify_html_content(html[last_end:]))

    minified = ''.join(parts)

    print(f"  HTML minification: {original_size} bytes -> {len(minified)} bytes ({100 - len(minified)*100//original_size}% reduction)")

    return minified

def minify_html_content(html):
    """Minify HTML content (non-script/style parts)."""
    # Remove whitespace between tags
    html = re.sub(r'>\s+<', '><', html)

    # Collapse multiple spaces to single space
    html = re.sub(r'\s+', ' ', html)

    # Remove leading/trailing whitespace
    html = html.strip()

    return html

def html_to_header(html_file, output_file, minify=True):
    with open(html_file, 'r', encoding='utf-8') as f:
        html_content = f.read()

    original_size = len(html_content)

    # Minify HTML if requested
    if minify:
        html_content = minify_html(html_content)

    # Create header file with raw string literal (no escaping needed)
    header = f'''// WebClientHTML.h - Embedded web client interface
// Auto-generated from web-client.html
// DO NOT EDIT MANUALLY - regenerate using html_to_header.py

#pragma once
#include <Arduino.h>

// Store HTML in flash memory (PROGMEM) to save RAM
// Original size: {original_size} bytes, minified: {len(html_content)} bytes
const char WEB_CLIENT_HTML[] PROGMEM = R"rawliteral(
{html_content}
)rawliteral";

const size_t WEB_CLIENT_HTML_SIZE = sizeof(WEB_CLIENT_HTML) - 1;
'''

    with open(output_file, 'w', encoding='utf-8') as f:
        f.write(header)

    print(f"Generated {output_file} ({len(html_content)} bytes)")

if __name__ == '__main__':
    minify = True

    if len(sys.argv) < 3 or len(sys.argv) > 4:
        print("Usage: html_to_header.py <input.html> <output.h> [--no-minify]")
        sys.exit(1)

    if len(sys.argv) == 4 and sys.argv[3] == '--no-minify':
        minify = False

    html_to_header(sys.argv[1], sys.argv[2], minify)
