#include "core/color.h"

namespace core {
    std::string defaultRenderAnsi(std::string_view input, ColorType color) {
        return std::string(input);
    }
    std::function<std::string(std::string_view,ColorType)> renderAnsi(defaultRenderAnsi);

    std::string defaultStripAnsi(std::string_view input) {
        return renderAnsi(input, ColorType::NoColor);
    }
    std::function<std::string(std::string_view)> stripAnsi(defaultStripAnsi);
}