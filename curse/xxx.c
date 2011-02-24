#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <signal.h>
#include <assert.h>

#include <curses.h>

/*
 * Init and quit
 */

static WINDOW *status_win;
static void init_colors(void);
static void quit(int sig);
static void init(void);
/* 
 * commands
 */

#define FIND_CMD "find ."	
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
static void report(const char *msg, ...);
static void report_position(struct view *view, int all);
/* declaration end */

/*
 * Main
 */

int main(int argc, char *argv[])
{

	init();
    update_view(p_main_view);
    update_view(p_main_view);
    int c = 0;
	while (view_driver(p_main_view, c)) 
    {

			if (p_main_view->pipe) {
				update_view(p_main_view);
			}
            c = wgetch(status_win);     /* refresh, accept single keystroke of input */

    }

	quit(0);
}

static void quit(int sig)
{
	if (status_win)
		delwin(status_win);
	endwin();
	/* do your non-curses wrapup here */

	exit(0);
}

static void init_colors(void)
{
	start_color();

	init_pair(COLOR_BLACK,	 COLOR_BLACK,	COLOR_BLACK);
	init_pair(COLOR_GREEN,	 COLOR_WHITE,	COLOR_GREEN);
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
	nonl();         /* tell curses not to do NL->CR/NL on output */
	cbreak();       /* take input chars one at a time, no wait for \n */
	noecho();       /* don't echo input */

	if (has_colors())
    {
		init_colors();
    }
	int x, y;
    getmaxyx(stdscr, y, x);
	status_win = newwin(1, 0, y - 2, 0);
	wattrset(status_win, COLOR_PAIR(COLOR_GREEN));

    // give some output, do the job of switch_view()
    p_main_view->render = default_renderer;
    p_main_view->pipe = popen(FIND_CMD, "r");
    p_main_view->win = newwin( y- 2, 0, 0, 0);
    scrollok(p_main_view->win, TRUE);
	keypad(p_main_view->win, TRUE);  /* enable keyboard mapping */
}
static void scroll_view(struct view *view, int request)
{
	int x, y, lines = 1;
	enum { BACKWARD = -1,  FORWARD = 1 } direction = FORWARD;
	getmaxyx(view->win, y, x);
    switch (request) 
    {
        case 'j':
            if (view->offset + lines > view->lines)
                lines = view->lines - view->offset - 1;

            if (lines == 0 || view->offset + y >= view->lines) {
                report("already at last line");
                return;
            }
            break;
        case 'k':
            if (lines > view->offset)
                lines = view->offset;

            if (lines == 0) {
                report("already at first line");
                return;
            }
		    direction = BACKWARD;
            break;
        default:
            lines = 0;
    }
	view->offset += lines * direction;

	/* Move current line into the view. */
	if (view->lineno < view->offset)
		view->lineno = view->offset;
	if (view->lineno > view->offset + y)
		view->lineno = view->offset + y;

	assert(0 <= view->offset && view->offset < view->lines);
	//assert(0 <= view->offset + lines && view->offset + lines < view->lines);
	assert(view->offset <= view->lineno && view->lineno <= view->lines);

	if (lines) {
		int from = direction == FORWARD ? y - lines : 0;
		int to	 = from + lines;

		wscrl(view->win, lines * direction);

		for (; from < to; from++) {
			if (!view->render(view, from))
				break;
		}
	}
	redrawwin(view->win);
	wrefresh(view->win);
	report_position(view, lines);
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

	if (redraw)
		redraw_view(view);

	if (ferror(view->pipe)) {
		printw("Failed to read %s", view->cmd);
		goto end;

	} else if (feof(view->pipe)) {
	    report_position(view, lines);
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

	mvwprintw(view->win, lineno, 0, "%4d--%d---offset:%d: %s", view->offset + lineno, view->lines,view->offset, line);

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
	case 'q':
        quit(0);
        break;

	default:
		return TRUE;
	}

	return TRUE;
}
static void report(const char *msg, ...)
{
	va_list args;

	va_start(args, msg);

	werase(status_win);
	wmove(status_win, 0, 0);

	vwprintw(status_win, msg, args);
	wrefresh(status_win);

	va_end(args);
}
static void report_position(struct view *view, int all)
{
    report(all ? "line %d of %d (%d%%) viewing from %d"
             : "line %d of %d",
           view->lineno + 1,
           view->lines,
           view->lines ? view->offset * 100 / view->lines : 0,
           view->offset);
}

