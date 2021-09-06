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

jump <- function(x, m) {
  om <- oneMax(x)
  n <- length(x)
  if ((m < om && om < n - m) || (om == n)) om else 0
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

ea_cost <- function(size, m, mu, eval_func) {
  sol <- as.bit(sample(2, size, replace=TRUE) == 1)
  curr_fitness <- eval_func(sol)
  mutate <- mutate_ea
  steps <- 0
  max_budget <- exp(1) * size ^ m
  while (curr_fitness != size && steps <= max_budget) {
    sol_m <- mutate_ea(sol, mu)
    fitness_m <- eval_func(sol_m)
    if (curr_fitness < fitness_m)
      sol <- sol_m
    curr_fitness <- max(curr_fitness, fitness_m)
    steps <- steps + 1
    # cat(size, mu, curr_fitness, steps, '\n')
  }
  steps / max_budget
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
  # 2,	10,	#0.2	100
  # 3,	10,	#0.3	1000
  # 4,	10,	#0.4	10000
  # 3,	20,	#0.15	8000
  # 3,	25,	#0.12	15625
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
  set.seed(seed)
  size <- problem@data$size
  l <- problem@data$l
  cost <- ea_cost(
    size = size,
    m = l,
    mu = as.double(config['mu']),
    eval_func = make_jump(l)
  )
  list(
    cost = cost,
    time = cost,
    no_evals = cost
  )
}

algorithm <- Algorithm(name = "EA (1+1)", parameters = parameter_space)

EXP_NAME <- "jump-ea-mutations"
EXP = list(
  # parameters
  parameter_space = parameter_space,
  algorithm = algorithm,
  # problems
  problems = jump_problems,
  objective_feature = "l",
  objectives = c("2", "3"),
  eval_problems = jump_problems,
  eval_no_samples = 10,
  # moead parameters
  moead_variation = "irace",
  moead_decomp = list(name = "SLD", H = 7),
  moead_neighbors = list(name = "lambda", T = 2, delta.p = 1),
  moead_max_iter = 50,
  # irace variation
  irace_variation_problems = jump_problems,
  irace_variation_no_evaluations = 100,
  irace_variation_no_samples = 10
)

best_solvers_obj1 <- train_best_solver(
  problem_space = ProblemSpace(problems = EXP$eval_problems$problem_space),
  algorithm = EXP$algorithm,
  solve_function = solve_function,
  irace_scenario = defaultScenario(
    list(
      maxExperiments = 50 * (8 * (100 + 10)) / 2
    )
  ),
  parallel = 7,
  quiet = F,
  cache = here("data", "problem_partition", EXP_NAME, "irace-obj1.rda"),
  recover = TRUE
)
