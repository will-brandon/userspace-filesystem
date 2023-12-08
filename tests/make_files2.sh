#!/bin/bash

MNT_ROOT="/home/wbrandon/cs3650/cs3650-project2/mnt";

STR_4K="";
STR_100K="";
STR_500K="";


for i in {1..4095}  # GOAL: 4096 = 4095 + 1 <- extra null end byte
do
    STR_4K="${STR_4K}x";
done;
printf "4K lenth: ${#STR_4K}\n";


for i in {1..31}  # GOAL: 131072 = (4095 + 1) * 31 + 4095 + 1 <- extra null end byte
do
    STR_100K="${STR_100K}${STR_4K}x";
done;
STR_100K="${STR_100K}${STR_4K}";
printf "100K lenth: ${#STR_100K}\n";


for i in {1..3}  # GOAL: 524288 = (131071 + 1) * 3 + 131071 + 1 <- extra null end byte
do
    STR_500K="${STR_500K}${STR_100K}x";
done;
STR_500K="${STR_500K}${STR_100K}";
printf "500K lenth: ${#STR_500K}\n";


printf "Making and removing all the mandatory sized files 10 times for good measure\n"
printf "===========================================================================\n"
for epoch in {1..1}
do
    printf "STARTING EPOCH ${epoch}\n";

    printf "Making 150 4K files\n";
    for i in {1..65}
    do
        echo "${STR_4K}" > "${MNT_ROOT}/${i}.txt";
    done;
    printf "Removing all 150 4K files\n";
    rm "${MNT_ROOT}"/*;

done;

exit;

    printf "Making 6 100K files.\n";
    for i in {1..6}
    do
        echo "${STR_100K}" > "${MNT_ROOT}/${i}.txt";
    done;
    ls -la "${MNT_ROOT}";
    printf "Removing all 6 100K files\n";
    rm "${MNT_ROOT}"/*;
    ls -la "${MNT_ROOT}";

    printf "Making 1 500K file\n";
    echo "${STR_500K}" > "${MNT_ROOT}/${i}.txt";
    ls -la "${MNT_ROOT}";
    printf "Removing 1 500K file\n";
    rm "${MNT_ROOT}"/*;
    ls -la "${MNT_ROOT}";

    printf "FINISHED EPOCH ${epoch}\n";

done;
