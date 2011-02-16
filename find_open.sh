#!/bin/bash

>output_file

mv $PWD'/'.git ~/git_tmp 2>/dev/null
data=`find . -exec grep -in "$1" {} \; -print`
echo "$data">>ttt

mv ~/git_tmp .git 2>/dev/null # this is not save

tac ttt > reversed ## reverse lines in the file
rm ttt

##############
#
# handle the reversed file
#
###############
while read line; do 
echo $line|grep ^./ &>/dev/null # serch the line that begins with "./", that is the filename

if [ $? -eq 0 ]
then
    filename=$line # if the line is a filename, then update filename
else
    echo $filename':'$line >>output_file # otherwise, add the filename to the lines below it
fi

done < reversed

rm reversed

#############
#
#   get info out of output_file
#
############

declare -a thefilename
declare -a linenumber

foo=0
while read line; do 
    echo -e "\033[1m NO.$(( ++foo )):\033[0m"
    thefilename[$foo]=`echo $line|awk -F":" '{print $1}'`
    linenumber[$foo]=`echo $line|awk -F":" '{print $2}'`
    linecontent=`echo $line|awk -F":" '{print $3}'`
    echo '    ' $linecontent
done < output_file

echo 
echo 
echo -n " Now which one do you want to open? [1,2,3...]  "
read nnn

case $nnn in
1)
vim +${linenumber[1]} ${thefilename[1]};;
2)
vim +${linenumber[2]} ${thefilename[2]};;
3)
vim +${linenumber[3]} ${thefilename[3]};;
4)
vim +${linenumber[4]} ${thefilename[4]};;
5)
vim +${linenumber[5]} ${thefilename[5]};;
6)
vim +${linenumber[6]} ${thefilename[6]};;
esac

rm output_file
