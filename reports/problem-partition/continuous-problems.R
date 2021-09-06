library(cmaesr)
library(smoof)
library(cec2013)
library(checkmate)
library(FlowshopSolveR)
library(MOEADr)

# plan(sequential)
plan(multisession, workers = 8)

cecTags <- function(i) {
  checkNumber(i, lower = 1, upper = 24)
  if (i <= 5) 'unimodal'
  else if (i <= 20) 'multimodal'
  else 'composition'
}

cecGlobalOptimum <- function(i) {
  checkNumber(i, lower = 1, upper = 24)
  Filter(function(x) x != 0, seq(from = -1400, to = 1400, by = 100))[i]
}

makeCECFunction <- function(i, dimensions) {
  assertCount(i)
  force(i)
  assertCount(dimensions)
  force(dimensions)
  makeSingleObjectiveFunction(
    name = paste0(dimensions, "-d ", i, "-th CEC-2013 benchmark function"),
    id = paste0("cec2013_", i, "_", dimensions, "d"),
    fn = function(x) {
      assertNumeric(x,
                    len = dimensions,
                    any.missing = FALSE,
                    all.missing = FALSE)
      cec2013(i, x)
    },
    par.set = makeNumericParamSet(
      len = dimensions,
      id = "x",
      lower = rep(-100, dimensions),
      upper = rep(100, dimensions),
      vector = TRUE
    ),
    tags = cecTags(i),
    global.opt.value = cecGlobalOptimum(i)
  )
}

