#include "Apps/Terminal/VtParser.h"

#include <algorithm>
#include <cstdlib>

namespace {
bool printable(wchar_t wc) { return wc >= 0x20 && wc != 0x7f; }

short rgb_to_xterm(unsigned char red, unsigned char green, unsigned char blue)
{
    const int red_cube = static_cast<int>((red * 5 + 127) / 255);
    const int green_cube = static_cast<int>((green * 5 + 127) / 255);
    const int blue_cube = static_cast<int>((blue * 5 + 127) / 255);
    const int cube_red = red_cube == 0 ? 0 : 55 + red_cube * 40;
    const int cube_green = green_cube == 0 ? 0 : 55 + green_cube * 40;
    const int cube_blue = blue_cube == 0 ? 0 : 55 + blue_cube * 40;
    const int cube_distance =
        (static_cast<int>(red) - cube_red) * (static_cast<int>(red) - cube_red) +
        (static_cast<int>(green) - cube_green) * (static_cast<int>(green) - cube_green) +
        (static_cast<int>(blue) - cube_blue) * (static_cast<int>(blue) - cube_blue);

    int gray_level = static_cast<int>((red + green + blue) / 3);
    gray_level = std::clamp((gray_level - 8 + 5) / 10, 0, 23);
    const int gray_value = 8 + gray_level * 10;
    const int gray_distance =
        (static_cast<int>(red) - gray_value) * (static_cast<int>(red) - gray_value) +
        (static_cast<int>(green) - gray_value) * (static_cast<int>(green) - gray_value) +
        (static_cast<int>(blue) - gray_value) * (static_cast<int>(blue) - gray_value);

    if (gray_distance < cube_distance)
        return static_cast<short>(232 + gray_level);
    return static_cast<short>(16 + red_cube * 36 + green_cube * 6 + blue_cube);
}
}

VtParser::VtParser(TermBuffer& buffer)
    : buf(&buffer), state(State::Ground), cur_fg(7), cur_bg(0), cur_bold(false),
      utf8_codepoint(0), utf8_remaining(0), saved_row(0), saved_col(0),
    scroll_top(0), scroll_bottom(buffer.get_rows() - 1), string_escape(false)
{
}

void VtParser::feed(const char* data, std::size_t len)
{
    for (std::size_t i = 0; i < len; ++i)
        process_byte(static_cast<unsigned char>(data[i]));
}

void VtParser::reset_parser()
{
    state = State::Ground;
    csi_params.clear();
    intermediates.clear();
    string_escape = false;
}

void VtParser::print_codepoint(wchar_t wc)
{
    if (!printable(wc))
        return;
    int row = buf->get_cursor_row();
    int col = buf->get_cursor_col();
    buf->put(row, col, wc, cur_fg, cur_bg, cur_bold);
    if (++col >= buf->get_cols()) {
        col = 0;
        ++row;
        if (row > scroll_bottom) {
            buf->scroll_up(scroll_top, scroll_bottom);
            row = scroll_bottom;
        }
    }
    buf->set_cursor(row, col);
}

void VtParser::execute_control(unsigned char c)
{
    int row = buf->get_cursor_row();
    int col = buf->get_cursor_col();
    switch (c) {
    case '\n':
        if (++row > scroll_bottom) {
            buf->scroll_up(scroll_top, scroll_bottom);
            row = scroll_bottom;
        }
        buf->set_cursor(row, col);
        break;
    case '\r': buf->set_cursor(row, 0); break;
    case '\b': buf->set_cursor(row, std::max(0, col - 1)); break;
    case '\t': buf->set_cursor(row, std::min(buf->get_cols() - 1, (col / 8 + 1) * 8)); break;
    default: break;
    }
}

std::vector<int> VtParser::parse_params() const
{
    std::vector<int> result;
    std::size_t start = 0;
    while (start <= csi_params.size()) {
        const std::size_t end = csi_params.find(';', start);
        const std::string part = csi_params.substr(start, end - start);
        result.push_back(part.empty() ? 0 : std::atoi(part.c_str()));
        if (end == std::string::npos)
            break;
        start = end + 1;
    }
    return result;
}

int VtParser::parameter(const std::vector<int>& params, std::size_t index,
                        int default_value) const
{
    if (index >= params.size() || params[index] == 0)
        return default_value;
    return params[index];
}

void VtParser::apply_sgr(const std::vector<int>& params)
{
    const std::vector<int> values = params.empty() ? std::vector<int>{0} : params;
    for (std::size_t i = 0; i < values.size(); ++i) {
        const int value = values[i];
        if (value == 0) { cur_fg = 7; cur_bg = 0; cur_bold = false; }
        else if (value == 1) cur_bold = true;
        else if (value == 22) cur_bold = false;
        else if (value == 39) cur_fg = 7;
        else if (value == 49) cur_bg = 0;
        else if (value >= 30 && value <= 37) cur_fg = static_cast<short>(value - 30);
        else if (value >= 40 && value <= 47) cur_bg = static_cast<short>(value - 40);
        else if (value >= 90 && value <= 97) cur_fg = static_cast<short>(value - 90 + 8);
        else if (value >= 100 && value <= 107) cur_bg = static_cast<short>(value - 100 + 8);
        else if ((value == 38 || value == 48) && i + 2 < values.size() && values[i + 1] == 5) {
            short& color = value == 38 ? cur_fg : cur_bg;
            color = static_cast<short>(std::clamp(values[i + 2], 0, 255));
            i += 2;
        }
        else if ((value == 38 || value == 48) && i + 4 < values.size() && values[i + 1] == 2) {
            short& color = value == 38 ? cur_fg : cur_bg;
            color = rgb_to_xterm(
                static_cast<unsigned char>(std::clamp(values[i + 2], 0, 255)),
                static_cast<unsigned char>(std::clamp(values[i + 3], 0, 255)),
                static_cast<unsigned char>(std::clamp(values[i + 4], 0, 255)));
            i += 4;
        }
    }
}

