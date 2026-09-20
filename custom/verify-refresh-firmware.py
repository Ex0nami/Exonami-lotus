from pathlib import Path
from elftools.elf.elffile import ELFFile
root=Path(__file__).resolve().parents[1]
app=(root/'firmware/build/sticky_lotus_firmware.bin').read_bytes()
for marker in (b'Performance: internal_free=', b'Refresh screen', b'Running scheduled full E-paper cleanup', b'retrying after one second'):
    assert marker in app, f'Refresh update missing: {marker!r}'
with (root/'firmware/build/sticky_lotus_firmware.elf').open('rb') as f:
    elf=ELFFile(f)
    addresses={s['st_value'] for s in elf.get_section_by_name('.symtab').iter_symbols() if 'ActionHistory4text' in s.name}
    sizes=[]
    for entry in elf.get_dwarf_info().CFI_entries():
        if entry.header.get('initial_location') in addresses:
            sizes += [r['cfa'].offset for r in entry.get_decoded().table if r['cfa'].offset is not None]
    if sizes: print(f'History formatter stack frame: {max(sizes)} bytes (previous build 960).')
print('PASS: manual cleanup, scheduled cleanup and retry backoff are linked into firmware.')
