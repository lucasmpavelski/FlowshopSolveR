library(FlowshopSolveR)
library(tidyverse)
library(here)
library(wrapr)
library(irace)

plan(multisession)

EXPERIMENT_NAME <- 'exp-02-single-step'
EXP_FOLDER <- here("reports", "aos", "data", EXPERIMENT_NAME)
UPDATE_CACHE <- F

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


compute_test_performance <- function(folder, config) {
  dir.create(folder, recursive = T, showWarnings = F)
  cache_file <- file.path(folder, "test.rds")
  if (file.exists(cache_file) && UPDATE_CACHE)
    file.remove(cache_file)
  set.seed(79879874)
  cat('.')
  sample_performance(
    algorithm = get_algorithm("IG"), 
    problemSpace = ProblemSpace(problems = test_problems),
    config = config,
    solve_function = fsp_solver_performance,
    no_samples = 10,
    cache = cache_file
  )
}

perfs <- crossing(
    adapt_strategies = qc(probability_matching,frrmab,linucb,thompson_sampling),
    adapt_type = qc(pos)
  ) %>%
  mutate(
    irace_result_file = here("reports", "aos", "data", EXPERIMENT_NAME, "irace", 
                             paste0("ig_ss_lsps-", adapt_type, "_", adapt_strategies, ".rds"))
  ) %>%
  filter(file.exists(irace_result_file)) %>%
  mutate(
    irace_result = map(irace_result_file, read_rds),
    irace_result = map(irace_result, removeConfigurationsMetaData),
    irace_result = map(irace_result, ~df_to_character(.x[1,])),
    folder = file.path(EXP_FOLDER, "perf", adapt_type, adapt_strategies)
  ) %>%
  mutate(perf = map2(folder, irace_result, compute_test_performance))

write_rds(perfs, file.path(EXP_FOLDER, "perf", "irace-perfs.rds"))
