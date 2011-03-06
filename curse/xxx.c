#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <signal.h>
#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <unistd.h>

#include <curses.h>

static void die(const char *err, ...);
static void quit(int sig);
static void report(const char *msg, ...);
static void init_colors(void);
static void init(void);

// annoying is the "\\" escape sign, why double? C string needs one for "\" and
// bash needs another for "(", "!" and ")"
#define FIND_CMD \
"find . -type d -name .git -prune -o \\( \\! -name *.swp \\) -exec grep -in %s {} +"

#define COLOR_DEFAULT  (-1)
#define ABS(x) ((x) >=0 ? (x) : -(x))
#define MIN(x) ((x) <= (y) ? (x) : (y))
#define ARRAY_SIZE(x)   (sizeof(x) / sizeof(x[0]))
#define VIM_CMD  "vim +%s %s"

/*
 * String helpers
 */
static inline void
string_ncopy(char *dst, const char *src, int dstlen)
{
	strncpy(dst, src, dstlen - 1);
	dst[dstlen - 1] = 0;

}

/* Shorthand for safely copying into a fixed buffer. */
#define string_copy(dst, src) \
	string_ncopy(dst, src, sizeof(dst))

struct view {
	char *name;

	/* Rendering */
    bool (*read)(struct view *view, char *line);
    bool (*render)(struct view *view, unsigned int lineno);
	WINDOW *win;
	WINDOW *title;
	int height, width;

	/* Navigation */
	unsigned long offset;	/* Offset of the window top */
	unsigned long lineno;	/* Current line number */

	/* Buffering */
	unsigned long lines;	/* Total number of lines */
	char **line;		/* Line index */
    char *cmd;

	/* Loading */
	FILE *pipe;
};

static int view_driver(struct view *view, int key);
static int  update_view(struct view *view);
static bool begin_update(struct view *view);
static void end_update(struct view *view);
static void redraw_view_from(struct view *view, int lineno);
static bool default_read(struct view *view, char *line);
static bool default_render(struct view *view, unsigned int lineno);
static void report_position(struct view *view, int all);
static void navigate_view(struct view *view, int request);
static void move_view(struct view *view, int lines);
static void update_title_win(struct view *view);
static void open_view(struct view *prev);
/* declaration end */


static struct view main_view = {
    "main", 
    default_read, 
    default_render,
};

/* The display array of active views and the index of the current view. */
static struct view *display[2];
static unsigned int current_view;

#define foreach_view(view, i) \
    for (i = 0; i < ARRAY_SIZE(display) && (view = display[i]); i++)

static bool cursed = false;
static WINDOW *status_win; 
static char fmt_cmd[BUFSIZ];
static char vim_cmd[BUFSIZ];
/*
 * Line-oriented content detection.
 */

#define LINE_INFO \
/*   Line type     String    Foreground     Background    Attributes
 *   ---------     -------   ----------     ----------    ---------- */ \
/* UI colors */ \
LINE(DEFAULT,       "",     COLOR_DEFAULT,  COLOR_DEFAULT,  A_NORMAL), \
LINE(CURSOR,        "",     COLOR_WHITE,    COLOR_GREEN,    A_BOLD), \
LINE(STATUS,        "",     COLOR_GREEN,    COLOR_DEFAULT,  0), \
LINE(TITLE_BLUR,    "",     COLOR_WHITE,    COLOR_BLUE,     0), \
LINE(TITLE_FOCUS,   "",     COLOR_WHITE,    COLOR_BLUE,     A_BOLD), \
LINE(FILE_NAME,     "",     COLOR_BLUE,     COLOR_DEFAULT,  0), \
LINE(FILE_LINUM,    "",     COLOR_GREEN,    COLOR_DEFAULT,  0), \
LINE(FILE_LINCON,   "",     COLOR_DEFAULT,  COLOR_DEFAULT,  0), \
LINE(FILE_DELIM,    "",     COLOR_MAGENTA,  COLOR_DEFAULT,  0),

enum line_type {
#define LINE(type, line, fg, bg, attr) \
    LINE_##type
    LINE_INFO
#undef  LINE
};

struct line_info {
    const char *line;   /* The start of line to match. */
    int linelen;        /* Size of string to match. */
    int fg, bg, attr;   /* Color and text attributes for the lines. */
};

static struct line_info line_info[] = {
#define LINE(type, line, fg, bg, attr) \
    { (line), sizeof(line), (fg), (bg), (attr) }
    LINE_INFO
#undef  LINE
};

