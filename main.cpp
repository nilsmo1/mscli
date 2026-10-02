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

#define MINE_PROBABILITY_PERCENTAGE 0

enum struct State { Hidden, Flagged, Shown };

struct Cell
{
    State state = State::Hidden;
    int8_t neighbouringMines = 0;

    virtual std::string symbol() const = 0;
    virtual bool show() = 0;
    void setState(const State& s) { state = s; }
    void flag()
    {
        if      (state == State::Flagged) { setState(State::Hidden);  }
        else if (state == State::Hidden ) { setState(State::Flagged); }
    }
    bool isShown()
    {
        return state == State::Shown;
    }
    bool isHidden()
    {
        return state == State::Hidden;
    }

};

struct Mine : Cell
{
    std::string symbol() const override
    {
        switch (state)
        {
        case State::Hidden: return ".";
        case State::Flagged: return "?";
        case State::Shown: return "@";
        default: return "!";
        }
    }

    bool show() override
    {
        if (state != State::Hidden) { return true; }
        setState(State::Shown);
        return false;
    }
};

struct Empty : Cell
{
    std::string numberOrEmpty() const
    {
        return neighbouringMines > 0
            ? std::to_string(neighbouringMines)
            : " ";
    }

    std::string symbol() const override
    {
        switch (state)
        {
        case State::Hidden: return ".";
        case State::Flagged: return "?";
        case State::Shown: return numberOrEmpty();
        default: return "!";
        }
    }

    bool show() override
    {
        if (state != State::Hidden) { return true; }
        setState(State::Shown);
        return true;
    }
};

struct Field
{
    std::array<std::array<std::unique_ptr<Cell>, COLS>, ROWS> container;
    size_t mineCount = 0;

    Field() { init(); }

    void init()
    {
        for (size_t row{}; row < ROWS; ++row)
        {
            for (size_t col{}; col < COLS; ++col)
            {
                if ((std::rand() % 100) + 1 > MINE_PROBABILITY_PERCENTAGE)
                {
                    container[row][col] = std::make_unique<Empty>();
                }
                else
                {
                    container[row][col] = std::make_unique<Mine>();
                    mineCount++;
                }
            }
        }

        for (int row{}; row < ROWS; ++row)
        {
            for (int col{}; col < COLS; ++col)
            {
                const std::array<std::array<int, 2>, 8> neigbours =
                {{
                    { row - 1, col - 1 }, { row - 1, col }, { row - 1, col + 1 },
                    { row    , col - 1 },                   { row    , col + 1 },
                    { row + 1, col - 1 }, { row + 1, col }, { row + 1, col + 1 },
                }};

                for (const auto& [r, c] : neigbours)
                {
                    if (r < 0 || c < 0 || r >= ROWS || c >= COLS) { continue; }
                    if (dynamic_cast<const Mine*>(container[r][c].get()))
                    {
                        container[row][col]->neighbouringMines++;
                    }
                }
            }
        }
    }

    void displayField(bool gameOver = false) const
    {
        for (size_t row{}; row < ROWS; ++row)
        {
            for (size_t col{}; col < COLS; ++col)
            {
                std::cout << ' ' << container[row][col]->symbol();
            }
            std::cout << " \n";
        }

        if (!gameOver)
        {
            moveUp(ROWS);
            moveLeft(COLS);
        }
    }

    void displayCursor(const size_t& col, const size_t& row)
    {
        moveDown(row);
        moveRight(2 * col);
        highlightCell();
        moveUp(row);
        moveLeft(2 * col + 3); // Moving back left, accounting for the extra characters placed
    }

    std::unique_ptr<Cell>& at(const size_t& col, const size_t& row)
    {
        return container[row][col];
    }

    bool showCell(int col, int row)
    {
        if (!at(col, row)->isHidden()) { return true; } // Should not open flagged or shown cells

        if (dynamic_cast<const Empty*>(at(col, row).get()) &&
            at(col, row)->neighbouringMines == 0)
        {
            auto neigbours = [](int row, int col)
            {
                return std::array<std::array<int, 2>, 8>
                {{
                    { row - 1, col - 1 }, { row - 1, col }, { row - 1, col + 1 },
                    { row    , col - 1 },                   { row    , col + 1 },
                    { row + 1, col - 1 }, { row + 1, col }, { row + 1, col + 1 },
                }};
            };

            std::vector<std::array<int, 2>> queue;
            queue.push_back({ row, col });

            while (!queue.empty())
            {
                const auto& [r, c] = queue.back();
                queue.pop_back();

                for (const auto& [rr, cc] : neigbours(r, c))
                {
                    if (rr < 0 || cc < 0 || rr >= ROWS || cc >= COLS) { continue; }
                    if (container[rr][cc]->isShown()) { continue; }

                    container[rr][cc]->show();
                    if (container[rr][cc]->neighbouringMines == 0)
                    {
                        queue.push_back({ rr, cc });
                    }
                }
            }
        }

        return at(col, row)->show();
    }

    bool allMinesFlagged()
    {
        for (const auto& row : container)
        {
            for (auto& cell : row)
            {
                if (dynamic_cast<const Mine*>(cell.get()) &&
                    cell->isHidden())
                {
                    return false;
                }
            }
        }

        return true;
    }

    void showAllMines()
    {
        for (const auto& row : container)
        {
            for (auto& cell : row)
            {
                if (dynamic_cast<const Mine*>(cell.get()))
                {
                    cell->setState(State::Shown);
                }
            }
        }
    }
};

std::shared_ptr<termios> setupWindow();
void resetWindow(const std::shared_ptr<termios>& savedAttributes, const bool clearField);

int main() {
    std::shared_ptr<termios> savedAttributes = setupWindow();

    std::srand(std::time(nullptr));

    // size_t cursorCol{ (COLS - 1) / 2 }, cursorRow{ (ROWS - 1) / 2 };
    size_t cursorCol{ }, cursorRow{ };

    Field field;
    field.displayField();
    field.displayCursor(cursorCol, cursorRow);

    bool goodMove = true;
    bool quit = false;
    char inputKey;
    while (std::cin >> inputKey)
    {
        switch(inputKey)
        {
        case 'w': if (cursorRow != 0     ) cursorRow--; break;
        case 's': if (cursorRow <  ROWS-1) cursorRow++ ; break;
        case 'a': if (cursorCol != 0     ) cursorCol-- ; break;
        case 'd': if (cursorCol <  COLS-1) cursorCol++ ; break;
        case 'e': goodMove = field.showCell(cursorCol, cursorRow); break;
        case 'f': field.at(cursorCol, cursorRow)->flag(); break;
        case 'q': quit = true;
        }

        if (quit) { break; }
        if (!goodMove)
        {
            field.showAllMines();
            field.displayField(true);
            std::cout << "\nYou lose...\n";
            break;
        }

        if (field.allMinesFlagged())
        {
            field.displayField(true);
            std::cout << "\nYou win! You found all " << field.mineCount << " mines!\n";
            break;
        }

        field.displayField();
        field.displayCursor(cursorCol, cursorRow);
    }

    resetWindow(savedAttributes, true);

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
