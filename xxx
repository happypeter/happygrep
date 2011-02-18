#!/bin/bash

#################################
#
#    usage error
#    
#################################
if [ $# -eq 0 ] 
then
    echo """
    Usage: xxx PATTERN 
    """
    exit 1
fi

#################################
#
#    the xxx method 
#    
#################################
>output_file
mv $PWD'/'.git ~/git_tmp 2>/dev/null
data=`find . -exec grep -in "$1" {} \; -print`
echo "$data">>ttt

mv ~/git_tmp .git 2>/dev/null # FIXME:this is not save

ttt_content=`cat ttt`

if [ -z "$ttt_content" ]
then
    echo  no match
    rm ttt
    exit ## ttt is empty, so no match, quit
fi

tac ttt > reversed ## reverse lines in the file
rm ttt

#################################
#
#    process reversed
#    
#################################
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


#################################
#
#    process output_file
#    
#################################

declare -a thefilename
declare -a linenumber

count=0
while read line; do 
    echo -ne "\033[1m NO.$(( ++count )):\033[0m"
    thefilename[$count]=`echo $line|awk -F":" '{print $1}'`
    linenumber[$count]=`echo $line|awk -F":" '{print $2}'`
    linecontent=`echo $line|awk -F":" '{print $3}'` # FIXME:it is possible to have "http://xxx" as content
    echo ${thefilename[$count]}
    echo '    ' $linecontent
done < output_file
rm output_file

#################################
#
#    choose
#    
#################################

echo 
echo 
echo -n " Now which one do you want to open? [1,2,3...]  "
read nu

vim +${linenumber[$nu]} ${thefilename[$nu]}