int main(int argc, char *argv[])
{
    char c = 'm';
    char buf[BUFSIZ];

    struct view *view;
    if (argc < 2) {
        printf("Usage: %s <an argument>\n", argv[0]);
        return;
    }
    snprintf(buf, sizeof(buf), FIND_CMD, argv[1]);
    string_copy(fmt_cmd, buf);
    
	init();
            
	while (view_driver(display[current_view], c)) 
    {
        int i;

        foreach_view (view, i)
            update_view(view);

        c = wgetch(status_win);     
    }

	quit(0);

    return 0;
}

#if __GNUC__ >= 3
#define __NORETURN __attribute__((__noreturn__))
#else
#define __NORETURN
#endif

static void __NORETURN quit(int sig)
{
    /* XXX: Restore tty modes and let the OS cleanup the rest! */
    if (cursed)
        endwin();
    exit(0);
}

static void __NORETURN die(const char *err, ...)
{
    va_list args;

    endwin();

    va_start(args, err);
    fputs("xxx: ", stderr);
    vfprintf(stderr, err, args);
    fputs("\n", stderr);
    va_end(args);

    exit(1);
}

static bool begin_update(struct view *view)
{
    if (view->pipe)
        end_update(view);
    else {
        view->cmd = fmt_cmd;
        view->pipe = popen(view->cmd, "r");
    }

    if (!view->pipe)
        return false;

    view->offset = 0;
    view->line = 0;
    view->lines = 0;

    return true;
}

static enum line_type get_line_type(char *line)
{
    int linelen = strlen(line);
    enum line_type type;

    for (type = 0; type < ARRAY_SIZE(line_info); type++)
        /* Case insensitive search matches Signed-off-by lines better. */
        if (linelen >= line_info[type].linelen &&
            !strncasecmp(line_info[type].line, line, line_info[type].linelen))
            return type;

    return LINE_DEFAULT;
}

static inline int get_line_attr(enum line_type type)
{
    assert(type < ARRAY_SIZE(line_info));
    return COLOR_PAIR(type) | line_info[type].attr;
}

static void init_colors(void)
{
    int default_bg = COLOR_BLACK;
    int default_fg = COLOR_WHITE;
    enum line_type type;

    start_color();

    if (use_default_colors() != ERR) {
        default_bg = -1;
        default_fg = -1;
    }

    for (type = 0; type < ARRAY_SIZE(line_info); type++) {
        struct line_info *info = &line_info[type];
        int bg = info->bg == COLOR_DEFAULT ? default_bg : info->bg;
        int fg = info->fg == COLOR_DEFAULT ? default_fg : info->fg;

        init_pair(type, fg, bg);
    }
}

static void init(void)
{
    int x, y;

    /* Initialize the curses library */
    if (isatty(STDIN_FILENO)) {
        cursed = !!initscr();
    } else {
    /* Leave stdin and stdout alone when acting as a pager. */
        FILE *io = fopen("/dev/tty", "r+");

        cursed = !!newterm(NULL, io, io);
    }

    if (!cursed)
        die("Failed to initialize curses");

	nonl();         /* tell curses not to do NL->CR/NL on output */
	cbreak();       /* take input chars one at a time, no wait for \n */
	noecho();       /* don't echo input */
    leaveok(stdscr, true);

	if (has_colors())
    {
		init_colors();
    }

    getmaxyx(stdscr, y, x);

    status_win = newwin(1, 0, y - 1, 0);
    if (!status_win)
        die("failed to create status window");

    keypad(status_win, true);
    wbkgdset(status_win, get_line_attr(LINE_STATUS));

}


static void update_title_win(struct view *view)
{
    if (view == display[current_view])
        wbkgdset(view->title, get_line_attr(LINE_TITLE_FOCUS));

    werase(view->title);
    wmove(view->title, 0, 0);

    if (view->lines) { 
        wprintw(view->title, "line %d of %d (%d%%)",
            view->lineno + 1,
            view->lines,
            (view->lineno + 1) * 100 / view->lines);
    }

    wrefresh(view->title);
}

static void resize_display(void)
{
    int offset, i;
    struct view *base = display[0];
    struct view *view = display[1] ? display[1] : display[0];

    /* Setup window dimensions */

    getmaxyx(stdscr, base->height, base->width);

    /* Make room for the status window. */
    base->height -= 1;

    /* Make room for the title bar. */
    base->height -= 1;

    offset = 0;

    foreach_view (view, i) {
        /* Keep the height of all view->win windows one larger than is
         * required so that the cursor can wrap-around on the last line
         * without scrolling the window. */
        if (!view->win) {
            view->win = newwin(view->height + 1, 0, offset, 0);
            if (!view->win)
                die("Failed to create %s view", view->name);

            scrollok(view->win, TRUE);

            view->title = newwin(1, 0, offset + view->height, 0);
            if (!view->title)
                die("Failed to create title window");

        } else {
            wresize(view->win, view->height + 1, view->width);
            mvwin(view->win,   offset, 0);
            mvwin(view->title, offset + view->height, 0);
            wrefresh(view->win);
        }

        offset += view->height + 1;
    }
}

