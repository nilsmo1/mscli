#include <iostream>
#include <array>
#include <cstdlib>
#include <ctime>
#include <termios.h>
#include <memory>
#include <cstdint>
#include "escapeCodes.h"

#define ROWS 15
#define COLS 17

#define MINE_PROBABILITY_PERCENTAGE 15

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
    std::array<std::unique_ptr<Cell>, ROWS * COLS> container;

    Field() { init(); }

    void init()
    {
        for (size_t i{}; i < ROWS * COLS; ++i)
        {
            if ((std::rand() % 100) + 1 > MINE_PROBABILITY_PERCENTAGE)
            {
                container[i] = std::make_unique<Empty>();
            }
            else
            {
                container[i] = std::make_unique<Mine>();
            }
            container[i]->show();
        }

        for (int i{}; i < ROWS * COLS; ++i)
        {
            const std::array<const int, 8> neigbours =
            {
                i - COLS - 1, i - COLS, i - COLS + 1,
                i - 1,                         i + 1,
                i + COLS - 1, i + COLS, i + COLS + 1,
            };

            for (const int& neighbourIndex : neigbours)
            {
                if (neighbourIndex < 0 || neighbourIndex >= ROWS * COLS) { continue; }
                if (dynamic_cast<const Mine*>(container[neighbourIndex].get()))
                {
                    container[i]->neighbouringMines++;
                }
            }
        }
    }

    void displayField(bool initial = false) const
    {
        for (size_t i{}; i < ROWS * COLS; ++i)
        {
            if (i != 0 && i % COLS == 0) { std::cout << " \n"; }
            std::cout << ' ' << container[i]->symbol();
        }
        std::cout << " \n";
        moveUp(ROWS);
        moveLeft(COLS);
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
        return container[row * COLS + col];
    }
};

std::shared_ptr<termios> setup_window();
void reset_window(const std::shared_ptr<termios>& saved_attributes);

int main() {
    std::shared_ptr<termios> saved_attributes = setup_window();

    std::srand(std::time(nullptr));

    // size_t cursorCol{ (COLS - 1) / 2 }, cursorRow{ (ROWS - 1) / 2 };
    size_t cursorCol{ }, cursorRow{ };

    Field field;
    field.displayField(true);
    field.displayCursor(cursorCol, cursorRow);

    bool goodMove = true;
    bool quit = false;
    char inputKey;
    while (std::cin >> inputKey) {
        switch(inputKey) {
            case 'w': if (cursorRow != 0     ) cursorRow--; break;
            case 's': if (cursorRow <  ROWS-1) cursorRow++ ; break;
            case 'a': if (cursorCol != 0     ) cursorCol-- ; break;
            case 'd': if (cursorCol <  COLS-1) cursorCol++ ; break;
            case 'e': goodMove = field.at(cursorCol, cursorRow)->show(); break;
            case 'f': field.at(cursorCol, cursorRow)->flag(); break;
            case 'q': quit = true;
        }

        if (quit) { break; }
        if (!goodMove) { /*game over*/ }

        field.displayField();
        field.displayCursor(cursorCol, cursorRow);
    }


    reset_window(saved_attributes);

    return 0;
}

std::shared_ptr<termios> setup_window()
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

void reset_window(const std::shared_ptr<termios>& saved_attributes)
{
    showCursor();
    tcsetattr(0, TCSANOW, saved_attributes.get());
}
