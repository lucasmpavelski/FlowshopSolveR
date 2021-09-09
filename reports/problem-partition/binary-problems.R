library(FlowshopSolveR)
library(tidyverse)
library(here)


plan(sequential)
# plan(multisession, workers = 7)
# plan(remote,
#      workers = rep("linode2", 32),
#      persistent = TRUE)
# 
# 
# 
# set.seed(54654654)
# 

arpf_by_objective <- function(perfs) {
  perfs %>% unnest(perf) %>%
    unnest(result) %>%
    unnest(result) %>%
    mutate(meta_objective = as.integer(map_dbl(problem, ~.x@data$meta_objective))) %>%
    select(result, meta_objective, conf_id) %>%
    group_by(meta_objective, conf_id) %>%
    summarise(performance = mean(result))
}


mean_if_present <- function(x) {
  ifelse(length(x) > 0, mean(x), 0)
}

ert <- function(data) {
  success_rate <- ((data[[1]] %>% pull(success) %>% sum()) / nrow(data[[1]])) + 1e-6
  success_evals <- data[[1]] %>% filter(success) %>% pull(no_evals) %>% mean_if_present()
  fail_evals <- data[[1]] %>% filter(!success) %>% pull(no_evals) %>% mean_if_present()
  success_evals + ((1 - success_rate) / success_rate) * fail_evals
}

aggregate_by_ert <- function(sample) {
  sample %>%
    sample %>% select(conf_id, perf) %>%
    unnest(perf) %>%
    select(-seed, -instance) %>%
    mutate(tolerance = 1e-8) %>%
    unnest(tolerance) %>%
    mutate(
      meta_objective = map_int(problem, ~as.integer(.x@data$meta_objective)),
      no_evals = map_int(result, ~as.integer(.x$no_evals)),
      fitness = map_dbl(result, ~.x$cost),
      optimal = map_dbl(problem, ~cecGlobalOptimum(.x@data$function_number)),
      fitness_delta = fitness - optimal,
      success = fitness_delta <= tolerance
    ) %>%
    select(meta_objective, conf_id, no_evals, success) %>%
    group_by(meta_objective, conf_id) %>%
    nest() %>%
    mutate(performance = ert(data)) %>%
    select(-data) %>%
    ungroup()
}


parameter_space <- readParameters(
  text = '
mu "" r (0.01, 0.4)
dummy "" c (0,1)
  '
)

jump_problems <- tribble(
  ~l, ~size, # mutation, budget
  2,	10,	#0.2	100
  3,	10,	#0.3	1000
  4,	10,	#0.4	10000
  3,	20,	#0.15	8000
  3,	25,	#0.12	15625
  2,	20,	#0.1	400
  2,	30,	#0.067	900
  2,	50,	#0.04	2500
  2,	100,	#0.02	10000
  2,	200,	#0.01	40000
) %>%
  mutate(
    instances = pmap(., function(l, size) list(instance = list(l = l, size = size))),
    instance_features = map_chr(instances, ~paste(names(.x), '=', .x, collapse = ', ')),
    model = "jump"
  ) %>%
  mutate(meta_objective = case_when(
    l / size <= .1 ~ 1,
    T ~ 2
  )) %>%
  mutate(problem_space = pmap(., as_metaopt_problem))

solve_function <- function(problem, config, seed, ...) {
  res <- cppEAJumpCost(
    size = problem@data$size,
    k = problem@data$l,
    mu = as.double(config['mu']),
    seed = seed
  )
  res$cost <- problem@data$size - res$cost
  res
}

algorithm <- Algorithm(name = "EA (1+1)", parameters = parameter_space)

experiments <- tribble(
    ~name, ~experiment_data,
    "jump-ea-mutations-ert", list(
      strategy = "moead",
      # parameters
      algorithm = algorithm,
      eval_problems = jump_problems,
      solve_function = solve_function,
      # moead parameters
      aggregation_function = aggregate_by_ert,
      eval_no_samples = 30,
      moead_variation = "irace",
      moead_decomp = list(name = "SLD", H = 7),
      moead_neighbors = list(name = "lambda", T = 2, delta.p = 1),
      moead_max_iter = 50,
      # irace variation
      irace_variation_problems = jump_problems,
      irace_variation_no_evaluations = 100,
      irace_variation_no_samples = 30
    ),
    "jump-ea-mutations-ga-ert", list(
      strategy = "moead",
      # parameters
      algorithm = algorithm,
      eval_problems = jump_problems,
      solve_function = solve_function,
      # moead parameters
      aggregation_function = aggregate_by_ert,
      eval_no_samples = 30,
      moead_variation = "ga",
      moead_decomp = list(name = "SLD", H = 7),
      moead_neighbors = list(name = "lambda", T = 2, delta.p = 1),
      moead_max_iter = 67
    ),
    "jump-ea-mutations-irace-ert", list(
      strategy = "irace",
      # parameters
      algorithm = algorithm,
      eval_problems = jump_problems,
      solve_function = solve_function,
      # irace parameters
      irace_max_evals = 160000
    )
  ) %>%
    mutate(
      configs = pmap(., run_experiment, 
                     folder = here("data", "problem_partition"))
    ) %>%
    mutate(
      validation = pmap(., run_validation, 
                        folder = here("data", "problem_partition"),
                        aggregation_function = aggregate_by_ert)
    )

experiments %>%
  mutate(perfs = map(validation, 'perf')) %>% 
  select(name, perfs) %>% 
  unnest(perfs) %>%
  pivot_wider(names_from = meta_objective, values_from = performance) %>%
  ggplot() +
  geom_point(aes(x = `1`, y = `2`, color = name))
