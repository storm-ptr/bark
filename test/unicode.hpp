// Andrew Naplavkov

#ifndef BARK_TEST_UNICODE_HPP
#define BARK_TEST_UNICODE_HPP

#include <bark/unicode.hpp>
#include <iostream>
#include <locale>
#include <string>

TEST_CASE("unicode")
{
    using namespace bark::unicode;

    char16_t u16[] = {0x68,
                      0x65,
                      0x6c,
                      0x6c,
                      0x6f,
                      0x20,
                      0x43f,
                      0x440,
                      0x438,
                      0x432,
                      0x435,
                      0x442,
                      0};                     // UTF-16
    auto u8(to_upper(to_string<char>(u16)));  // UTF-8
    auto u32(to_string<char32_t>(u8));        // UTF-32
    auto w(to_string<wchar_t>(u32));          // UTF-16 or UTF-32
    REQUIRE(to_string<char>(w) == u8);
    // std::locale::global(std::locale("rus"));
    std::wcout << w << std::endl;
}

#endif  // BARK_TEST_UNICODE_HPP
