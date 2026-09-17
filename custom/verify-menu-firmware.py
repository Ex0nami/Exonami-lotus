from pathlib import Path
import re
root=Path(__file__).resolve().parents[1]
header=(root/'assets/icons/planeswalker_menu_icon.h').read_text()
initializer=header.split('= {',1)[1].split('};',1)[0]
icon=bytes(int(v,16) for v in re.findall(r'0x([0-9A-F]{2})',initializer))
assert len(icon)==176
assert icon in (root/'firmware/build/sticky_lotus_firmware.bin').read_bytes(), 'Menu icon is missing from firmware'
print('PASS: Planeswalker icon is linked into firmware.')
