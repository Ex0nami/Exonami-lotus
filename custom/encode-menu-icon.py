from pathlib import Path
from PIL import Image
root=Path(__file__).resolve().parents[1]
folder=root/'assets/icons'
source=Image.open(folder/'planeswalker-source.png').convert('RGB')
# Extract the colored symbol from its neutral background for the 1-bit panel.
mask=Image.new('L',source.size)
mask.putdata([255 if r>g+20 and r>b+20 else 0 for r,g,b in source.getdata()])
mask=mask.crop(mask.getbbox())
mask.thumbnail((28,44),Image.Resampling.LANCZOS)
icon=Image.new('1',(32,44),1)
mono=mask.point(lambda v:0 if v>=96 else 255,mode='1')
icon.paste(mono,((32-mono.width)//2,(44-mono.height)//2))
data=icon.tobytes()
assert len(data)==176
assert Image.frombytes('1',(32,44),data).tobytes()==icon.tobytes()
icon.save(folder/'planeswalker-menu.png')
header='// Encoded from user-provided Planeswalker image. MSB first; 1=white.\n#pragma once\n#include <cstdint>\n#define PLANESWALKER_ICON_WIDTH 32\n#define PLANESWALKER_ICON_HEIGHT 44\n#define PLANESWALKER_ICON_BYTES_PER_ROW 4\ninline constexpr std::uint8_t planeswalker_icon[] = {\n'
header+='\n'.join('    '+', '.join(f'0x{v:02X}' for v in data[i:i+16])+',' for i in range(0,len(data),16))
header+='\n};\nstatic_assert(sizeof(planeswalker_icon)==176);\n'
(folder/'planeswalker_menu_icon.h').write_text(header)
print('Encoded exact source silhouette as 32x44 menu asset; round-trip passed.')
