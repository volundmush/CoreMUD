#pragma once
#include "core/base.h"
namespace core {
    extern std::function<std::string(std::string_view,ColorType)> renderAnsi;
    std::string defaultRenderAnsi(std::string_view input, ColorType color);

    extern std::function<std::string(std::string_view)> stripAnsi;
    std::string defaultStripAnsi(std::string_view input);

    extern std::function<std::string(std::string_view, ColorType)> renderAnsi;

}