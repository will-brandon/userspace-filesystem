#!/bin/bash

MNT_ROOT="/home/wbrandon/cs3650/cs3650-project2/mnt"

STRING="1"

#for k in {1..4095}
#do
#    STRING="${STRING}o\n"
#done

for i in {1..100}
do
    echo ${STRING} > "${MNT_ROOT}/${i}.txt"
done
