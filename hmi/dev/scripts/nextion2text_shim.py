#!/usr/bin/env python3
"""
nextion2text_shim.py

Runs the pinned upstream Nextion2Text.py unmodified on any platform, producing
the same pure-ASCII output format the project has always tracked.

Nextion2Text was written for Windows and relies on two behaviours that do not
survive a move to Linux:

1. It decodes HMI strings with the "ansi" codec, which exists only on Windows.
   This shim registers "ansi" as an alias for iso-8859-1, which maps every byte
   value to the identical code point and therefore never rejects input. That
   matters because HMI files contain bytes cp1252 leaves undefined.

2. It opens its output files with no explicit encoding, so the result depends on
   the machine's locale. This shim pins that to ASCII with backslashreplace, so
   every non-ASCII code point is written back as \\xHH. Combined with the
   latin-1 decode above, each original HMI byte reappears as its own escape:
   the MDI icon b"\\xee\\x92\\x97" is written as the text \\xee\\x92\\x97.

The result is byte-for-byte reproducible on any platform and contains no
non-ASCII characters, which keeps the generated dumps readable in diffs.

The upstream file is left untouched, so the pinned revision stays verifiable
against its source.

Usage:
    python3 nextion2text_shim.py <Nextion2Text.py> [tool arguments...]
"""

import builtins
import codecs
import runpy
import sys

# Output encoding for the generated text dumps. ASCII plus backslashreplace is
# what produces the tracked \xHH escape format; do not change it without
# regenerating the whole hmi/dev/nextion2text tree.
_OUTPUT_ENCODING = "ascii"
_OUTPUT_ERRORS = "backslashreplace"


def _lookup_ansi(name):
    """Resolve the Windows-only "ansi" codec to iso-8859-1."""
    if name.lower() == "ansi":
        return codecs.lookup("iso-8859-1")
    # if ansi

    return None
# _lookup_ansi


_real_open = builtins.open


def _open_as_ascii(file, mode="r", buffering=-1, encoding=None,
                   errors=None, newline=None, closefd=True, opener=None):
    """Pin text-mode output to ASCII with backslashreplace.

    Nextion2Text opens its output with no explicit encoding, which would
    otherwise make the result depend on the locale of the machine running it.
    """
    if "b" not in mode and encoding is None:
        encoding = _OUTPUT_ENCODING

        if errors is None:
            errors = _OUTPUT_ERRORS
        # if default error handler
    # if text mode without explicit encoding

    return _real_open(file, mode, buffering, encoding,
                      errors, newline, closefd, opener)
# _open_as_ascii


def main():
    """Register the codec and encoding default, then run the tool directly."""
    if len(sys.argv) < 2:
        print("ERROR: Path to Nextion2Text.py is required.", file=sys.stderr)
        return 2
    # if tool path missing

    codecs.register(_lookup_ansi)

    # runpy reads the tool's source through io.open_code, not builtins.open,
    # so patching here does not affect how the script itself is loaded.
    builtins.open = _open_as_ascii

    tool = sys.argv[1]
    sys.argv = [tool] + sys.argv[2:]  # Hide the shim from the tool's argparse

    try:
        # Nextion2Text has no main guard; run_path executes it as __main__.
        runpy.run_path(tool, run_name="__main__")
    finally:
        builtins.open = _real_open
    # try

    return 0
# main


if __name__ == "__main__":
    sys.exit(main())
# __main__
