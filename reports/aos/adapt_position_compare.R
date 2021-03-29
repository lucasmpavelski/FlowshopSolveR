library(tidyverse)
library(here)
library(FlowshopSolveR)
library(future)
library(furrr)

# plan(remote, workers = rep("linode2", 1), persistent = TRUE)
NCORES <- 8
plan(multisession, workers = NCORES)
# plan(sequential)

print_cpp <- function(config, var = "params") {
  names(config) %>%
    walk(~cat(paste0(var, '["', .x, '"] = "', config[.x], '"'), ';\n'))
}

run_irace <- function(name, params) {
  set.seed(65487873)
  
  problems <- all_problems_df() %>%
    filter(
      problem %in% c('flowshop'),
      budget == 'high',
      type == 'PERM',
      objective == 'MAKESPAN',
      dist == 'uniform',
      corr == 'random',
      no_jobs %in% c(100, 200, 300)
    ) %>%
    mutate(
      # budget = 'med',
      instances = map(instances, ~filter(.x, inst_n == 1))
    ) %>%
    mutate(problem_space = pmap(., as_metaopt_problem)) %>%
    pull(problem_space)
  
  train_best_solver(
    problem_space = ProblemSpace(problems = problems),
    algorithm = Algorithm(
      name = 'IG',
      parameters = readParameters(text = params)
    ),
    solve_function = fsp_solver_performance,
    defaultScenario(list(
      maxExperiments = 1000
    )),
    parallel = NCORES,
    cache = here("reports", "aos", "data", "irace", paste(name, "rds", sep = "."))
  )
}

ig_base_config <- '
    IG.Init                            "" c (neh)
    IG.Init.NEH.Ratio                  "" c (0)
    IG.Init.NEH.Priority               "" c (sum_pij)
    IG.Init.NEH.PriorityOrder          "" c (incr)
    IG.Init.NEH.PriorityWeighted       "" c (no)
    IG.Init.NEH.Insertion              "" c (last_best)
    IG.Comp.Strat                      "" c (strict)
    IG.Neighborhood.Size               "" c (1.0)
    IG.Neighborhood.Strat              "" c (ordered)
    IG.Local.Search                    "" c (best_insertion)
    IG.LS.Single.Step                  "" c (1)
    IG.Accept                          "" c (temperature)
    IG.Accept.Better.Comparison        "" c (strict)
    IG.Accept.Temperature              "" c (0.25)
    IG.Perturb.Insertion               "" c (random_best)
    IG.Perturb                         "" c (lsps)
    IG.LSPS.Local.Search               "" c (best_insertion)
    IG.LSPS.Single.Step                "" c (1)
'

ig_adapt_pos_base_config <- paste(ig_base_config, '
  IG.Perturb.DestructionSizeStrategy "" c (fixed)
  IG.Perturb.DestructionSize         "" c (2)
  IG.DestructionStrategy                    "" c (adaptive_position)
  IG.AdaptivePosition.AOS.WarmUp.Strategy   "" c (random)
  IG.AdaptivePosition.AOS.Options           "" c (0_1_2_3)
  IG.AdaptivePosition.AOS.WarmUp            "" i (0,2000)
  IG.AdaptivePosition.AOS.RewardType        "" c (0,1,2,3)
')

ig_adapt_ds_base_config <- paste(ig_base_config, '
  IG.Perturb.DestructionSize         "" c (2)
  IG.DestructionStrategy             "" c (random)
  IG.Perturb.DestructionSizeStrategy "" c (adaptive)
  IG.AOS.Options               "" c (1_2_4)
  IG.AOS.WarmUp.Strategy   "" c (random, fixed)
  IG.AOS.WarmUp            "" i (0,2000)
  IG.AOS.RewardType        "" c (0,1,2,3)
')

configs <- tribble(
  ~name, ~params,
  'ig_ss_lsps-pos_probability_matching', paste(ig_adapt_pos_base_config, '
    IG.AdaptivePosition.AOS.Strategy        "" c (probability_matching)
    IG.AdaptivePosition.AOS.PM.RewardType   "" c (avgabs,avgnorm,extabs,extnorm)
    IG.AdaptivePosition.AOS.PM.Alpha        "" r (0.1, 0.9)
    IG.AdaptivePosition.AOS.PM.PMin         "" r (0.05, 0.2)
    IG.AdaptivePosition.AOS.PM.UpdateWindow "" i (1,500)
  '),
  'ig_ss_lsps-pos_frrmab', paste(ig_adapt_pos_base_config, '
    IG.AdaptivePosition.AOS.Strategy          "" c (frrmab)
    IG.AdaptivePosition.AOS.FRRMAB.WindowSize "" i (10, 500)
    IG.AdaptivePosition.AOS.FRRMAB.Scale      "" r (0.01, 100)
    IG.AdaptivePosition.AOS.FRRMAB.Decay      "" r (0.25, 1.0)
  '),
  'ig_ss_lsps-pos_linucb', paste(ig_adapt_pos_base_config, '
    IG.AdaptivePosition.AOS.Strategy     "" c (linucb)
    IG.AdaptivePosition.AOS.LINUCB.Alpha "" r (0.0, 1.5)
  '),
  'ig_ss_lsps-pos_thompson_sampling', paste(ig_adapt_pos_base_config, '
    IG.AdaptivePosition.AOS.Strategy    "" c (thompson_sampling)
    IG.AdaptivePosition.AOS.TS.Strategy "" c (static, dynamic)
    IG.AdaptivePosition.AOS.TS.C        "" i (1,500) | IG.AdaptivePosition.AOS.TS.Strategy == "dynamic"
  '),
  'ig_ss_lsps-ds_probability_matching', paste(ig_adapt_ds_base_config, '
    IG.AOS.Strategy        "" c (probability_matching)
    IG.AOS.PM.RewardType   "" c (avgabs,avgnorm,extabs,extnorm)
    IG.AOS.PM.Alpha        "" r (0.1, 0.9)
    IG.AOS.PM.PMin         "" r (0.05, 0.2)
    IG.AOS.PM.UpdateWindow "" i (1,500)
  '),
  'ig_ss_lsps-ds_frrmab', paste(ig_adapt_ds_base_config, '
    IG.AOS.Strategy          "" c (frrmab)
    IG.AOS.FRRMAB.WindowSize "" i (10, 500)
    IG.AOS.FRRMAB.Scale      "" r (0.01, 100)
    IG.AOS.FRRMAB.Decay      "" r (0.25, 1.0)
  '),
  'ig_ss_lsps-ds_linucb', paste(ig_adapt_ds_base_config, '
    IG.AOS.Strategy     "" c (linucb)
    IG.AOS.LINUCB.Alpha "" r (0.0, 1.5)
  '),
  'ig_ss_lsps-ds_thompson_sampling', paste(ig_adapt_ds_base_config, '
    IG.AOS.Strategy    "" c (thompson_sampling)
    IG.AOS.TS.Strategy "" c (static, dynamic)
    IG.AOS.TS.C        "" i (1,500) | IG.AOS.TS.Strategy == "dynamic"
  ')
) %>% pmap(., run_irace) 
