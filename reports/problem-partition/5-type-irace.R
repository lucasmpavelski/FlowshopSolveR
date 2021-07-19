library(MOEADr)
library(FlowshopSolveR)
library(here)
library(tidyverse)
library(furrr)

EXP_NAME <- "objective-medium-hp"

# plan(sequential)
plan(multisession, workers = 32)
# # plan(remote,
# #      workers = rep("linode2", 8),
# #      persistent = TRUE)
# 
# 
# 
# set.seed(54654654)
# 
parameter_space <- readParameters(
  text = '
IG.Init                            "" c (neh)
IG.Init.NEH.Ratio                  "" c (0)
IG.Init.NEH.Priority               "" c (sum_pij)
IG.Init.NEH.PriorityOrder          "" c (incr)
IG.Init.NEH.PriorityWeighted       "" c (no)
IG.Init.NEH.Insertion              "" c (first_best)
IG.Comp.Strat                      "" c (strict,equal)
IG.Neighborhood.Size               "" r (0.0, 1.0)
IG.Neighborhood.Strat              "" c (random)
IG.LS.Single.Step                  "" c (0, 1)
IG.Accept                          "" c (always, better, temperature)
IG.Accept.Better.Comparison        "" c (strict)
IG.Perturb.Insertion               "" c (random_best,first_best,last_best)
IG.Perturb                         "" c (rs,lsps,swap)
IG.LSPS.Local.Search               "" c (none,first_improvement,best_improvement,random_best_improvement,best_insertion) | IG.Perturb == "lsps"
IG.LSPS.Single.Step                "" c (0,1) | IG.Perturb == "lsps" & IG.LSPS.Local.Search != "none"
IG.Perturb.DestructionSizeStrategy "" c (fixed)
IG.DestructionStrategy             "" c (random)
IG.Local.Search                    "" c (none,first_improvement,best_improvement,random_best_improvement,best_insertion)
IG.Accept.Temperature              "" r (0.0, 5.0) | IG.Accept == "temperature"
IG.Perturb.DestructionSize         "" i (2,8) | IG.Perturb == "rs" || IG.Perturb == "lsps"
IG.Perturb.NumberOfSwapsStrategy   "" c (fixed) | IG.Perturb == "swap"
IG.Perturb.NumberOfSwaps           "" i (2,8) | IG.Perturb == "swap" && IG.Perturb.NumberOfSwapsStrategy == "fixed"
'
)


all_problems <- all_problems_df() %>%
  filter(
    problem == "flowshop",
    no_jobs %in% c(100),
    no_machines %in% c(20),
    type %in% c('PERM', 'NOWAIT', 'NOIDLE'),
    objective %in% c('MAKESPAN'),
    dist %in% c('exponential', 'uniform'),
    stopping_criterion == 'TIME',
    budget == 'low'
  ) %>%
  mutate(stopping_criterion = "FIXEDTIME") %>%
  unnest(instances) %>%
  filter(inst_n <= 6) %>%
  mutate(id = id * 100 + inst_n) %>%
  nest(instances = c(inst_n, instance)) %>%
  rowwise() %>%
  mutate(meta_objective = which(type == c('PERM', 'NOWAIT', 'NOIDLE'))) %>%
  ungroup() %>%
  mutate(problem_space = pmap(., as_metaopt_problem))

objective_axis <- unique(all_problems$objective)
all_problems <- all_problems %>%
  rowwise() %>%
  mutate(meta_objective = which(objective == objective_axis)) %>%
  ungroup() %>%
  mutate(problem_space = pmap(., as_metaopt_problem))

solve_function <- fsp_solver_performance

algorithm <- Algorithm(name = "IG", parameters = parameter_space)

best_solvers <- train_best_solver(
  problem_space = ProblemSpace(problems = all_problems$problem_space),
  algorithm = algorithm,
  solve_function = solve_function,
  irace_scenario = defaultScenario(
    list(
      maxExperiments = 89600 #, minNbSurvival = 1
    )
  ),
  parallel = 32,
  quiet = F,
  cache = here("data", "problem_partition", "type-medium-100jobs-20machines-irace"),
  recover = TRUE
)
