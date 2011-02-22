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
static void scroll_view(/* win */ int request);

/*
 * Main
 */

int
main(int argc, char *argv[])
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

	{
		FILE *rev_list = popen("git diff HEAD^", "r");
		char buffer[BUFSIZ];
		char *line;
		int lineno = 1;

		while ((line = fgets(buffer, sizeof(buffer), rev_list))) {
			move(lineno, 0);
			printw("%2d: ", lineno++);
			addch(ACS_LTEE);
			addch(ACS_HLINE);
			addstr(line);
		}
	}
	attrset(A_NORMAL);
/*        scrollok();*/

	for (;;) 
    {
		int c = getch();     /* refresh, accept single keystroke of input */

		/* Process the command keystroke */
		switch (c) 
        {
            case 'q':
                quit(0);
                return 0;

            case 's':
                addstr("Shelling out...");
                def_prog_mode();           /* save current tty modes */
                endwin();                  /* restore original tty modes */
                system("sh");              /* run shell */
                addstr("returned.\n");     /* prepare return message */
                reset_prog_mode();
                //refresh();                 /* restore save modes, repaint screen */
                break;
            case 'k':
                scroll_view(c);           
                break;
            case 'j':
                scroll_view(c);           
                break;

            default:
                if (isprint(c) || isspace(c))
                    addch(c);
		}

	}

	quit(0);
}

static void 
quit(int sig)
{
	endwin();
    printf("printf quit\n");

	/* do your non-curses wrapup here */

	exit(0);
}

static void
init_colors(void)
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

static void
init(void)
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
static void
scroll_view(/* win */ int request)
{
    int lines = 0;
    switch (request) 
    {
        case 'j':
            lines = 1;
            break;
        case 'k':
            lines = -1;
            break;
        default:
            lines = 0;
    }
    if (lines) 
    {
        wscrl(stdscr, lines);
    }
	redrawwin(stdscr);
	wrefresh(stdscr);
}
