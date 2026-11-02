"""Check commander UI/storage and bounded renderer stack in the linked ESP32 image."""
from pathlib import Path
from elftools.elf.elffile import ELFFile
root=Path(__file__).resolve().parents[1]
app=(root/'firmware/build/sticky_lotus_firmware.bin').read_bytes()
for marker in ['session_v2','session_v1','Partner commanders: ON','Partner commanders: OFF','Own commander','Power hold enabled on GPIO']:
    assert marker.encode() in app, 'Missing linked marker: '+marker
with (root/'firmware/build/sticky_lotus_firmware.elf').open('rb') as f:
    elf=ELFFile(f);symbols=list(elf.get_section_by_name('.symtab').iter_symbols());frames=elf.get_dwarf_info().CFI_entries()
    for name in ['drawCommanderDamageRegion']:
        addresses={s['st_value'] for s in symbols if name in s.name and s['st_info']['type']=='STT_FUNC'}
        sizes=[row['cfa'].offset for entry in frames if entry.header.get('initial_location') in addresses for row in entry.get_decoded().table if row['cfa'].offset is not None]
        assert sizes, 'Missing stack metadata: '+name
        assert max(sizes)<=768, f'{name} needs stack review: {max(sizes)}'
        print(f'PASS: {name} stack frame {max(sizes)} bytes.')
print('PASS: commander toggle, own damage, v2 storage/v1 migration and battery latch linked.')