cmaes_params <- readParameters(text = '
sigma "" r (0.01, 150.0)
lambda "" i (4, 200)
mu_factor "" r (0.01, 1.0)
max.restarts "" i (0, 10)
noEffectAxisRestart "" c (no, yes) | max.restarts != 0
noEffectCoordRestart "" c (no, yes) | max.restarts != 0
conditionCovRestart "" c (no, yes) | max.restarts != 0
restart.multiplier "" i (1, 4) | max.restarts != 0
')

cec_problems <- tribble(
  ~function_number, ~dimensions,
  1, 10,
  2, 10,
  3, 10,
  4, 10,
  5, 10,
  6, 10,
  7, 10,
  8, 10,
  9, 10,
  10, 10
) %>%
  mutate(
    instances = pmap(., function(function_number, dimensions) 
        list(instance = paste(function_number, dimensions))),
    instance_features = map_chr(instances, ~paste(names(.x), '=', .x, collapse = ', ')),
    model = "cec2013"
  ) %>%
  mutate(meta_objective = case_when(
    function_number <= 5 ~ 1,
    T ~ 2
  )) %>%
  mutate(problem_space = pmap(., as_metaopt_problem))

solve_function <- function(problem, config, seed, ...) {
  fn <- makeCECFunction(problem@data$function_number, problem@data$dimensions)
  
  max.restarts <- as.integer(config['max.restarts'])
  restart.triggers <- c()
  if (max.restarts != 0 && config['noEffectAxisRestart'] == "yes") {
    restart.triggers <- c(restart.triggers, "noEffectAxis")
  }
  if (max.restarts != 0 && config['noEffectCoordRestart'] == "yes") {
    restart.triggers <- c(restart.triggers, "noEffectCoord")
  }
  if (max.restarts != 0 && config['conditionCovRestart'] == "yes") {
    restart.triggers <- c(restart.triggers, "conditionCov")
  }
  
  mu <- ceiling(as.integer(config['lambda']) * as.numeric(config['mu_factor']))
  
  restart.multiplier <- 2
  if (max.restarts != 0) {
    restart.multiplier <- as.integer(config['restart.multiplier'])
  }
  
  budget <- 10000 * getNumberOfParameters(fn)
  
  control <- list(
    sigma = as.numeric(config['sigma']),
    lambda = as.integer(config['lambda']),
    mu = mu,
    restart.triggers = restart.triggers,
    max.restarts = max.restarts,
    restart.multiplier = restart.multiplier,
    stop.ons = list(
      stopOnNoEffectAxis(),
      stopOnNoEffectCoord(), 
      stopOnCondCov(),
      stopOnMaxEvals(budget),
      stopOnOptValue(getGlobalOptimum(fn)$value, tol = 1e-08)
    )
  )
  
  # print(config)
  res <- cmaes(fn, control = control, monitor = NULL)
  
  cost <- res$best.fitness
  if (is.infinite(res$best.fitness)) {
    cost <- 1e60
  }
  
  list(
    cost = cost,
    time = res$past.time,
    no_evals = res$n.evals,
    res = res
  )
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

EXPERIMENTS <- list(
  "cmaes-cec2013" = list(
    strategy = "moead",
    # parameters
    parameter_space = cmaes_params,
    algorithm = Algorithm(name = "cmaes", parameters = cmaes_params),
    # problems
    problems = cec_problems,
    objectives = c("1", "2"),
    eval_problems = cec_problems,
    eval_no_samples = 6,
    # moead parameters
    moead_variation = "irace",
    moead_decomp = list(name = "SLD", H = 7),
    moead_neighbors = list(name = "lambda", T = 2, delta.p = 1),
    moead_max_iter = 50,
    # irace variation
    irace_variation_problems = cec_problems,
    irace_variation_no_evaluations = 100,
    irace_variation_no_samples = 4
  ),
  "cmaes-cec2013-ga" = list(
    strategy = "moead",
    # parameters
    parameter_space = cmaes_params,
    algorithm = Algorithm(name = "cmaes", parameters = cmaes_params),
    # problems
    problems = cec_problems,
    objectives = c("1", "2"),
    eval_problems = cec_problems,
    eval_no_samples = 6,
    # moead parameters
    moead_variation = "ga",
    moead_decomp = list(name = "SLD", H = 7),
    moead_neighbors = list(name = "lambda", T = 2, delta.p = 1),
    moead_max_iter = 134 
  ),
  "cmaes-cec2013-irace" = list(
    strategy = "irace",
    # parameters
    parameter_space = cmaes_params,
    algorithm = Algorithm(name = "cmaes", parameters = cmaes_params),
    # problems
    problems = cec_problems,
    # irace parameters
    irace_max_evals = 64000
  )
)

EXP_NAME <- "cmaes-cec2013-irace"
EXP <- EXPERIMENTS[[EXP_NAME]]

if (EXP$strategy == "irace") {
  results <- train_best_solver(
    problem_space = ProblemSpace(problems = EXP$problems$problem_space),
    algorithm = EXP$algorithm,
    solve_function = solve_function,
    irace_scenario = defaultScenario(
      list(
        maxExperiments = EXP$irace_max_evals,
        minNbSurvival = 1
      )
    ),
    parallel = 8,
    quiet = F,
    cache = here("data", "problem_partition", EXP_NAME),
    recover = TRUE
  )
} else if (EXP$strategy == "moead") {
  test_problems_eval <- make_performance_sample_evaluation(
    algorithm = EXP$algorithm,
    problem_space = ProblemSpace(problems = EXP$eval_problems$problem_space),
    solve_function = solve_function,
    no_samples = EXP$eval_no_samples,
    parallel = T,
    aggregate_performances = aggregate_by_ert,
    cache_folder = here("data", "problem_partition", EXP_NAME)
  )
  
  moead_problem <- list(
    name   = "test_problems_eval",
    xmin   = real_lower_bounds(EXP$algorithm@parameters, fixed = FALSE),
    xmax   = real_upper_bounds(EXP$algorithm@parameters, fixed = FALSE),
    m      = length(EXP$objectives)
  )
  
  variation <- NULL
  
  variation_irace <- NULL
  if (EXP$moead_variation == "irace") {
    variation_irace <<- make_irace_variation(
      algorithm = EXP$algorithm,
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
}
  
  



  