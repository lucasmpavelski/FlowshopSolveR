library(FlowshopSolveR)
library(tidyverse)
library(here)
library(wrapr)
library(irace)

# test_problems <- all_problems_df() %>%
#   filter(
#     problem %in% c('flowshop'),
#     budget == 'high',
#     type == 'PERM',
#     objective == 'MAKESPAN',
#     dist == 'uniform',
#     corr == 'random',
#     no_jobs %in% c(50, 200),
#   ) %>%
#   mutate(
#     budget = 'med' #,
#     # instances = map(instances, ~filter(.x, inst_n > 5))
#   ) %>%
#   mutate(problem_space = pmap(., as_metaopt_problem)) %>%
#   pull(problem_space)
# 
# plan(sequential)
# 
# set.seed(79879874)
# perfs <- sample_performance(
#   algorithm = get_algorithm("IG"), 
#   problemSpace = ProblemSpace(problems = test_problems),
#   config = c(
#     IG.Init                            = "neh",
#     IG.Init.NEH.Ratio                  = "0",
#     IG.Init.NEH.Priority               = "sum_pij",
#     IG.Init.NEH.PriorityOrder          = "incr",
#     IG.Init.NEH.PriorityWeighted       = "no",
#     IG.Init.NEH.Insertion              = "last_best",
#     IG.Comp.Strat                      = "strict",
#     IG.Neighborhood.Size               = "1.0",
#     IG.Neighborhood.Strat              = "ordered",
#     IG.Local.Search                    = "best_insertion",
#     IG.LS.Single.Step                  = "0",
#     IG.Accept                          = "temperature",
#     IG.Accept.Better.Comparison        = "strict",
#     IG.Accept.Temperature              = "0.25",
#     IG.Perturb                         = "rs",
#     IG.Perturb.Insertion               = "random_best",
#     IG.Perturb.DestructionSizeStrategy = "fixed",
#     IG.Perturb.DestructionSize         = "4",
#     IG.DestructionStrategy             = "random"
#   ),
#   solve_function = fsp_solver_performance,
#   no_samples = 30,
#   cache = here("reports", "aos", "data", "perf", paste0("ig-def_test.rds"))
# )
# 
# 
# set.seed(79879874)
# perfs <- sample_performance(
#   algorithm = get_algorithm("IG"), 
#   problemSpace = ProblemSpace(problems = test_problems),
#   config = c(
#     IG.Init                            = "neh",
#     IG.Init.NEH.Ratio                  = "0",
#     IG.Init.NEH.Priority               = "sum_pij",
#     IG.Init.NEH.PriorityOrder          = "incr",
#     IG.Init.NEH.PriorityWeighted       = "no",
#     IG.Init.NEH.Insertion              = "last_best",
#     IG.Comp.Strat                      = "strict",
#     IG.Neighborhood.Size               = "1.0",
#     IG.Neighborhood.Strat              = "ordered",
#     IG.Local.Search                    = "best_insertion",
#     IG.LS.Single.Step                  = "0",
#     IG.Accept                          = "temperature",
#     IG.Accept.Better.Comparison        = "strict",
#     IG.Accept.Temperature              = "0.25",
#     IG.Perturb                         = "rs",
#     IG.Perturb.Insertion               = "random_best",
#     IG.Perturb.DestructionSizeStrategy = "fixed",
#     IG.Perturb.DestructionSize         = "4",
#     IG.DestructionStrategy             = "random"
#   ),
#   solve_function = fsp_solver_performance,
#   no_samples = 30,
#   cache = here("reports", "aos", "data", "perf", paste0("ig-def_test.rds"))
# )

plan(multisession)

set.seed(79879874)
test_problems <- all_problems_df() %>%
  filter(
    problem %in% c('flowshop'),
    budget == 'high',
    type == 'PERM',
    objective == 'MAKESPAN',
    dist == 'uniform',
    corr == 'random',
    no_jobs %in% c(100, 200, 300),
  ) %>%
  mutate(
    instances = map(instances, ~filter(.x, inst_n == 6))
  ) %>%
  mutate(problem_space = pmap(., as_metaopt_problem)) %>%
  pull(problem_space)

save_folder <- here("reports", "aos", "data", "exp-02-single-step", "perf", "default")
dir.create(save_folder, recursive = T, showWarnings = F)

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
    IG.LS.Single.Step                  = "1",
    IG.Accept                          = "temperature",
    IG.Accept.Better.Comparison        = "strict",
    IG.Accept.Temperature              = "0.25",
    IG.Perturb                         = "lsps",
    IG.Perturb.Insertion               = "random_best",
    IG.Perturb.DestructionSizeStrategy = "fixed",
    IG.Perturb.DestructionSize         = "4",
    IG.DestructionStrategy             = "random",
    IG.LSPS.Local.Search               = "best_insertion",
    IG.LSPS.Single.Step                = "1"
  ),
  solve_function = fsp_solver_performance,
  no_samples = 10,
  cache = file.path(save_folder, paste0("test.rds"))
)


save_folder <- here("reports", "aos", "data", "exp-02-single-step", "perf", "default-original")
dir.create(save_folder, recursive = T, showWarnings = F)

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
    IG.Perturb.DestructionSize         = "2",
    IG.DestructionStrategy             = "random",
    IG.LSPS.Local.Search               = "best_insertion",
    IG.LSPS.Single.Step                = "0"
  ),
  solve_function = fsp_solver_performance,
  no_samples = 10,
  cache = file.path(save_folder, paste0("test.rds"))
)
