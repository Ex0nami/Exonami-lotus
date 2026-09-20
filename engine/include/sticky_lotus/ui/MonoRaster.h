#pragma once
#include <algorithm>
#include <cstdint>
#include <cstring>
namespace sticky_lotus::ui {
// MSB-first packed bitmap, width divisible by eight. Preserve pixels outside rect.
inline void fillMonoRect(std::uint8_t* data, int width, int height,
                         int left, int top, int right, int bottom, bool black) {
    if (!data || width<=0 || height<=0 || width%8) return;
    left=std::clamp(left,0,width); right=std::clamp(right,0,width);
    top=std::clamp(top,0,height); bottom=std::clamp(bottom,0,height);
    if(left>=right || top>=bottom) return;
    const int first=left/8, last=(right-1)/8, stride=width/8;
    const auto head=static_cast<std::uint8_t>(0xffu>>(left%8));
    const auto tail=static_cast<std::uint8_t>(0xffu<<(7-(right-1)%8));
    const auto apply=[black](std::uint8_t& byte, std::uint8_t mask) {
        if(black) byte &= static_cast<std::uint8_t>(~mask); else byte |= mask;
    };
    for(int y=top;y<bottom;++y) {
        auto* row=data+y*stride;
        if(first==last) { apply(row[first],head&tail); continue; }
        apply(row[first],head);
        if(last>first+1) std::memset(row+first+1,black ? 0 : 255,last-first-1);
        apply(row[last],tail);
    }
}
}
