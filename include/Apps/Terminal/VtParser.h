#ifndef VTPARSER_H
#define VTPARSER_H

#include <string>
#include <vector>
#include <cwctype>
#include "Apps/Terminal/TermBuffer.h"
#include "Apps/Terminal/State.h"

class VtParser {
    private :
        TermBuffer* buf;
        State state;
        std::string csi_params;
        std::string intermediates;
        short cur_fg;
        short cur_bg;
        bool cur_bold;
        int utf8_codepoint;
        int utf8_remaining;
        int saved_row;
        int saved_col;
        int scroll_top;
        int scroll_bottom;
        bool string_escape;
    
    public :
        explicit VtParser(TermBuffer& buffer);
        void feed(const char* data, std::size_t len);
    
    private :
        void process_byte(unsigned char c);
        void dispatch_csi(char cmd);
        void apply_sgr(const std::vector<int>& params);
        void print_codepoint(wchar_t wc);
        void execute_control(unsigned char c);
        void reset_parser();
        std::vector<int> parse_params() const;
        int parameter(const std::vector<int>& params, std::size_t index,
                  int default_value = 1) const;
};

#endif // VTPARSER_H