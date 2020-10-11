#!/bin/bash

cp -r /fsp/* /aclib2

cd results

IFS=$'\n' read -d '' -r -a lines < /aclib2/experiments.txt

ncores=11
nexper=${#lines[@]}

#for ((i=0; i<$nexper; i=$i+$ncores))
#do
#  for ((j=0; j+$i<$nexper && j<$ncores; j++))
#  do
#    exp=${lines[$i+$j]}
#    python /aclib2/aclib/run.py -s $exp -c IRACE3 -n 1 --ac_cores 1 --cores_per_job 1 --env local --aclib_root /aclib2 &
#  done
#  wait
#done

for ((i=0; i<$nexper; i=$i+$ncores))
do
  for ((j=0; j+$i<$nexper && j<$ncores; j++))
  do
    exp=${lines[$i+$j]}
    python /aclib2/aclib/run.py -s $exp -c PARAMILS -n 1 --ac_cores 1 --cores_per_job 1 --env local --aclib_root /aclib2 &
  done
  wait
done

for ((i=0; i<$nexper; i=$i+$ncores))
do
  for ((j=0; j+$i<$nexper && j<$ncores; j++))
  do
    exp=${lines[$i+$j]}
    python /aclib2/aclib/run.py -s $exp -c SMAC3 -n 1 --ac_cores 1 --cores_per_job 1 --env local --aclib_root /aclib2 &
  done
  wait
done

#for exp in NEH_flowshop,FLOWTIME,NOIDLE,TIME,low,erlang,random,100,20
#do
# # python3 /aclib2/aclib/run.py -s NEH_flowshop,FLOWTIME,NOIDLE,TIME,low,erlang,random,100,20 -c IRACE3 -n 1 --ac_cores 1 --cores_per_job 1 --env local --aclib_root /aclib2
#  #python3 /aclib2/aclib/validate.py -s $exp -c IRACE3 -n 1 --set TEST --num_validation_runs 1 --confs INC --env local
#done

# for exp in `cat /aclib2/experiments.txt`
# do
#   for conf in SMAC3 PARAMILS
#   do
#       python3 /aclib2/aclib/validate.py -s $exp -c $conf -n 1 --set TEST --num_validation_runs 1 --confs INC --env local
#   done
#   python3 /aclib2/aclib/validate.py -s $exp -c PARAMILS -n 1 --set TEST --num_validation_runs 1 --confs DEF --env local
# done

