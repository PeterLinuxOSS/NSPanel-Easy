"""
nextion2text_dict.py

Custom dictionaries for Nextion2Text, passed via its -c option.

Upstream's built-in codeEvents dictionary is incomplete: keys it does not know
are printed raw, so a slider's touch-move handler appears as "codesslide"
instead of a readable name. Without -x or -r, Nextion2Text merges these entries
into the built-in dictionaries rather than replacing them, so only the additions
belong here.

Changing a name here rewrites every affected line in hmi/dev/nextion2text, so
regenerate the whole tree in the same commit.
"""

# Event keys missing from the upstream dictionary.
codeEvents = {
    "codesslide": "Touch Move Event",
}

# Declared empty so Nextion2Text's attribute merge is a no-op instead of a
# caught exception that prints a spurious warning.
attributes = {}
