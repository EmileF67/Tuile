#include "Apps/Terminal/TermBuffer.h"
#include <algorithm>
#include <stdexcept>

namespace {
Cell blank_cell(short fg = 7, short bg = 0, bool bold = false)
{
    return Cell{' ', fg, bg, bold, true};
}
}

TermBuffer::TermBuffer(int row_count, int column_count)
    : rows(std::max(1, row_count)), cols(std::max(1, column_count)),
      grid(rows, std::vector<Cell>(cols, blank_cell())),
      cursor_row(0), cursor_col(0)
{
}

int TermBuffer::get_rows() const { return rows; }
int TermBuffer::get_cols() const { return cols; }

Cell& TermBuffer::at(int r, int c)
{
    if (r < 0 || r >= rows || c < 0 || c >= cols)
        throw std::out_of_range("terminal buffer coordinate");
    return grid[r][c];
}


int TermBuffer::get_cursor_row() const
{
    return cursor_row;
}

int TermBuffer::get_cursor_col() const
{
    return cursor_col;
}

void TermBuffer::set_cursor(int r, int c)
{
    cursor_row = std::clamp(r, 0, rows - 1);
    cursor_col = std::clamp(c, 0, cols - 1);
}

void TermBuffer::scroll_up(int top, int bot)
{
    top = std::clamp(top, 0, rows - 1);
    bot = std::clamp(bot, top, rows - 1);
    std::rotate(grid.begin() + top, grid.begin() + top + 1,
                grid.begin() + bot + 1);
    grid[bot].assign(cols, blank_cell());
}

void TermBuffer::resize(int row_count, int column_count)
{
    row_count = std::max(1, row_count);
    column_count = std::max(1, column_count);
    std::vector<std::vector<Cell>> resized(
        row_count, std::vector<Cell>(column_count, blank_cell()));
    for (int r = 0; r < std::min(rows, row_count); ++r)
        for (int c = 0; c < std::min(cols, column_count); ++c)
            resized[r][c] = grid[r][c];
    rows = row_count;
    cols = column_count;
    grid = std::move(resized);
    set_cursor(cursor_row, cursor_col);
}

void TermBuffer::clear(int r, int c, int row_count, int column_count,
                       short fg, short bg, bool bold)
{
    const int end_row = std::min(rows, r + row_count);
    const int end_col = std::min(cols, c + column_count);
    for (int row = std::max(0, r); row < end_row; ++row)
        for (int col = std::max(0, c); col < end_col; ++col)
            grid[row][col] = blank_cell(fg, bg, bold);
}

void TermBuffer::put(int r, int c, wchar_t ch, short fg, short bg, bool bold)
{
    if (r >= 0 && r < rows && c >= 0 && c < cols)
        grid[r][c] = Cell{ch, fg, bg, bold, true};
}