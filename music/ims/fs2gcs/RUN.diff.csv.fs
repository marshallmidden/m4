#!/bin/bash -epux
#-----------------------------------------------------------------------------
rm -f t.fs TTT u.csv u.fs UUU ZZZ

cat HEADER.gcs test.gcs > tmp.gcs
../imscomp --fs tmp.gcs t.fs
./fs2gcs t.fs t.gcs 2>&1 | tee TTT

../imscomp --csv tmp.gcs u.csv
../csv2fs/csv2fs u.csv u.fs
./fs2gcs u.fs u.gcs 2>&1 | tee UUU

diff -u TTT UUU  >ZZZ || true
#-- vi ZZZ
#-----------------------------------------------------------------------------
