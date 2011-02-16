#!/bin/bash

>output_file

mv $PWD'/'.git ~/git_tmp 2>/dev/null
data=`find . -exec grep -in "$1" {} \; -print`
echo "$data">>ttt

mv ~/git_tmp .git

tac ttt > reversed ## reverse lines in the file
rm ttt

##############
#
# handle the reversed file
#
###############
while read line; do 
echo $line # or whaterver you want to do with the $line variable
echo $line|grep ^./  # serch the line that begins with "./", that is the filename

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
    echo $(( ++foo ))
    echo $line
    thefilename[$foo]=`echo $line|awk -F":" '{print $1}'`
    linenumber[$foo]=`echo $line|awk -F":" '{print $2}'`
    echo ${thefilename[$foo]} xxx---xxx ${linenumber[$foo]}
done < output_file


echo -n " Now which one do you want to open? "
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
esac

rm output_file
