#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <signal.h>

#include <curses.h>

/*
 * Init and quit
 */

static void init_colors(void);
static void quit(int sig);
static void init(void);
/* 
 * commands
 */

#define DIFF_CMD	\
	"git log --stat -n1 %s ; echo; " \
	"git diff --find-copies-harder -B -C %s^ %s"

#define LOG_CMD	\
	"git log --stat -n100 %s"
/*
 * view
 */
struct view {
	char *name;
	char *cmd;

	/* Rendering */
	int (*render)(struct view *, int);
	WINDOW *win;

	/* Navigation */
	unsigned long offset;	/* Offset of the window top */
	unsigned long lineno;	/* Current line number */

	/* Buffering */
	unsigned long lines;	/* Total number of lines */
	char **line;		/* Line index */

	/* Loading */
	FILE *pipe;
};
static struct view main_view;
struct view *p_main_view = &main_view; // we only need one view for now 
static int  update_view(struct view *view);
static void end_update(struct view *view);
static void scroll_view(struct view *view, int request);
static void redraw_view(struct view *view);
static int default_renderer(struct view *view, int lineno);
static int view_driver(struct view *view, int key);

/* declaration end */

/*
 * Main
 */

int main(int argc, char *argv[])
{
	int x, y;

	init();

	getmaxyx(stdscr, y, x); 
    // this maybe not likes "call by value", it just a macro

	attrset(COLOR_PAIR(COLOR_GREEN));

	addch(ACS_VLINE);
	printw("%s", "cg-view");
	addch(ACS_LTEE);
	addch(ACS_HLINE);
	addch(ACS_HLINE);
	addch(ACS_HLINE);
	addch(ACS_HLINE);
	addch(ACS_HLINE);
	addch(ACS_HLINE);
	addch(ACS_HLINE);
	mvprintw(y - 1, 0, "%s", "press 'q' to quit");

	attrset(A_NORMAL);

    // give some output, do the job of switch_view()
    p_main_view->render = default_renderer;
    p_main_view->pipe = popen("git log", "r");
    p_main_view->win = stdscr;
    update_view(p_main_view);
    int c = getch();     /* refresh, accept single keystroke of input */
	while (view_driver(p_main_view, c)) 
    {

			if (p_main_view->pipe) {
				update_view(p_main_view);
			}

    }

	quit(0);
}

static void quit(int sig)
{
	endwin();
    printf("printf quit\n");

	/* do your non-curses wrapup here */

	exit(0);
}

static void init_colors(void)
{
	start_color();

	init_pair(COLOR_BLACK,	 COLOR_BLACK,	COLOR_BLACK);
	init_pair(COLOR_GREEN,	 COLOR_GREEN,	COLOR_BLACK);
	init_pair(COLOR_RED,	 COLOR_RED,	COLOR_BLACK);
	init_pair(COLOR_CYAN,	 COLOR_CYAN,	COLOR_BLACK);
	init_pair(COLOR_WHITE,	 COLOR_WHITE,	COLOR_BLACK);
	init_pair(COLOR_MAGENTA, COLOR_MAGENTA,	COLOR_BLACK);
	init_pair(COLOR_BLUE,	 COLOR_BLUE,	COLOR_BLACK);
	init_pair(COLOR_YELLOW,	 COLOR_YELLOW,	COLOR_BLACK);
}

static void init(void)
{
    signal(SIGINT, quit);      /* arrange interrupts to terminate */
    // when you <Ctr-c>, SIGINT is sent to this process, and quit() is called

	initscr();      /* initialize the curses library */
	keypad(stdscr, TRUE);  /* enable keyboard mapping */
	nonl();         /* tell curses not to do NL->CR/NL on output */
	cbreak();       /* take input chars one at a time, no wait for \n */
	noecho();       /* don't echo input */
    scrollok(stdscr, TRUE);

	if (has_colors())
    {
		init_colors();
    }
}
static void scroll_view(struct view *view, int request)
{
    int lines = 1;
    int y, x;
	getmaxyx(view->win, y, x);
    switch (request) 
    {
        case 'j':
            lines = 1;
            if (view->offset + lines > view->lines)
                lines = view->lines - view->offset - 1;
            printw("view->lines: %d", view->lines);

            if (lines == 0 || view->offset + y >= view->lines) {
                printw("already at last line");
                return;
            }
            break;
        case 'k':
            lines = -1;
            if (lines > view->offset)
                lines = view->offset;

            if (lines == 0) {
                printw("already at first line");
                return;
            }
            break;
        default:
            lines = 0;
    }
    if (lines == 1) 
    {
        wscrl(stdscr, lines);
        view->render(view, y - lines);
    }
    else if (lines == -1)
    {
    
        wscrl(stdscr, lines);
        view->render(view, 0);
    }
    else 
    {
        printw("sth wrong!!");
    }
	if (view->lineno < view->offset)
		view->lineno = view->offset;
	if (view->lineno > view->offset + y)
		view->lineno = view->offset + y;
	view->offset += lines;
	redrawwin(stdscr);
	wrefresh(stdscr);
}
/** 
* @brief clean up work, after things 
* 
* @param view : the big sturct
*/
static int update_view(struct view *view)
{
	char buffer[BUFSIZ];
	char *line;
	int lines, cols;
	char **tmp;
	int redraw;

	if (!view->pipe)
		return TRUE;

	getmaxyx(view->win, lines, cols);

	redraw = !view->line;

	tmp = realloc(view->line, sizeof(*view->line) * (view->lines + lines));
	if (!tmp)
		goto alloc_error;

	view->line = tmp;

	while ((line = fgets(buffer, sizeof(buffer), view->pipe))) 
    {
		int linelen;
        if (!lines--)
            break;

		linelen = strlen(line);
		if (linelen)
			line[linelen - 1] = 0;

		view->line[view->lines] = strdup(line);
		if (!view->line[view->lines])
			goto alloc_error;
		view->lines++;
	}
    printw("Update_view(): view->lines: %d", view->lines);

	if (redraw)
		redraw_view(view);

	if (ferror(view->pipe)) {
		printw("Failed to read %s", view->cmd);
		goto end;

	} else if (feof(view->pipe)) {
		printw("end of pipe");
		goto end;
	}

	return TRUE;

alloc_error:
	printw("Allocation failure");

end:
	end_update(view);
	return FALSE;
}

static void end_update(struct view *view)
{
	pclose(view->pipe);
	view->pipe = NULL;
}
static void redraw_view(struct view *view)
{
	int lineno;
	int lines, cols;

	wclear(view->win);
	wmove(view->win, 0, 0);

	getmaxyx(view->win, lines, cols);

	for (lineno = 0; lineno < lines; lineno++) {
		view->render(view, lineno);
	}

	redrawwin(view->win);
	wrefresh(view->win);
}
static int default_renderer(struct view *view, int lineno)
{
	char *line;
	int i;

	line = view->line[view->offset + lineno];
	if (!line) return FALSE;

	mvwprintw(view->win, lineno, 0, "%4d: %s", view->offset + lineno, line);

	return TRUE;
}
/* Process a keystroke */
static int view_driver(struct view *view, int key)
{
	switch (key) 
    {
	case 'j':
	case 'k':
		if (view)
			scroll_view(view, key);
		break;

	default:
		return TRUE;
	}

	return TRUE;
}
