from pathlib import Path
root=Path(__file__).resolve().parents[1]
app=(root/'firmware/build/sticky_lotus_firmware.bin').read_bytes()
for marker in (b'history_v1',b'No actions yet',b'Newest first',b'Previous'):
    assert marker in app, f'History marker missing: {marker!r}'
print('PASS: history interface and storage key are present in firmware.')
