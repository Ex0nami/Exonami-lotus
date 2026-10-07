"""Verify linked player screens, seeds, persistence and bounded UI stack frames."""
from pathlib import Path
from elftools.elf.elffile import ELFFile
root=Path(__file__).resolve().parents[1]
app=(root/'firmware/build/sticky_lotus_firmware.bin').read_bytes()
for marker in ['v1.3','session_v1','Players & scores','Who won this game?','Undo last recorded win?',
               'Steve','Chris','Marvin','Marian','Erwin','Max','Fabi','Duki','David','Marcel','Emil']:
    assert marker.encode() in app, f'Missing firmware marker: {marker}'
with (root/'firmware/build/sticky_lotus_firmware.elf').open('rb') as f:
    elf=ELFFile(f)
    symbols=list(elf.get_section_by_name('.symtab').iter_symbols())
    frames=elf.get_dwarf_info().CFI_entries()
    for name in ['18handleProfileInput','12drawProfiles','saveGameState']:
        addresses={s['st_value'] for s in symbols if name in s.name and s['st_info']['type']=='STT_FUNC'}
        sizes=[row['cfa'].offset for entry in frames if entry.header.get('initial_location') in addresses
               for row in entry.get_decoded().table if row['cfa'].offset is not None]
        assert sizes, f'Missing stack metadata for {name}'
        size=max(sizes)
        assert size<=768, f'{name} stack frame needs review: {size} bytes'
        print(f'PASS: {name} maximum individual stack frame {size} bytes.')
print('PASS: profiles, all eleven names, winner prompt, session storage and v1.3 linked.')