static int update_view(struct view *view)
{
	char buffer[BUFSIZ];
	char *line;
	char **tmp;
	int redraw_from = -1;
	unsigned long lines = view->height;

	if (!view->pipe)
		return TRUE;

	/* Only redraw if lines are visible. */
	if (view->offset + view->height >= view->lines)
		redraw_from = view->lines - view->offset;

	tmp = realloc(view->line, sizeof(*view->line) * (view->lines + lines));
	if (!tmp)
		goto alloc_error;

	view->line = tmp;

	while ((line = fgets(buffer, sizeof(buffer), view->pipe))) 
    {
		int linelen;
		linelen = strlen(line);

		if (linelen)
			line[linelen - 1] = 0;

		if (!view->read(view, line))
			goto alloc_error;

		if (lines-- == 1)
			break;
	}

	if (redraw_from >= 0) {
		/* If this is an incremental update, redraw the previous line
		 * since for commits some members could have changed when
		 * loading the main view. */
		if (redraw_from > 0)
			redraw_from--;

		/* Incrementally draw avoids flickering. */
		redraw_view_from(view, redraw_from);
	}

	update_title_win(view);

	if (ferror(view->pipe)) {
		printw("Failed to read %s", view->cmd);
		goto end;

	} else if (feof(view->pipe)) {
        report("load %d lines", view->lines);
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

static void redraw_view_from(struct view *view, int lineno)
{
	assert(0 <= lineno && lineno < view->height);

	for (; lineno < view->height; lineno++) {
		view->render(view, lineno);
	}

	redrawwin(view->win);
	wrefresh(view->win);
}

struct fileinfo {
	char name[65];		
	char content[85];
    char number[5];
};

static char word[BUFSIZ];

char *strsplit(const char *line, const char c)
{
    int i = 0;
    while (*line != c) {
        word[i++] = *line;
        line++;
    }
    word[i] = '\0'; 
    return word;
}

static bool default_read(struct view *view, char *line)
{
	struct fileinfo *fileinfo;
    char *top = "Binary file";
    char *end;

    fileinfo= calloc(1, sizeof(struct fileinfo));
    if (!fileinfo)
        return false;

    if(!strncmp(line, top, strlen(top))) 
            return true;

    line += 2;
    view->line[view->lines++] = fileinfo;
    string_copy(fileinfo->name, strsplit(line, ':'));

	end = strchr(line, ':');
    end += 1;
    string_copy(fileinfo->number, strsplit(end, ':'));

    end = strchr(end, ':');
    end += 1;
    while (*end == ' ') 
        end++;
    string_copy(fileinfo->content, end);

    return true;
}
  
static bool default_render(struct view *view, unsigned int lineno)
{
	struct fileinfo *fileinfo;
	enum line_type type;
	int col = 0;
	size_t numberlen;
	size_t namelen;
    char *fname, *fnumber;

	if (view->offset + lineno >= view->lines)
		return false;

	fileinfo = view->line[view->offset + lineno];
	if (!*fileinfo->name)
		return false;

	wmove(view->win, lineno, col);

	if (view->offset + lineno == view->lineno) {
        fnumber = fileinfo->number;
        fname = fileinfo->name;
        snprintf(vim_cmd, sizeof(vim_cmd), VIM_CMD, fnumber, fname);
		type = LINE_CURSOR;
		wattrset(view->win, get_line_attr(type));
		wchgat(view->win, -1, 0, type, NULL);

	} else {
		type = LINE_FILE_LINCON;
		wchgat(view->win, -1, 0, type, NULL);
		wattrset(view->win, get_line_attr(LINE_FILE_NAME));
	}

	waddstr(view->win, fileinfo->name);
    namelen = strlen(fileinfo->name);

    col += 5;
	wmove(view->win, lineno, col);
	if (type != LINE_CURSOR)
		wattrset(view->win, get_line_attr(LINE_FILE_LINUM));

	waddstr(view->win, fileinfo->number);
    numberlen = strlen(fileinfo->number);

    col += 7;
	if (type != LINE_CURSOR)
		wattrset(view->win, A_NORMAL);

	mvwaddch(view->win, lineno, col, ACS_LTEE);
	wmove(view->win, lineno, col + 2);
	col += 2;

	if (type != LINE_CURSOR)
		wattrset(view->win, get_line_attr(type));

    int contentlen = strlen(fileinfo->content);

    if (col + contentlen > view->width-5*col)
        contentlen = view->width - 2*col;

    waddnstr(view->win, fileinfo->content, contentlen);

	return true;
}

static void open_view(struct view *prev)
{
    struct view *view = &main_view;

    if (view == prev) {
        report("Already in %s view", view->name);
        return;
    }
    
    if (!begin_update(view)) {
        report("Failed to load %s view", view->name);
        return;
    }

    /* Maximize the current view. */
    memset(display, 0, sizeof(display));
    current_view = 0;
    display[current_view] = view; 

    resize_display();

    if (view->pipe) {
        /* Clear the old view and let the incremental updating refill
         * the screen. */
        wclear(view->win);
        report("Loading...");
    }
}

static int view_driver(struct view *view, int key)
{
	switch (key) 
    {
	case 'j':
	case 'k':
		if (view)
			navigate_view(view, key);
		break;
	case 'q':
        quit(0);
        break;
    case 'e':
        addstr("Shelling out...");
        def_prog_mode();           /* save current tty modes */
        endwin();                  /* restore original tty modes */
        system(vim_cmd);              /* run shell */
        addstr("returned.\n");     /* prepare return message */
        reset_prog_mode();
        refresh();                 /* restore save modes, repaint screen */
        break;
    case 'm':
        open_view(view);
        break;
	default:
		return TRUE;
	}

	return TRUE;
}

static void report(const char *msg, ...)
{
    static bool empty = true;
    struct view *view = display[current_view];
    if (!empty || *msg) {
        va_list args;

        va_start(args, msg);

        werase(status_win);
        wmove(status_win, 0, 0);
        if (*msg) {
            vwprintw(status_win, msg, args);
            empty = false;
        } else{
            empty = true;
        }
        wrefresh(status_win);

        va_end(args);
    }
    update_title_win(view);
    
    if (view->lines) {
        wmove(view->win, view->lineno - view->offset, view->width - 1);
        wrefresh(view->win);
    }
}

static void navigate_view(struct view *view, int request)
{
	int steps;

	switch (request) {

	case 'k':
		steps = -1;
		break;

	case 'j':
		steps = 1;
		break;
	}

	if (steps <= 0 && view->lineno == 0) {
		report("already at first line");
		return;

	} else if (steps >= 0 && view->lineno + 1 == view->lines) {
		report("already at last line");
		return;
	}

	/* Move the current line */
	view->lineno += steps;
	assert(0 <= view->lineno && view->lineno < view->lines);

	/* Repaint the old "current" line if we be scrolling */
    view->render(view, view->lineno - steps - view->offset);

	/* Check whether the view needs to be scrolled */
	if (view->lineno < view->offset ||
	    view->lineno >= view->offset + view->height) 
    {
		if (steps < 0 && -steps > view->offset) 
        {
			steps = -view->offset;
		} 
        else if (steps > 0) 
        {
			if (view->lineno == view->lines - 1 &&
			    view->lines > view->height) 
            {
				steps = view->lines - view->offset - 1;
				if (steps >= view->height)
                {
					steps -= view->height - 1;
                }
			}
		}
        move_view(view, steps); 
		return;
	}
   
	/* Draw the current line */
	view->render(view, view->lineno - view->offset);

	redrawwin(view->win);
	wrefresh(view->win);
	update_title_win(view);
}

static void move_view(struct view *view, int lines)
{
	/* The rendering expects the new offset. */
	view->offset += lines;

	assert(0 <= view->offset && view->offset < view->lines);
	assert(lines);

    int line = lines > 0 ? view->height - lines : 0;
    int end = line + (lines > 0 ? lines : -lines);

    wscrl(view->win, lines);

    for (; line < end; line++) 
    {
        if (!view->render(view, line))
            break;
    }

	/* Move current line into the view. */
	if (view->lineno < view->offset) 
    {
		view->lineno = view->offset;
		view->render(view, 0);

	} 
    else if (view->lineno >= view->offset + view->height) 
    {
		view->lineno = view->offset + view->height - 1;
		view->render(view, view->lineno - view->offset);
	}

	assert(view->offset <= view->lineno && view->lineno < view->lines);

	redrawwin(view->win);
	wrefresh(view->win);

	update_title_win(view);
}

