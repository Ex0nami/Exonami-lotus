"""Inspect the actual ESP32 ELF's compiler-emitted stack-frame information."""
from pathlib import Path
from elftools.elf.elffile import ELFFile
root=Path(__file__).resolve().parents[1]
with (root/'firmware/build/sticky_lotus_firmware.elf').open('rb') as f:
    elf=ELFFile(f)
    target='_ZN12sticky_lotus3app11Application8rollDiceEv'
    symbols=elf.get_section_by_name('.symtab')
    address=next(s['st_value'] for s in symbols.iter_symbols() if s.name==target)
    frames=[entry for entry in elf.get_dwarf_info().CFI_entries() if entry.header.get('initial_location')==address]
    assert frames, 'Missing rollDice frame metadata; stack check cannot be skipped'
    offsets=[row['cfa'].offset for frame in frames for row in frame.get_decoded().table if row['cfa'].offset is not None]
    size=max(offsets)
    assert size<=512, f'Unsafe dice stack frame: {size} bytes (limit 512); previous firmware used 2624'
    print(f'PASS: ESP32 rollDice stack frame is {size} bytes (was 2624).')
