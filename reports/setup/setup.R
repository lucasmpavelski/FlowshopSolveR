library(FlowshopSolveR)
library(tidyverse)
library(here)
library(wrapr)
library(irace)
library(furrr)

NCORES <- 16
options(parallelly.debug = TRUE)
plan(remote, workers = rep("linode2", NCORES), persistent = TRUE)

all_problems <- all_problems_df() %>%
  filter(
    no_jobs %in% c(10, 30, 50),
    no_machines %in% c(5, 10),
    dist %in% c('uniform', 'exponential'),
    stopping_criterion == 'TIME',
    budget == 'high'
  ) %>%
  mutate(stopping_criterion = "FIXEDTIME") %>%
  mutate(meta_opt_problem = pmap(., as_metaopt_problem))

LOWER_BOUNDS_FOLDER <- here("data", "lower_bounds")
dir.create(LOWER_BOUNDS_FOLDER, FALSE, TRUE)

pwalk(all_problems, function(meta_opt_problem, id, ...) {
  print(id)
  print(meta_opt_problem)
  sample_performance(
    algorithm = get_algorithm("IG"),
    config = default_configs("IG"),
    problemSpace = ProblemSpace(problems = list(meta_opt_problem)),
    solve_function = fsp_solver_performance,
    no_samples = 30,
    cache = file.path(LOWER_BOUNDS_FOLDER, paste0(id, ".rds")),
    parallel = TRUE
  )
})

plan(sequential)
