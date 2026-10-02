#include <iostream>
#include <array>
#include <cstdlib>
#include <ctime>
#include <termios.h>
#include <memory>
#include <cstdint>
#include <vector>
#include "escapeCodes.h"

#define ROWS 15
#define COLS 17

#define MINE_PROBABILITY_PERCENTAGE 15

// NEIGHBOURS N
// max neighbours = 0b1000
//
// TYPE T
// empty = 0b0
// mine  = 0b1
//
// STATE S
// hidden  = 0b00
// flagged = 0b01
// shown   = 0b10
//
// cell = 0b 0 NNNN T SS

std::array<std::array<uint8_t, COLS>, ROWS> f;
void init() {
    for (size_t r{}; r < ROWS; ++r) {
        for (size_t c{}; c < COLS; ++c) {
            const auto roll = (std::rand() % 100) + 1 > MINE_PROBABILITY_PERCENTAGE;
            f[r][c] = roll ? 0b0 : 0b100;
        }
    }
}
char symbol(uint8_t cell) {
    const auto n = (cell >> 3) % 16;
    const auto t = (cell >> 2) % 2;
    const auto s = cell % 4;
    switch (s) {
        case 0: return '.';
        case 1: return '?';
        case 2: return t ? '@' : (n > 0 ? (char)('0' + n) : ' ');
    }
    return '!';
};
void display(uint8_t r, uint8_t c, bool goUp = true) {
    for (size_t rr{}; rr < ROWS; ++rr) {
        for (size_t cc{}; cc < COLS; ++cc) {
            if (r == rr && c == cc) {
                std::cout << '(' << symbol(f[rr][cc]) << ')';
            } else if (r == rr && c + 1 == cc) {
                std::cout << symbol(f[rr][cc]);
            } else {
                std::cout << ' ' << symbol(f[rr][cc]);
            }
        }
        std::cout << " \n";
    }
    if (goUp) { moveUpToBeginningOfLine(ROWS); }
}
bool open(uint8_t r, uint8_t c) {
    const auto n = (f[r][c] >> 3) % 16;
    const auto t = (f[r][c] >> 2) % 2;
    const auto s = f[r][c] % 4;

    if (s > 0) { return false; }

    f[r][c] = ((f[r][c] >> 2) << 2) + 2;
    if (t) { return true; }

    // expand here

    return false;
}

std::shared_ptr<termios> setupWindow();
void resetWindow(const std::shared_ptr<termios>& savedAttributes, const bool clearField);

int main() {
    std::shared_ptr<termios> savedAttributes = setupWindow();

    std::srand(std::time(nullptr));

    uint8_t r{ (ROWS - 1) / 2 };
    uint8_t c{ (COLS - 1) / 2 };

    init();
    display(r, c);

    bool quit = false;
    char inputKey;
    while (std::cin >> inputKey)
    {
        const auto n = (f[r][c] >> 3) % 16;
        const auto t = (f[r][c] >> 2) % 2;
        const auto s = f[r][c] % 4;

        switch (inputKey)
        {
        case 'w': if (r != 0     ) r--; break;
        case 's': if (r <  ROWS-1) r++ ; break;
        case 'a': if (c != 0     ) c-- ; break;
        case 'd': if (c <  COLS-1) c++ ; break;
        case 'e': quit = open(r, c); break;
        case 'f': f[r][c] = ((f[r][c] >> 2) << 2) + 1; break;
        case 'q': quit = true; break;
        }

        if (quit)
        {
            // showAllMines();
            display(r, c, false);
            break;
        }

        // if (field.allMinesFlagged() &&
        //     field.noEmptiesFlagged())
        // {
        //     field.displayField(true);
        //     std::cout << "\nYou win! You found all " << field.mineCount << " mines!\n";
        //     break;
        // }

        display(r, c);
    }

    resetWindow(savedAttributes, false);

    return 0;
}

std::shared_ptr<termios> setupWindow()
{
    termios tattr;
    termios saved_attributes;

    tcgetattr(0, &saved_attributes);
    tcgetattr(0, &tattr);
    tattr.c_lflag &= ~(ICANON|ECHO);
    tattr.c_cc[VMIN] = 1;
    tattr.c_cc[VTIME] = 0;
    tcsetattr(0, TCSAFLUSH, &tattr);

    hideCursor();

    return std::make_shared<termios>(saved_attributes);
}

void resetWindow(const std::shared_ptr<termios>& saved_attributes, const bool clearField)
{
    if (clearField)
    {
        std::cin.get();
        moveUpToBeginningOfLine(ROWS + 2);
    }
    showCursor();
    tcsetattr(0, TCSANOW, saved_attributes.get());
}
