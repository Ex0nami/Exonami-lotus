#pragma once
#include "sticky_lotus/ui/Canvas.h"
// Drawing calls are recorded by RecordedCanvas; no native window is opened.
namespace sticky_lotus::ui {
struct HeadlessCanvas : Canvas {
    void beginFrame() override {}
    void endFrame() override {}
    void clear(Ink) override {}
    void fillRect(const Rect&, Ink) override {}
    void drawRect(const Rect&, float, Ink) override {}
    void fillRoundedRect(const Rect&, float, Ink) override {}
    void drawRoundedRect(const Rect&, float, float, Ink) override {}
    void drawLine(Point, Point, float, Ink) override {}
    void fillCircle(Point, float, Ink) override {}
    void drawCircle(Point, float, float, Ink) override {}
    void flush() override {}
    void flushImmediately() override {}
    void invalidate(const Rect&) override {}
    void drawMonochromeBitmap(const std::uint8_t*, int, int, int, const Rect&, float) override {}
    void drawText(std::string_view, const Rect&, int, Ink, TextAlignment, float) override {}
};
}
