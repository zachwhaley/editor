#include "buffer.hpp"

#include <fstream>
#include <sstream>

using namespace std;

namespace eli {

Buffer::~Buffer()
{
    if (m_textwin) {
        delwin(m_textwin);
    }
    if (m_titlewin) {
        delwin(m_titlewin);
    }
}

void
Buffer::read(const char *filename)
{
    m_filename = filename;
    m_lines.clear();

    ifstream ifs(m_filename, ios::in);
    if (ifs) {
        string line;
        while (getline(ifs, line)) {
            m_lines.push_back(line);
        }
    }
    else {
        m_lines.push_back(string());
    }
}

void
Buffer::write() const
{
    ofstream ofs(m_filename, ios::out | ios::trunc);
    for (string line : m_lines) {
        ofs << line << endl;
    }
}

int
Buffer::getchar() const
{
    return wgetch(m_textwin);
}

void
Buffer::update(int ch)
{
    switch (ch) {
    case KEY_UP:
        prevline();
        break;
    case KEY_DOWN:
        nextline();
        break;
    case KEY_RIGHT:
        nextchar();
        break;
    case KEY_LEFT:
        prevchar();
        break;
    case KEY_HOME:
        begofline();
        break;
    case KEY_END:
        endofline();
        break;
    case KEY_BACKSPACE:
        delchar();
        break;
    case '\n':
    case '\r':
        newline();
        break;
    default:
        addchar(ch);
    }
}

void
Buffer::display()
{
    initwindows();
    const size_t cols = COLS, lines = LINES;

    // Refresh title window
    wmove(m_titlewin, 0, 0);
    for (size_t col = 0; col < cols; col++) {
        waddch(m_titlewin, ' ');
    }
    stringstream ss;
    ss << " " << m_filename << " " << cursorY << "," << m_cur.x;
    string title = ss.str();
    mvwaddstr(m_titlewin, 0, 0, title.c_str());
    wnoutrefresh(m_titlewin);

    // Refresh text window
    int scroll_factor = 0;
    for (size_t line = 0; line < lines - 1; line++) {
        wmove(m_textwin, line, 0);
        for (size_t col = 0; col < cols; col++) {
            if( cursorY  > lines - 2) {
	        m_cur.y = lines - 2;
	        scroll_factor = cursorY - lines + 2 ;
	    }
            else {
	        scroll_factor = 0;
	    }
            if (line+scroll_factor < m_lines.size() && col < m_lines[line+scroll_factor].length())
                waddch(m_textwin, m_lines[ line + scroll_factor ][col]);
            else
                waddch(m_textwin, ' ');
        }
    }
    wnoutrefresh(m_textwin);
    wmove(m_textwin, m_cur.y, m_cur.x);
    doupdate();
}

void
Buffer::initwindows()
{
    const int cols = COLS, lines = LINES;
    if (!m_titlewin) {
        m_titlewin = newwin(1, cols, lines - 1, 0);
        wattron(m_titlewin, A_REVERSE);
    }
    if (!m_textwin) {
        m_textwin = newwin(lines - 1, cols, 0, 0);
        keypad(m_textwin, true);
        scrollok(m_textwin, false);
    }
}

void
Buffer::begofline()
{
    m_cur.x = 0;
}

void
Buffer::endofline()
{
    m_cur.x = m_lines[cursorY].length();
}

void
Buffer::nextline()
{
    if (m_cur.y != m_lines.size() - 1) {
        m_cur.y++;
    }
    if (m_cur.x > m_lines[cursorY].length()) {
        endofline();
    }
    if (cursorY != m_lines.size() - 1) {
        cursorY++;
    }
}

void
Buffer::prevline()
{
    if (m_cur.y != 0) {
        m_cur.y--;
    }
    if (m_cur.x > m_lines[cursorY].length()) {
        endofline();
    }
    if (cursorY != 0) {
        cursorY--;
    }
}

void
Buffer::nextchar()
{
    if (m_cur.x != m_lines[cursorY].length()) {
        m_cur.x++;
    }
    else if (cursorY != m_lines.size() - 1) {
        m_cur.y++;
        cursorY++;
        begofline();
    }
}

void
Buffer::prevchar()
{
    if (m_cur.x != 0) {
        m_cur.x--;
    }
    else if (cursorY != 0) {
        m_cur.y--;
        cursorY--;
        endofline();
    }
}

void
Buffer::addchar(char ch)
{
    m_lines[cursorY].insert(m_cur.x, 1, ch);
    nextchar();
}

void
Buffer::delchar()
{
    if (m_cur.x != 0) {
        m_lines[cursorY].erase(m_cur.x - 1, 1);
        prevchar();
    }
    else if (m_cur.y != 0) {
        string old_line = m_lines[cursorY];
        m_lines.erase(m_lines.begin() + m_cur.y);
        prevchar();
        // Append what was left of the erased line to the now current line
        m_lines[cursorY] += old_line;
    }
}

void
Buffer::newline()
{
    string new_line = m_lines[cursorY].substr(m_cur.x);
    m_lines[cursorY].erase(m_cur.x);
    auto new_pos = m_lines.begin() + m_cur.y + 1;
    m_lines.insert(new_pos, new_line);
    nextchar();
}

} // namespace eli
