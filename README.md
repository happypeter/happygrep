## Simple and nice grep front-end: happygrep

While `grep` is a powerful command line tool, it is difficult to use
So for the convenient of human being, we need a more smart
tool to do our work.

`happygrep` is a small and nice `grep` front-end based on `ncursesw` __TUI__.
Its internal command is `grep` used to find all the matching text line
containing specific strings, then displaying them in the `ncursesw` window.
So you can view the results with ease.

### Installation (Ubuntu)

Since the application is dependent on `ncursesw` library, so we need to
install the library firstly, otherwise it can't be compiled successfully :

    sudo apt-get -y install libncursesw5 libncursesw5-dev

### Installation (Mac OS X)
First, install Brew package manager. Then run the following commands in Terminal.

	 brew install ncurses
	 brew install libiconv
	 make -f Makefile.macosx
	 make -f Makefile.macosx install

Note: the executable file is installed in ~/bin folder.

#### Basic GitHub Checkout

This will get you going with the latest version of `happygrep` and make it
easy to fork and contribute any changes back upstream.

1. Check out the code

        $ git clone git://github.com/happypeter/happygrep.git

2. Then compile and install it.

        $ cd ~/happypeter/
        $ make
        $ sudo make install

### Usage

By default, happygrep skips the `.git` directory, and it supports regular
expression, it is `grep` anyway. In addition, `happygrep` can specify one
directory to ignore. The directory name can use regex.

For instance, we want to search the string `hello world` in a git` repository called
`techdoc`, we can do like this:

    happygrep "hello world"

Also, you can ignore a directory named `image/`:

    happygrep "hello world" -i "image"

After running the commands above respectively, then you will get a nice window based
text user interface.

* use `j` and `k` (or up/down arrows) to select the entry listed in window

* type `e` character to open the file where the proper entry appeared

* edit the opened file in `vim` editor

* close `vim` to return to the original window to continue

* type `q` character to quit

### Development

Please feel free to submit pull requests and file bugs on the [issue
tracker](https://github.com/happypeter/happygrep/issues).

### Contributors

* [happypeter (author)](https://github.com/happypeter)

* [billie66](https://github.com/billie66)

### License

(The MIT license)

Copyright (c) 2011 happpypeter

