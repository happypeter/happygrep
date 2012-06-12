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

>output_file # clear old data

#################################
#
#    on_die
#    
#################################
on_die()
{

rm output_file &>/dev/null
echo
echo mission canceled, bye.
exit 0

}

trap 'on_die' INT # Ctr-c sent SIGINT

#################################
#
#    billie's xxx
#    
#################################

data=`find . -name .git -prune -o \( \! -name *.swp \) -exec grep -in "$1" {} +`
# according to examples from the end of man find 
# *.swp , now I do not need to stop vim to run xxx, so I can use ':sh' in vim
echo "$data">output_file

if [ -z "$data" ] ## output_file is empty, so no match, quit
then
    echo  No match, bye!
    rm output_file 
    exit 
fi


#################################
#
#    now remove training "\"
#    otherwise the below "read line" 
#    will read two lines as one
#    
#################################

sed -i 's/[\\]*$//g' output_file ## NOTE: it does not matter if there are spaces after '\' in source file

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
    thefilename[$count]=`echo "$line"|awk -F":" '{print $1}'`
    linenumber[$count]=`echo "$line"|awk -F":" '{print $2}'`

    linecontent=`echo "$line"|sed 's/^.*:[0-9]*://'` ## .* matches any set of charaters
    # http://en.wikipedia.org/wiki/Regular_expression
    # "./filename:linmumber:linecontent" -> "linecontent"
    # can not use awk {print $3}, because we may have "http://www..."
    
    echo ${thefilename[$count]}:${linenumber[$count]}
    echo "    $linecontent"  # echo will expand special characters like *, unless you add "" 
done < output_file

# FIXME: if the program dies in the middle, like Ctr-C is used, the
# output_file won't be removed

rm output_file

#################################
#
#    choose
#    
#################################

echo 
echo 

while [ 0 ]
do
    echo -n " Now which one do you want to open? [1-$count]  "
    read nu
    if [ $nu -gt 0 -a $nu -lt $(($count+1)) 2> /dev/null ] ## stderr output when $nu is not a number
    then 
# return to main window to select a number, then use ctrl+c to cancel
        vim +${linenumber[$nu]} ${thefilename[$nu]}
    else 
        echo wrong nu, input again.
    fi
done
