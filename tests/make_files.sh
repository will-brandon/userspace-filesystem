#!/bin/bash


MNT_ROOT="/home/wbrandon/cs3650/cs3650-project2/mnt"
DIR1="${MNT_ROOT}/hello"
DIR2="${DIR1}/world"
DIR3="${DIR2}/123"

mkdir -p "${DIR3}"

for j in {1..100}
do
    STRING=""

    for k in {1..100}
    do
        STRING="${STRING}Data Line ${k}.\n"
    done

    echo ${STRING} > "${DIR3}/${j}.txt"
done


for j in {1..100}
do
    rm "${DIR3}/${j}.txt"
done

rmdir ${DIR3} ${DIR2} ${DIR1}
