#include "homedir.h"
#include "results.h"

#include <ncurses.h>
#include <stdio.h>
#include <termios.h>
#include <signal.h>


bool running = true;
bool win_changed = false;
int numrows = 24;
int numcols = 80;
int cur_row = 0;
int cur_col = 0;

void handle_char(int ch)
{
    if (ch == ('X' & 31)) {
        running = false;
    }
    mvprintw(0, 0, "%d           ", ch);
    move(cur_row, cur_col);
}

void handle_idle()
{
}

void full_repaint()
{
    attron(COLOR_PAIR(1));
    move(cur_row, cur_col);
}

void handle_window_changed()
{
    int oldr = numrows;
    int oldc = numcols;
    getmaxyx(stdscr, numrows, numcols);
    if (oldr != numrows || oldc != numcols) {
        if (cur_row >= numrows) {
            cur_row = numrows-1;
        }
        if (cur_col >= numcols) {
            cur_col = numcols-1;
        }
        full_repaint();
    }
}

struct termios reset_tty_attr;

static void reset_tty()
{
    tcsetattr(0, TCSANOW, &reset_tty_attr);
}

static void sig_int(int)
{
    running = false;
}

static void sig_hup(int)
{
    running = false;
}

static void sig_winch(int)
{
    win_changed = true;
}

void inittty()
{
    struct termios tio;
    if (tcgetattr(0, &tio) < 0) {
        result("stdin is not a terminal", StatusInfo);
        return;
    }
    reset_tty_attr = tio;
    atexit(reset_tty);
    signal(SIGINT, sig_int);
    signal(SIGHUP, sig_hup);
    signal(SIGWINCH, sig_winch);
    cfmakeraw(&tio);
    tio.c_lflag &= ~(ECHO);
    tcsetattr(0, TCSANOW, &tio);
}

void polledtty()
{
    struct termios tio;
    if (tcgetattr(0, &tio) < 0) {
        return;
    }
    tio.c_cc[VMIN] = 0;
    tio.c_cc[VTIME] = 1;
    tcsetattr(0, TCSANOW, &tio);
}

static char const *colortypes[] = {
    "_unused",
    "text",
    "status",
    "browser",
    "info",
    "warning",
    "error",
};
static short colorvalues[][2][3] = {
    { { 0, 0, 400, },     { 800, 800, 800, } },
    { { 800, 800, 800, }, { 0, 0, 400, } },
    { { 600, 600, 300, }, { 200, 200, 200, } },
    { { 600, 600, 999, }, { 200, 200, 200, } },
    { { 600, 600, 600, }, { 200, 200, 200, } },
    { { 800, 800, 300, }, { 200, 200, 200, } },
    { { 999, 700, 500, }, { 200, 200, 200, } },
};

void apply_colors_from_settings()
{
    if (COLORS < 16 || COLOR_PAIRS < 16) {
        return;
    }
    for (size_t i = 0; i != sizeof(colortypes)/sizeof(colortypes[0]); ++i) {
        std::string s(colortypes[i]);
        s = "color." + s;
        char const *val = get_setting(s.c_str(), nullptr);
        if (val) {
            int fr = colorvalues[i][0][0];
            int fg = colorvalues[i][0][1];
            int fb = colorvalues[i][0][2];
            int br = colorvalues[i][1][0];
            int bg = colorvalues[i][1][1];
            int bb = colorvalues[i][1][2];
            sscanf(val, "%d,%d,%d,%d,%d,%d", &fr, &fg, &fb, &br, &bg, &bb);
            colorvalues[i][0][0] = fr;
            colorvalues[i][0][1] = fg;
            colorvalues[i][0][2] = fb;
            colorvalues[i][1][0] = br;
            colorvalues[i][1][1] = bg;
            colorvalues[i][1][2] = bb;
            if (i == 1) {
                colorvalues[0][0][0] = br;
                colorvalues[0][0][1] = bg;
                colorvalues[0][0][2] = bb;
                colorvalues[0][1][0] = fr;
                colorvalues[0][1][1] = fg;
                colorvalues[0][1][2] = fb;
            }
        }
        init_color(i*2, colorvalues[i][0][0], colorvalues[i][0][1], colorvalues[i][0][2]);
        init_color(i*2+1, colorvalues[i][1][0], colorvalues[i][1][1], colorvalues[i][1][2]);
        init_pair(i, i*2, i*2+1);
    }
}

int main(int argc, char const *argv[])
{
    read_settings("startup");
    inittty();
    initscr();
    start_color();
    noecho();
    keypad(stdscr, TRUE);
    raw();
    polledtty();
    apply_colors_from_settings();
    full_repaint();
    while (running) {
        std::vector<std::pair<Status, std::string> > res;
        result_harvest(res);
        refresh();
        int ch = getch();
        if (win_changed) {
            win_changed = false;
            handle_window_changed();
        }
        if (ch != -1) {
            handle_char(ch);
        }
        handle_idle();
    }
    echo();
    endwin();
    return 0;
}

