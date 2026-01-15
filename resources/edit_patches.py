#!/usr/bin/env python3

import sys
from pathlib import Path
import re

if len(sys.argv) != 2:
    print("Usage: edit_patches.py <xlab_build_dir>")
    sys.exit(1)

root = Path(sys.argv[1])

if not root.is_dir():
    print(f"Error: {root} is not a directory")
    sys.exit(1)

# Regex patterns
else_object = re.compile(r'\belse/([A-Za-z0-9_.~]+)')
declare_else = re.compile(r'\bdeclare\s+-lib\s+else\b')
relative_saf = re.compile(r'\.\./Sources/(saf\.[A-Za-z0-9_.~]+)')

for pd_file in root.rglob("*.pd"):
    text = pd_file.read_text(encoding="utf-8")

    new_text = text
    new_text = else_object.sub(r'\1', new_text)
    new_text = declare_else.sub('declare -lib xlab', new_text)
    new_text = relative_saf.sub(r'\1', new_text)

    if new_text != text:
        pd_file.write_text(new_text, encoding="utf-8")
        print(f"patched: {pd_file}")
