library(FlowshopSolveR)
library(tidyverse)
library(here)
library(wrapr)
library(irace)

plan(multisession)

exp_folder <- here("reports", "aos", "data", "exp-04-adaptive_destruction_size")
perf_folder <- file.path(exp_folder, "perf")
irace_folder <- file.path(exp_folder, "irace")

dir.create(perf_folder, recursive = T, showWarnings = F)
dir.create(irace_folder, recursive = T, showWarnings = F)

train_problems <- all_problems_df() %>%
  filter(
    problem %in% c('flowshop'),
    budget == 'high',
    type == 'PERM',
    objective == 'FLOWTIME',
    dist == 'uniform',
    corr == 'random',
    no_jobs %in% c(100, 200, 300)
  ) %>%
  mutate(
    budget = 'med',
    instances = map(instances, ~filter(.x, inst_n == 1))
  ) %>%
  mutate(problem_space = pmap(., as_metaopt_problem)) %>%
  pull(problem_space)

test_problems <- all_problems_df() %>%
  filter(
    problem %in% c('flowshop'),
    budget == 'high',
    type == 'PERM',
    objective == 'FLOWTIME',
    dist == 'uniform',
    corr == 'random',
    no_jobs %in% c(100, 200, 300)
  ) %>%
  mutate(
    budget = 'med',
    instances = map(instances, ~filter(.x, inst_n == 6))
  ) %>%
  mutate(problem_space = pmap(., as_metaopt_problem)) %>%
  pull(problem_space)

run_irace <- function(name, params) {
  set.seed(65487873)
  print(name)
  
  problems <- train_problems
  
  train_best_solver(
    problem_space = ProblemSpace(problems = problems),
    algorithm = Algorithm(
      name = 'IG',
      parameters = readParameters(text = params)
    ),
    solve_function = fsp_solver_performance,
    irace_scenario = defaultScenario(list(
      maxExperiments = 5000
    )),
    parallel = 8,
    cache = here(irace_folder, paste(name, "rds", sep = ".")),
    recover = T
  )
}

strategy_params <- tribble(
  ~name, ~params,
  'ts', '
  IG.AOS.Strategy              "" c (thompson_sampling)
  IG.AOS.TS.Strategy           "" c (static, dynamic)
  IG.AOS.TS.C                  "" i (1,500)  | IG.AOS.TS.Strategy == "dynamic"
  ',
  'pm', '
  IG.AOS.Strategy              "" c (probability_matching)
  IG.AOS.PM.RewardType         "" c (avgabs,avgnorm,extabs,extnorm)
  IG.AOS.PM.Alpha              "" r (0.1, 0.9)
  IG.AOS.PM.PMin               "" r (0.05, 0.2)
  IG.AOS.PM.UpdateWindow       "" i (1,500)
  ',
  'frrmab', '
  IG.AOS.Strategy              "" c (frrmab)
  IG.AOS.FRRMAB.WindowSize     "" i (10, 500)
  IG.AOS.FRRMAB.Scale          "" r (0.01, 100)
  IG.AOS.FRRMAB.Decay          "" r (0.25, 1.0)
  ',
  'linucb', '
  IG.AOS.Strategy              "" c (linucb)
  IG.AOS.LINUCB.Alpha          "" r (0.0, 1.5)
  '
) %>% 
  crossing(
    tribble(
      ~ig_variant, ~base_config,
      'ig', '
          IG.Init                            "" c (neh)
          IG.Init.NEH.Ratio                  "" c (0)
          IG.Init.NEH.Priority               "" c (sum_pij)
          IG.Init.NEH.PriorityOrder          "" c (incr)
          IG.Init.NEH.PriorityWeighted       "" c (no)
          IG.Init.NEH.Insertion              "" c (last_best)
          IG.Comp.Strat                      "" c (strict)
          IG.Neighborhood.Size               "" c (1.0)
          IG.Neighborhood.Strat              "" c (ordered)
          IG.LS.Single.Step                  "" c (1)
          IG.Accept                          "" c (better)
          IG.Accept.Better.Comparison        "" c (strict)
          IG.Perturb.Insertion               "" c (random_best)
          IG.Perturb                         "" c (rs)
          IG.Local.Search                    "" c (none)
          
          IG.Perturb.DestructionSizeStrategy "" c (adaptive)
          IG.Perturb.DestructionSize         "" c (4)
          IG.DestructionStrategy             "" c (random)
      
          IG.AOS.Options "" c (1_2, 1_4, 1_2_4)
          IG.AOS.RewardType "" c (0,1,2,3)
          IG.AOS.WarmUp  "" c (0,1000,10000)
          IG.AOS.WarmUp.Strategy "" c (random)
      ',
      'ig-lsps', '
          IG.Init                            "" c (neh)
          IG.Init.NEH.Ratio                  "" c (0)
          IG.Init.NEH.Priority               "" c (sum_pij)
          IG.Init.NEH.PriorityOrder          "" c (incr)
          IG.Init.NEH.PriorityWeighted       "" c (no)
          IG.Init.NEH.Insertion              "" c (last_best)
          IG.Comp.Strat                      "" c (strict)
          IG.Neighborhood.Size               "" c (1.0)
          IG.Neighborhood.Strat              "" c (ordered)
          IG.LS.Single.Step                  "" c (1)
          IG.Accept                          "" c (better)
          IG.Accept.Better.Comparison        "" c (strict)
          IG.Accept.Temperature              "" c (0.25)
          IG.Perturb.Insertion               "" c (first_best)
          IG.Local.Search                    "" c (none)
          IG.Perturb                         "" c (lsps)

          IG.LSPS.Local.Search               "" c (best_insertion)
          IG.LSPS.Single.Step                "" c (1)
          
          IG.Perturb.DestructionSizeStrategy "" c (adaptive)
          IG.Perturb.DestructionSize         "" c (4)
          IG.DestructionStrategy             "" c (random)
      
          IG.AOS.Options "" c (1_2, 1_4, 1_2_4)
          IG.AOS.RewardType "" c (0,1,2,3)
          IG.AOS.WarmUp  "" c (0,100,1000,10000)
          IG.AOS.WarmUp.Strategy "" c (random)
      '
    )
  ) %>%
  mutate(
    name = paste0(ig_variant, '-adaptive-ds-', name),
    params = paste(base_config, params)
  ) %>%
  mutate(best_config = map2(name, params, run_irace))


