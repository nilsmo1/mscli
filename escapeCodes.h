#pragma once

#include <iostream>

inline void moveUpToBeginningOfLine(const size_t& n)
{
    if (n < 1) { return; }
    std::cout << "\e[" << n << 'F';
}

inline void moveDownToBeginningOfLine(const size_t& n)
{
    if (n < 1) { return; }
    std::cout << "\e[" << n << 'E';
}

inline void moveUp(const size_t& n)
{
    if (n < 1) { return; }
    std::cout << "\e[" << n << 'A';
}

inline void moveDown(const size_t& n)
{
    if (n < 1) { return; }
    std::cout << "\e[" << n << 'B';
}

inline void moveRight(const size_t& n)
{
    if (n < 1) { return; }
    std::cout << "\e[" << n << 'C';
}

inline void moveLeft(const size_t& n)
{
    if (n < 1) { return; }
    std::cout << "\e[" << n << 'D';
}

inline void highlightCell()
{
    std::cout << '(';
    moveRight(1);
    std::cout << ')';
}

inline void hideCursor()
{
    std::cout << "\e[?25l";
}

inline void showCursor()
{
    std::cout << "\e[?25h";
}

