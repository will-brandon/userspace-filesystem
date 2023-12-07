#!/bin/bash

MNT_ROOT="/home/wbrandon/cs3650/cs3650-project2/mnt"
DIR="${MNT_ROOT}/hello/123/456/789"

mkdir -p "${DIR}"

for i in {1..1000}
do
    touch "${DIR}/${i}.txt"
done
