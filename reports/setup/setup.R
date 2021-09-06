library(FlowshopSolveR)
library(tidyverse)
library(here)
library(wrapr)
library(irace)
library(furrr)

plan(sequential)
# NCORES <- 48
# options(parallelly.debug = TRUE)
# plan(multisession)
# plan(remote, workers = rep("linode2", NCORES), persistent = TRUE)

all_problems <- all_problems_df() %>%
  filter(
    no_jobs %in% c(100),
    no_machines %in% c(20),
    dist %in% c('uniform', 'exponential'),
    stopping_criterion == 'TIME',
    budget == 'high'
  ) %>%
  mutate(stopping_criterion = "FIXEDTIME") %>%
  mutate(instances = map(instances, ~filter(.x, inst_n > 6))) %>%
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
    cache = file.path(LOWER_BOUNDS_FOLDER, paste0(id, '-', paste0(meta_opt_problem@instances, collapse = '-'), ".rds")),
    parallel = TRUE
  )
})

plan(sequential)


lower_bound_cache <- map(dir(LOWER_BOUNDS_FOLDER, full.names = T), read_rds) %>%
  bind_rows() %>%
  mutate(
    problem_data = map(problem, ~.x@data),
    dist = map_chr(problem_data, 'dist'),
    corr = map_chr(problem_data, 'corr'),
    no_jobs = as.integer(map_chr(problem_data, 'no_jobs')),
    no_machines = as.integer(map_chr(problem_data, 'no_machines')),
    problem_ = map_chr(problem_data, 'problem'),
    type = map_chr(problem_data, 'type'),
    objective = map_chr(problem_data, 'objective'),
    cost = as.integer(map_dbl(result, 'cost')),
    time = as.integer(map_dbl(result, 'time')),
    no_evals = as.integer(map_dbl(result, 'no_evals'))
  ) %>%
  select(-problem, 
         -problem_data,
         problem = problem_,
         -result)


lower_bounds <- lower_bound_cache %>%
  group_by(dist, corr, no_jobs, no_machines, problem, type, objective, instance) %>%
  summarise(
    cost = min(cost),
    time = sum(time),
    no_evals = sum(no_evals)
  )
  
write_csv(lower_bounds, here("data", "lower_bounds.csv"))
  