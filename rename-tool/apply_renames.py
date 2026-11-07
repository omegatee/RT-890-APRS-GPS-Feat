#!/usr/bin/env python3
"""
Renames the dial / VFO / MEM functions of the RT-890 firmware (see renames.tsv) and the
parameter names inside them.  Usage:   python3 apply_renames.py [project_dir]
Safe to run on a tree that already has some of the renames applied (old names that are gone are skipped).
"""
import os, re, sys

root = sys.argv[1] if len(sys.argv) > 1 else '.'
here = os.path.dirname(os.path.abspath(__file__))

renames = {}
for line in open(os.path.join(here, 'renames.tsv')):
    line = line.rstrip('\n')
    if line and not line.startswith('#'):
        old, new = line.split('\t')
        renames[old] = new

# parameter names, keyed by the OLD function name
Vfo2Dial = {'Vfo': 'Dial'}
params = {
    'CHANNELS_LoadChannel':     {'ChNo': 'MemNo', 'Vfo': 'Dial'},
    'CHANNELS_SaveChannel':     {'Channel': 'MemNo', 'pChannel': 'pState'},
    'CHANNELS_GetChannelUp':    {'Channel': 'MemNo', 'Vfo': 'Dial'},
    'CHANNELS_GetChannelDown':  {'Channel': 'MemNo', 'Vfo': 'Dial'},
    'SETTINGS_SetWorkMode':     {'bChannelMode': 'bMemMode'},
    'UI_DrawChannel':           {'Channel': 'StoredNumber'},
    'UI_DrawChannelName':       {'Channel': 'MemNo'},
    'UI_DrawName': Vfo2Dial, 'UI_DrawFrequency': Vfo2Dial, 'UI_DrawFrequencyEx': Vfo2Dial,
    'UI_DrawDigits': Vfo2Dial, 'UI_DrawRegisters': Vfo2Dial, 'UI_DrawCss': Vfo2Dial,
    'UI_DrawTxPower': Vfo2Dial, 'UI_DrawRX': Vfo2Dial, 'UI_DrawBar': Vfo2Dial,
    'UI_DrawMainBitmap': Vfo2Dial, 'UI_DrawScanLists': Vfo2Dial, 'DrawRadio': Vfo2Dial,
    'UI_DrawModType': {'dial': 'Dial'}, 'UI_DrawDial': {'dial': 'Dial'}, 'DrawBandwidth': {'dial': 'Dial'},
}

def source_files():
    for d, dirs, files in os.walk(root):
        dirs[:] = [x for x in dirs if x not in ('external', 'Documents', '.vs', '.git')]
        for f in files:
            if f.endswith(('.c', '.h')):
                yield os.path.join(d, f)

def rename_params(text, path):
    lines = text.split('\n')
    out_changes = 0
    for func, pmap in params.items():
        defpat = re.compile(r'^[A-Za-z_][\w \t\*]*?\b' + re.escape(func) + r'\s*\(')
        i = 0
        while i < len(lines):
            ln = lines[i]
            if defpat.match(ln):
                if ln.rstrip().endswith(';'):                  # prototype: just this line
                    j = i
                else:                                          # definition: up to the closing brace at column 0
                    j = i
                    while j < len(lines) and lines[j].rstrip() != '}':
                        j += 1
                region = '\n'.join(lines[i:j + 1])
                for o, n in pmap.items():
                    if re.search(r'\b' + n + r'\b', region):
                        sys.exit('%s: %s already uses identifier "%s", refusing to rename "%s"' % (path, func, n, o))
                    region, c = re.subn(r'\b' + o + r'\b', n, region)
                    out_changes += c
                lines[i:j + 1] = region.split('\n')
                i = j
            i += 1
    return '\n'.join(lines), out_changes

total_files = total_names = total_params = 0
pat = re.compile(r'\b(' + '|'.join(sorted(map(re.escape, renames), key=len, reverse=True)) + r')\b')
for path in source_files():
    text = open(path, newline='').read()
    new, pc = rename_params(text, path)
    new, nc = pat.subn(lambda m: renames[m.group(1)], new)
    if new != text:
        open(path, 'w', newline='').write(new)
        total_files += 1; total_names += nc; total_params += pc
print('files changed: %d, function names replaced: %d, parameter names replaced: %d' % (total_files, total_names, total_params))
