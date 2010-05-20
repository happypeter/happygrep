:syn keyword PETER peter
:hi link PETER Statement
:syntax match FILENAME /\.\/[a-z1-9\.]*/ "filename like ./muse2markdown.sh is now handled
" all the backslash above are used to remove the special meaning of . and /
" TODO: I do need to highlight the keyword (the $1 from xxx $1) as well
:hi link FILENAME Statement  
" ref: http://vimdoc.sourceforge.net/htmldoc/usr_44.html


