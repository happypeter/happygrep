## Simple and nice grep front-end: happygrep 

While `grep` is a powerful command line tool, its syntax is difficult and its
output is ugly. So for the convenient of human being, we need a more smart
tool to do our work. 

`happygrep` is a small and nice `grep` front-end based on `ncursesw` __TUI__. 
Its internal command is `grep` used to find all the matching text line
containing specific strings, then displaying them in the `ncursesw` window.
So you can view the results with ease.

### Installation (Ubuntu)

Since the application is dependent on `ncursesw` library, so we need to
install the library firstly, otherwise it can't be compiled successfully :
    
    sudo apt-get install libncursesw5 libncursesw5-dev 

Up to this point, I didn't give other platforms a try.

#### Basic GitHub Checkout

This will get you going with the latest version of `happygrep` and make it
easy to fork and contribute any changes back upstream.

* Check out `happygrep` into `~/happygrep`.

    $ cd
    $ git clone git://github.com/happygrep/happygrep.git 

* Then go to `~/happygrep/search/curse/` directory, compire and install it.

    $ cd ~/happypeter/search/curse/
    $ make
    $ sudo make install
         
Note After compiling `happygrep`, you will get an executable file called `xxx`
that will be installed in `~/bin` directory, the default search path in
unbuntu, so it is not neccessary to add `~/bin` to `PATH` environment
variable.  

### Upgrading

If you've installed `happygrep` using the instructions above, you can
upgrade your installation at any time using git.

To upgrade to the latest development version of `happygrep`, use `git pull`:

    $ cd ~/happygrep
    $ git pull

### Usage

The small application has limited usages, just two forms, so in order to
improve it, there are many work needed to do. By default, it will skip the 
`.git` directory when you search a `git` repository, and it does support
regular expression, since the search task is done by `grep` command. In
addition, `happygrep` can specify one directory to ignore. The directory name
can use regex. 

For instance, we want to search the string `hello world` in a git` repository called
`techdoc`, we can do like this:

    xxx "hello world"

Also, you can ignore a directory named "image":

    xxx "hello world" "image"

After running the commands above respectively, then you will get a nice window based
text user interface. 

* use up/down arrow to select the entry listed in window

* type `e` character to open the file where the proper entry appeared

* edit the opened file in `vim` editor

* close `vim` to return to the original window to continue

* type `q` character to quit 

### Development

The `happygrep`source code is [hosted on
GitHub](https://github.com/happypeter/happygrep). It's clean and easy
to understand, even if you're not a C hacker.

Please feel free to submit pull requests and file bugs on the [issue
tracker](https://github.com/happypeter/happygrep/issues).

### Contributors 

* [happypeter (author)](https://github.com/happypeter)

* [billie66](https://github.com/billie66)

### License

(The MIT license)

Copyright (c) 2011 happpypeter 

