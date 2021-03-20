#!/bin/sh

mkdir -p ownresults
rm -rf ownresults/output*.txt
rm -rf ownresults/simout*.txt
make clean && make full



./full.out 1 2 0.01 256 4 0.5 128 > ownresults/output02-full.txt

./full.out 2 2 0.01 256 4 0.5 128 > ownresults/output03-full.txt

./full.out 16 2 0.01 256 4 0.75 64 > ownresults/output04-full.txt

./full.out 8 64 0.001 4096 4 0.5 2048 > ownresults/output05-full.txt

./full.out 8 64 0.001 4096 4 0.5 2048 BEGINNING > ownresults/output06-full.txt

make clean && make limited


./limited.out 1 2 0.01 256 4 0.5 128 > ownresults/output02.txt
mv simout.txt ownresults/simout02.txt

./limited.out 2 2 0.01 256 4 0.5 128 > ownresults/output03.txt
mv simout.txt ownresults/simout03.txt

./limited.out 16 2 0.01 256 4 0.75 64 > ownresults/output04.txt
mv simout.txt ownresults/simout04.txt

./limited.out 8 64 0.001 4096 4 0.5 2048 > ownresults/output05.txt
mv simout.txt ownresults/simout05.txt

./limited.out 8 64 0.001 4096 4 0.5 2048 BEGINNING > ownresults/output06.txt
mv simout.txt ownresults/simout06.txt

./limited.out 8 64 0.001 4096 20 0.5 2048 > ownresults/output06.txt
mv simout.txt ownresults/simout07.txt


mkdir -p diff-files
names=`ls ownresults/*.txt | xargs -n 1 basename`
for fname in $names; do
  diff "ownresults/$fname" "profresults/$fname" > "diff-files/diff-$fname"
done

cat diff-files/* > diff.txt

cat ownresults/simout*.txt > ownresults/all_simout.txt

make clean