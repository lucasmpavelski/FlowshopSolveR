exec="/home/lucasmp/dev/aos-ig-flowshop/build/main/fsp_solver"
data="/home/lucasmp/dev/aos-ig-flowshop/data"
seed=123
objective=MAKESPAN
algo=0
aos=0
ds=4
expdir="runs/ig"
mkdir $expdir
rm $expdir/*
for instance in $(cat /home/lucasmp/dev/evolutionary_tunners/scripts/instances.txt)
do
  for ds in 2 4 8
  do
    problem_names=problem,type,objective,budget,instance,stopping_criterium
    problem_values=FSP,PERM,$objective,med,$instance,FIXEDTIME
    params_names=IG.Init.Strat,IG.Comp.Strat,IG.Neighborhood.Size,IG.Neighborhood.Strat,IG.Local.Search,IG.Accept,IG.Accept.Temperature,IG.Algo,IG.Destruction.Size,IG.LS.Single.Step,IG.LSPS.Local.Search,IG.LSPS.Single.Step,IG.AOS.Strategy
    params_values=0,0,9.9999,0,3,2,0.5,$algo,$ds,0,0,0,$aos
    filename=$expdir/${i}_${problem_values}_${params_values}


      echo "$exec \
      --seed=$seed \
      --data_folder=$data \
      --mh=IG \
      --problem_names=$problem_names \
      --problem_values=$problem_values \
      --params_names=$params_names \
      --params_values=$params_values > ${filename}.out 2> ${filename}.err " 

    taskset -c $((d/2)) $exec \
      --seed=$seed \
      --data_folder=$data \
      --mh=IG \
      --problem_names=$problem_names \
      --problem_values=$problem_values \
      --params_names=$params_names \
      --params_values=$params_values > ${filename}.out 2> ${filename}.err 
  done
  wait
done