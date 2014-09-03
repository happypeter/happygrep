## Simple and nice grep front-end: happygrep

happygrep is a grep TUI front-end based on ncursesw.

### Installation (Ubuntu)

需要先安装依赖库

    sudo apt-get -y install libncursesw5 libncursesw5-dev
    cd happygrep/
    make
    
得到二进制文件 happygrep ，可以

    mv happygrep ~/bin/xxx
    
以后就可以这样用了

    xxx string

### Installation (Mac OS X)

在我自己的 10.8.5 系统上

    $ brew --version
    0.9.5
    $ brew search ncurses
    homebrew/dupes/ncurses
    $ brew install homebrew/dupes/ncurses
    $ make -f Makefile.macosx

这样就装好了。libiconv 在 10.8.5
的系统上已经预装了，但是较老的系统上好像没有，所以在老版本的 mac
上，我自己安装的过程是：

    $ brew install ncurses
    $ brew install libiconv
    $ make -f Makefile.macosx

并且老版本的 Makefile.macosx 中的编译语句是这样的：

    gcc happygrep.c  -I/usr/local/opt/libiconv/include -I/usr/local/opt/ncurses/include -L/usr/local/opt/libiconv/lib -L/usr/local/opt/ncurses/lib -o happygrep -lncursesw  -liconv -Wall

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

Copyright (c) 2011 happypeter

