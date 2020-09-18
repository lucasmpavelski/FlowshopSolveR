#!/bin/sh

cd results && \
python /aclib2/aclib/run.py -s neh_fsp-50-10 -c IRACE3 -n 1 --ac_cores 14 --cores_per_job 14 --env local --aclib_root /aclib2