void VtParser::dispatch_csi(char cmd)
{
    const std::vector<int> p = parse_params();
    const int row = buf->get_cursor_row();
    const int col = buf->get_cursor_col();
    switch (cmd) {
    case 'A': buf->set_cursor(row - parameter(p, 0), col); break;
    case 'B': case 'e': buf->set_cursor(row + parameter(p, 0), col); break;
    case 'C': case 'a': buf->set_cursor(row, col + parameter(p, 0)); break;
    case 'D': buf->set_cursor(row, col - parameter(p, 0)); break;
    case 'E': buf->set_cursor(row + parameter(p, 0), 0); break;
    case 'F': buf->set_cursor(row - parameter(p, 0), 0); break;
    case 'G': case '`': buf->set_cursor(row, parameter(p, 0) - 1); break;
    case 'd': buf->set_cursor(parameter(p, 0) - 1, col); break;
    case 'H': case 'f': buf->set_cursor(parameter(p, 0) - 1, parameter(p, 1) - 1); break;
    case 'J': {
        const int mode = p.empty() ? 0 : p[0];
        if (mode == 2) buf->clear(0, 0, buf->get_rows(), buf->get_cols());
        else if (mode == 0) buf->clear(row, col, 1, buf->get_cols() - col);
        else if (mode == 1) buf->clear(0, 0, row + 1, buf->get_cols());
        break;
    }
    case 'K': {
        const int mode = p.empty() ? 0 : p[0];
        if (mode == 0) buf->clear(row, col, 1, buf->get_cols() - col);
        else if (mode == 1) buf->clear(row, 0, 1, col + 1);
        else if (mode == 2) buf->clear(row, 0, 1, buf->get_cols());
        break;
    }
    case 'm': apply_sgr(p); break;
    case 's': saved_row = row; saved_col = col; break;
    case 'u': buf->set_cursor(saved_row, saved_col); break;
    case 'r':
        scroll_top = std::clamp(parameter(p, 0) - 1, 0, buf->get_rows() - 1);
        scroll_bottom = std::clamp(parameter(p, 1, buf->get_rows()), scroll_top, buf->get_rows() - 1);
        buf->set_cursor(0, 0);
        break;
    case 'P': buf->clear(row, col, 1, parameter(p, 0)); break;
    case 'X': buf->clear(row, col, 1, parameter(p, 0), cur_fg, cur_bg, cur_bold); break;
    case 'S': for (int i = 0; i < parameter(p, 0); ++i) buf->scroll_up(scroll_top, scroll_bottom); break;
    default: break;
    }
}

void VtParser::process_byte(unsigned char c)
{
    if (state == State::String) {
        if (c == '\a') { reset_parser(); return; }
        if (string_escape && c == '\\') { reset_parser(); return; }
        string_escape = c == 0x1b;
        return;
    }
    if (utf8_remaining > 0) {
        if ((c & 0xc0) != 0x80) { utf8_remaining = 0; process_byte(c); return; }
        utf8_codepoint = (utf8_codepoint << 6) | (c & 0x3f);
        if (--utf8_remaining == 0) print_codepoint(static_cast<wchar_t>(utf8_codepoint));
        return;
    }
    if (c < 0x20 || c == 0x7f) {
        if (c == 0x1b) { state = State::Escape; csi_params.clear(); intermediates.clear(); return; }
        if (state == State::Ground) execute_control(c);
        return;
    }
    if (state == State::Ground) {
        if (c < 0x80) print_codepoint(static_cast<wchar_t>(c));
        else if ((c & 0xe0) == 0xc0) { utf8_codepoint = c & 0x1f; utf8_remaining = 1; }
        else if ((c & 0xf0) == 0xe0) { utf8_codepoint = c & 0x0f; utf8_remaining = 2; }
        else if ((c & 0xf8) == 0xf0) { utf8_codepoint = c & 0x07; utf8_remaining = 3; }
        return;
    }
    if (state == State::Escape) {
        if (c == '[') { state = State::CsiEntry; csi_params.clear(); return; }
        if (c == ']' || c == 'P' || c == 'X' || c == '^' || c == '_') {
            state = State::String;
            string_escape = false;
            return;
        }
        if (c == '7') { saved_row = buf->get_cursor_row(); saved_col = buf->get_cursor_col(); reset_parser(); return; }
        if (c == '8') { buf->set_cursor(saved_row, saved_col); reset_parser(); return; }
        if (c == 'c') { buf->clear(0, 0, buf->get_rows(), buf->get_cols()); buf->set_cursor(0, 0); cur_fg = 7; cur_bg = 0; cur_bold = false; reset_parser(); return; }
        reset_parser();
        return;
    }
    if (state == State::CsiEntry || state == State::CsiParam) {
        if ((c >= '0' && c <= '9') || c == ';' || c == '?' || c == '>') { csi_params += static_cast<char>(c); state = State::CsiParam; return; }
        if (c >= 0x20 && c <= 0x2f) { intermediates += static_cast<char>(c); state = State::CsiIntermediate; return; }
        if (c >= 0x40 && c <= 0x7e) { dispatch_csi(static_cast<char>(c)); reset_parser(); return; }
        reset_parser();
        return;
    }
    if (state == State::CsiIntermediate) {
        if (c >= 0x20 && c <= 0x2f) { intermediates += static_cast<char>(c); return; }
        if (c >= 0x40 && c <= 0x7e) { dispatch_csi(static_cast<char>(c)); reset_parser(); return; }
        reset_parser();
    }
}