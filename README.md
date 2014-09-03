### 命令行查找可爱多: happygrep

happygrep 采用 ncursesw 的 TUI 字符界面图形库，相当于 find + grep + 前端界面。


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

happygrep 会默认忽略 `.git` 目录. 另外也可以通过 -i 参数指定另一个要忽略的目录，例如

    happygrep "hello world" -i "image"

这样可以忽略 image/ 目录。


在打开的 TUI 界面上，可以使用的快捷键


* use `j` and `k` (or up/down arrows) to select the entry listed in window

* type `e` character to open the file where the proper entry appeared

* edit the opened file in `vim` editor

* close `vim` to return to the original window to continue

* type `q` character to quit

### Development

有任何的问题和建议，欢迎到 [issue
tracker](https://github.com/happypeter/happygrep/issues).

### Contributors

* [happypeter (原作者)](https://github.com/happypeter)

* [billie66 (当前代码的最大贡献者)](https://github.com/billie66)


### License

The MIT license

