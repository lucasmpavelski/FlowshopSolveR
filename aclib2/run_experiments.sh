#!/bin/sh

cd results

for exp in `cat /experiments.txt`
do
    python /aclib2/aclib/run.py -s $exp -c IRACE3 -n 1 --ac_cores 5 --cores_per_job 5 --env local --aclib_root /aclib2
done