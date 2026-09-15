"""Check that the linked firmware contains the actual custom sleep framebuffer."""
from pathlib import Path
import re
root = Path(__file__).resolve().parents[1]
header = (root / 'assets/sleep/exonami_sleep.h').read_text()
initializer = header.split('= {', 1)[1].split('};', 1)[0]
artwork = bytes(int(v, 16) for v in re.findall(r'0x([0-9A-F]{2})', initializer))
app = (root / 'firmware/build/sticky_lotus_firmware.bin').read_bytes()
merged = (root / 'firmware/build/merged-binary.bin').read_bytes()
if len(artwork) != 48000 or artwork not in app:
    raise SystemExit('FAIL: Exonami sleep bitmap is missing from the linked application.')
if merged[0x20000:0x20000 + len(app)] != app:
    raise SystemExit('FAIL: Merged image does not contain the same application.')
print('PASS: complete 48,000-byte Exonami artwork is present in the application and merged firmware.')
