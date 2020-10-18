library(irace)
library(tidyverse)
library(FlowshopSolveR)
library(here)
library(metaOpt)

as_metaopt_problem <- function(model, instance_features, instances, ...) {
  Problem(
    name = paste(model, instance_features, sep = ','),
    instances = as.list(instances$instance),
    data = list(...)
  )
}

problems_dt <- all_problems_df()
problem_space <- ProblemSpace(problems = pmap(problems_dt, as_metaopt_problem))
algorithm <- get_algorithm('NEH')
default_neh <- default_configs('NEH')
algorithm_space <- AlgorithmSpace(algorithms = list(algorithm))

cache_folder <- here('runs', 'neh')
dir.create(cache_folder, showWarnings = F)

library(future)
plan(multisession)

irace_trained <- build_performance_data(
  problem_space = problem_space,
  algorithm_space = algorithm_space,
  solve_function = fsp_solver_performance,
  irace_scenario = defaultScenario(list(
    deterministic = 1,
    maxExperiments = 5000,
    initConfigurations = default_neh
  )),
  cache_folder = cache_folder,
  parallel = 7
)
