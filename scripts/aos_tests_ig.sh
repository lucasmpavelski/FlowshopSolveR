dir=`pwd`
exec="${dir}/build/main/fsp_solver"
data="${dir}/data"
seed=123
objective=MAKESPAN
algo=0
aos=0
ds=4
expdir="runs/ig"
mkdir $expdir
rm $expdir/*
sqlite3 "$expdir/experiments.db" "create table experiments (id integer, problem_names text, problem_values text, params_names text, params_values, filename)"
i=1
for instance in $(cat /home/lucasmp/dev/evolutionary_tunners/scripts/instances.txt)
do
  for ds in 2 4 8
  do
    problem_names=problem,type,objective,budget,instance,stopping_criterium
    problem_values=FSP,PERM,$objective,med,$instance,FIXEDTIME
    params_names=IG.Init.Strat,IG.Comp.Strat,IG.Neighborhood.Size,IG.Neighborhood.Strat,IG.Local.Search,IG.Accept,IG.Accept.Temperature,IG.Algo,IG.Destruction.Size,IG.LS.Single.Step,IG.LSPS.Local.Search,IG.LSPS.Single.Step,IG.AOS.Strategy
    params_values=0,0,9.9999,0,3,2,0.5,$algo,$ds,1,0,0,$aos
    filename=$expdir/${i}_${problem_values}_${params_values}
    taskset -c 2 $exec \
      --seed=$seed \
      --data_folder=$data \
      --mh=IG \
      --problem_names=$problem_names \
      --problem_values=$problem_values \
      --params_names=$params_names \
      --params_values=$params_values > ${filename}.out 2> ${filename}.err
    echo "$exec \
      --seed=$seed \
      --data_folder=$data \
      --mh=IG \
      --problem_names=$problem_names \
      --problem_values=$problem_values \
      --params_names=$params_names \
      --params_values=$params_values > ${filename}.out"
    sqlite3 "$expdir/experiments.db" "insert into experiments values (${i}, '${problem_names}', '${problem_values}', '${params_names}', '${params_values}', '${filename}')"
    ((i=i+1))
  done
done

