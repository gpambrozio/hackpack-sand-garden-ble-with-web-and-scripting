#!/usr/bin/env python3
"""Convert HTML file to C++ header with PROGMEM string

Usage: python3 html_to_header.py web-client.html WebClientHTML.h
"""

import sys

def html_to_header(html_file, output_file):
    with open(html_file, 'r', encoding='utf-8') as f:
        html_content = f.read()

    # Create header file with raw string literal (no escaping needed)
    header = f'''// WebClientHTML.h - Embedded web client interface
// Auto-generated from web-client.html
// DO NOT EDIT MANUALLY - regenerate using html_to_header.py

#pragma once
#include <Arduino.h>

// Store HTML in flash memory (PROGMEM) to save RAM
const char WEB_CLIENT_HTML[] PROGMEM = R"rawliteral(
{html_content}
)rawliteral";

const size_t WEB_CLIENT_HTML_SIZE = sizeof(WEB_CLIENT_HTML) - 1;
'''

    with open(output_file, 'w', encoding='utf-8') as f:
        f.write(header)

    print(f"Generated {output_file} ({len(html_content)} bytes)")

if __name__ == '__main__':
    if len(sys.argv) != 3:
        print("Usage: html_to_header.py <input.html> <output.h>")
        sys.exit(1)

    html_to_header(sys.argv[1], sys.argv[2])
