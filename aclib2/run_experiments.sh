#!/bin/bash

cp -r /fsp/* /aclib2

cd results

# IFS=$'\n' read -d '' -r -a lines < /aclib2/experiments.txt
# 
# ncores=7
# nexper=${#lines[@]}
# 
# for ((i=0; i<$nexper; i=$i+$ncores))
# do
#   for ((j=0; j+$i<$nexper && j<$ncores; j++))
#   do
#     exp=${lines[$i+$j]}
#     python /aclib2/aclib/run.py -s $exp -c SMAC2 -n 1 --ac_cores 1 --cores_per_job 1 --env local --aclib_root /aclib2 &
#   done
#   wait
# done

for exp in NEH_FSP,FLOWTIME,NOIDLE,FIXEDTIME_15,med,erlang,jcorr,100,40
do
  python /aclib2/aclib/run.py -s NEH_FSP,FLOWTIME,NOIDLE,FIXEDTIME_15,med,erlang,jcorr,100,40 -c SMAC2 -n 1 --ac_cores 1 --cores_per_job 1 --env local --aclib_root /aclib2
done


# for exp in `cat /aclib2/experiments.txt`
# do
#     python3 /aclib2/aclib/validate.py -s $exp -c IRACE3 -n 1 --set TEST --num_validation_runs 1 --confs INC --env local
# done
# 
# for exp in `cat /aclib2/experiments.txt`
# do
#     python3 /aclib2/aclib/validate.py -s $exp -c PARAMILS -n 1 --set TEST --num_validation_runs 1 --confs INC --env local
# done
# 
# 
# for exp in `cat /aclib2/experiments.txt`
# do
#     python3 /aclib2/aclib/validate.py -s $exp -c IRACE3 -n 1 --set TEST --num_validation_runs 1 --confs DEF --env local
# done
# 
# for exp in `cat /aclib2/experiments.txt`
# do
#     python3 /aclib2/aclib/validate.py -s $exp -c PARAMILS -n 1 --set TEST --num_validation_runs 1 --confs DEF --env local
# done
#
# for exp in `cat /aclib2/experiments.txt`
# do
#     python3 /aclib2/aclib/validate.py -s $exp -c SMAC2 -n 1 --set TEST --num_validation_runs 1 --confs INC --env local
# done

