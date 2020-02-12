#!/bin/bash
SHELL="./shell"
TEST_FOLDER="tests"

if [ "$#" -le 0 ]; then
    for f in "$TEST_FOLDER"/*.txt;
    do
        ./sdriver.pl -t "$f" -s "$SHELL"
    done
    exit
fi

for f in "$@";
do
    ./sdriver.pl -t "$TEST_FOLDER/$f" -s "$SHELL"
done