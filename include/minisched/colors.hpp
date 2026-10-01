#pragma once

// ANSI terminal color codes. Print one before the text and RESET after it:
//   std::cout << color::YELLOW << "hello" << color::RESET;
namespace color {
    inline constexpr const char* RESET  = "\033[0m";
    inline constexpr const char* RED    = "\033[31m";
    inline constexpr const char* YELLOW = "\033[33m";
}
