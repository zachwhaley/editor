#include "editor.hpp"

#include <ncurses.h>

#define CTRL(c) ((c) & 037)

using namespace std;

namespace eli {

Editor::Editor()
{
    initscr();
    cbreak();
    noecho();
    m_buffer.cursorY = 0;
}

Editor::~Editor()
{
    endwin();
}

void
Editor::open(const char *filename)
{
    m_buffer.read(filename);
}

void
Editor::edit()
{
    int ch;

    while (true) {
        m_buffer.display();

        ch = m_buffer.getchar();
        if (ch == KEY_F(12)) {
            return;
        }
        else if (ch == CTRL('w')) {
            m_buffer.write();
        }
	else if (ch == KEY_RESIZE ) {
        }
        else {
            m_buffer.update(ch);
        }
    }
}

} // namespace eli
