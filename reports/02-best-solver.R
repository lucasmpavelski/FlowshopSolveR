library(irace)
library(tidyverse)
library(FlowshopSolveR)
library(here)
library(metaOpt)

problems_dt <- train_problems_df() %>%
  filter(budget == 'low')

instances_dt <- problems_dt %>%
  unnest(instances)
instances_idx <- as.list(1:nrow(problems_dt))

problem_space <- ProblemSpace(problems = list(Problem(
  name = "all",
  instances = instances_idx,
  data = list(problems_dt)
)))

algorithm <- get_algorithm('NEH')
default_neh <- default_configs('NEH')
algorithm_space <- AlgorithmSpace(algorithms = list(algorithm))

cache_folder <- here('runs', 'neh_best_solver')
dir.create(cache_folder, showWarnings = F)


best_solver_performance <- function(algorithm, config, instance, problem, seed, ...) {
  instance_dt <- instances_dt[instance,]
  problem <- Problem(
    name = paste(instance_dt$model, instance_dt$instance_features, sep = ','),
    instances = list(instance_dt$instance),
    data = as.list(instance_dt)
  )
  fsp_solver_performance(algorithm, config, instance_dt$instance, problem, seed, ...)
}

library(future)
plan(multisession)

logFile <- file.path(cache_folder, 'all', 'NEH', 'log.Rdata')
recFile <- file.path(cache_folder, 'all', 'NEH', 'recover.Rdata')
if (file.exists(logFile)) {
  file.copy(logFile, recFile, overwrite = T)
} else {
  recFile <- ''
}

options(future.rng.onMisuse = 'ignore')

irace_trained <- build_performance_data(
  problem_space = problem_space,
  algorithm_space = algorithm_space,
  solve_function = best_solver_performance,
  irace_scenario = defaultScenario(list(
    deterministic = 1,
    maxExperiments = 5000 * nrow(problems_dt),
    initConfigurations = default_neh,
    recoveryFile = recFile
  )),
  cache_folder = cache_folder,
  parallel = 8
)