strategy_params %>%
  mutate(best_config = map(best_config, ~.x[1,])) %>%
  mutate(perf = map2(name, best_config, function(name, config) {
    set.seed(79879874)
    sample_performance(
      algorithm = get_algorithm("IG"),
      problemSpace = ProblemSpace(problems = test_problems),
      config = df_to_character(config),
      solve_function = fsp_solver_performance,
      no_samples = 10,
      cache = file.path(perf_folder, paste0(name, "-tuned-perf.rds"))
    )
  }))



set.seed(79879874)
perfs <- sample_performance(
  algorithm = get_algorithm("IG"),
  problemSpace = ProblemSpace(problems = test_problems),
  config = c(
    IG.Init                            = "neh",
    IG.Init.NEH.Ratio                  = "0",
    IG.Init.NEH.Priority               = "sum_pij",
    IG.Init.NEH.PriorityOrder          = "incr",
    IG.Init.NEH.PriorityWeighted       = "no",
    IG.Init.NEH.Insertion              = "last_best",
    IG.Comp.Strat                      = "strict",
    IG.Neighborhood.Size               = "1.0",
    IG.Neighborhood.Strat              = "ordered",
    IG.Local.Search                    = "best_insertion",
    IG.LS.Single.Step                  = "0",
    IG.Accept                          = "temperature",
    IG.Accept.Better.Comparison        = "strict",
    IG.Accept.Temperature              = "0.25",
    IG.Perturb                         = "rs",
    IG.Perturb.Insertion               = "random_best",
    IG.Perturb.DestructionSizeStrategy = "fixed",
    IG.Perturb.DestructionSize         = "4",
    IG.DestructionStrategy             = "random"
  ),
  solve_function = fsp_solver_performance,
  no_samples = 10,
  cache = file.path(perf_folder, paste0("ig-def-perf.rds"))
)



set.seed(79879874)
perfs <- sample_performance(
  algorithm = get_algorithm("IG"),
  problemSpace = ProblemSpace(problems = test_problems),
  config = c(
    IG.Init                            = "neh",
    IG.Init.NEH.Ratio                  = "0",
    IG.Init.NEH.Priority               = "sum_pij",
    IG.Init.NEH.PriorityOrder          = "incr",
    IG.Init.NEH.PriorityWeighted       = "no",
    IG.Init.NEH.Insertion              = "last_best",
    IG.Comp.Strat                      = "strict",
    IG.Neighborhood.Size               = "1.0",
    IG.Neighborhood.Strat              = "ordered",
    IG.Local.Search                    = "best_insertion",
    IG.LS.Single.Step                  = "0",
    IG.Accept                          = "temperature",
    IG.Accept.Better.Comparison        = "strict",
    IG.Accept.Temperature              = "0.25",
    IG.Perturb                         = "lsps",
    IG.Perturb.Insertion               = "random_best",
    IG.Perturb.DestructionSizeStrategy = "fixed",
    IG.DestructionStrategy             = "random",
    IG.Perturb.DestructionSize         = "2",
    IG.LSPS.Local.Search               = "best_insertion",
    IG.LSPS.Single.Step                = "0"
  ),
  solve_function = fsp_solver_performance,
  no_samples = 10,
  cache = file.path(perf_folder, paste0("ig-lsps-def-perf.rds"))
)

