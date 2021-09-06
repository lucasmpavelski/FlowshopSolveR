library(bit)
library(MOEADr)
library(FlowshopSolveR)
library(here)
library(tidyverse)
library(furrr)
library(optparse)
library(Rcpp)

oneMax <- function(x) {
  sum(x)
}

jump <- function(x, k) {
  om <- oneMax(x)
  n <- length(x)
  if (om <= n - k || om == n) {
    om
  } else {
    n - om
  }
}

make_jump <- function(l) {
  function(x) jump(x, l)
}

mutate_rls <- function(x, k) {
  invert <- sample(length(x), k)
  xor(x, bitwhich(length(x), invert))
}

mutate_ea <- function(x, mu) {
  invert <- runif(length(x)) < mu
  xor(x, as.bit(invert))
}

ea_jump_cost <- function(size, m, mu, seed) {
  # sol <- as.bit(sample(2, size, replace=TRUE) == 1)
  # curr_fitness <- eval_func(sol)
  # mutate <- mutate_ea
  # steps <- 0
  # max_budget <- exp(1) * size ^ m
  # while (curr_fitness != size && steps <= max_budget) {
  #   sol_m <- mutate_ea(sol, mu)
  #   fitness_m <- eval_func(sol_m)
  #   if (curr_fitness < fitness_m)
  #     sol <- sol_m
  #   curr_fitness <- max(curr_fitness, fitness_m)
  #   steps <- steps + 1
  #   # cat(size, mu, curr_fitness, steps, '\n')
  # }
  # steps / max_budget
  cppEAJumpCost(size, m, mu, seed)
}


# plan(sequential)
plan(multisession, workers = 7)
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

parameter_space <- readParameters(
  text = '
mu "" r (0.01, 0.4)
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
  size <- problem@data$size
  l <- problem@data$l
  cost <- ea_jump_cost(
    size = size,
    m = l,
    mu = as.double(config['mu']),
    seed = seed
  )
  list(
    cost = cost,
    time = cost,
    no_evals = cost
  )
}

algorithm <- Algorithm(name = "EA (1+1)", parameters = parameter_space)

EXP_NAME <- "jump-ea-mutations-test-pbi"
EXP = list(
  # parameters
  parameter_space = parameter_space,
  algorithm = algorithm,
  # problems
  problems = jump_problems,
  objective_feature = "l",
  objectives = c("2", "3"),
  eval_problems = jump_problems,
  eval_no_samples = 30,
  # moead parameters
  moead_variation = "irace",
  moead_decomp = list(name = "SLD", H = 7),
  moead_neighbors = list(name = "lambda", T = 2, delta.p = 1),
  moead_max_iter = 50,
  # irace variation
  irace_variation_problems = jump_problems,
  irace_variation_no_evaluations = 100,
  irace_variation_no_samples = 30
)

test_problems_eval <- make_performance_sample_evaluation(
  algorithm = algorithm,
  problem_space = ProblemSpace(problems = EXP$eval_problems$problem_space),
  solve_function = solve_function,
  no_samples = EXP$eval_no_samples,
  parallel = T,
  aggregate_performances = arpf_by_objective,
  cache_folder = here("data", "problem_partition", EXP_NAME)
)

moead_problem <- list(
  name   = "test_problems_eval",
  xmin   = c(real_lower_bounds(parameter_space, fixed = FALSE), 0),
  xmax   = c(real_upper_bounds(parameter_space, fixed = FALSE), 1),
  m      = length(EXP$objectives)
)

variation <- NULL

variation_irace <- NULL
if (EXP$moead_variation == "irace") {
  variation_irace <<- make_irace_variation(
    algorithm = algorithm,
    problem_space = ProblemSpace(problems = EXP$irace_variation_problems$problem_space),
    solve_function = solve_function,
    irace_scenario = defaultScenario(
      list(
        maxExperiments = EXP$irace_variation_no_evaluations,
        minNbSurvival = 1
      )
    ),
    no_samples = EXP$irace_variation_no_samples,
    cache_folder = here("data", "problem_partition", EXP_NAME)
  )
  variation <<- list(list(name = "irace"))
} else if (EXP$moead_variation == "ga") {
  variation <<- preset_moead("original")$variation
}

results <- moead(
  preset   = preset_moead("original"),
  problem  = moead_problem,
  variation = variation,
  decomp = EXP$moead_decomp,
  showpars = list(show.iters = "numbers", showevery = 1),
  neighbors = EXP$moead_neighbors,
  stopcrit = list(list(name  = "maxiter",
                       maxiter  = EXP$moead_max_iter)),
  seed     = 42
)

