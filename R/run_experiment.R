run_experiment <- function(experiment_data, name, folder) {
  dir.create(folder, showWarnings = F, recursive = T)
  if (experiment_data$strategy == "irace") {
    irace_experiment(experiment_data, name, folder)
  } else if (experiment_data$strategy == "moead") {
    moead_experiment(experiment_data, name, folder)
  }
}

irace_experiment <- function(experiment_data, name, folder) {
  experiment_dir <- file.path(folder, name)
  dir.create(experiment_dir, showWarnings = F, recursive = T)
  train_best_solver(
    problem_space = ProblemSpace(problems = experiment_data$eval_problems$problem_space),
    algorithm = experiment_data$algorithm,
    solve_function = experiment_data$solve_function,
    irace_scenario = defaultScenario(
      list(maxExperiments = experiment_data$irace_max_evals)
    ),
    parallel = 8,
    quiet = F,
    cache = file.path(experiment_dir, "result.rds"),
    recover = TRUE
  ) |>
    removeConfigurationsMetaData()
}

make_variation <- function(experiment_data, name, folder) {
  if (experiment_data$moead_variation == "irace") {
    variation_irace <- make_irace_variation(
      algorithm = experiment_data$algorithm,
      problem_space = ProblemSpace(problems = experiment_data$irace_variation_problems$problem_space),
      solve_function = experiment_data$solve_function,
      irace_scenario = defaultScenario(
        list(maxExperiments = experiment_data$irace_variation_no_evaluations)
      ),
      no_samples = experiment_data$irace_variation_no_samples,
      cache_folder = file.path(folder, name)
    )
    
    assign("variation_irace", variation_irace, envir = .GlobalEnv)
    list(list(name = "irace"))
  } else if (experiment_data$moead_variation == "ga") {
    variation <<- preset_moead("original")$variation
  }
}

moead_experiment <- function(experiment_data, name, folder) {  
  test_problems_eval <- make_performance_sample_evaluation(
    algorithm = experiment_data$algorithm,
    problem_space = ProblemSpace(problems = experiment_data$eval_problems$problem_space),
    solve_function = experiment_data$solve_function,
    no_samples = experiment_data$eval_no_samples,
    parallel = T,
    aggregate_performances = experiment_data$aggregation_function,
    cache_folder = file.path(folder, name)
  )
  
  assign("test_problems_eval", test_problems_eval, envir = .GlobalEnv)
  
  moead_problem <- list(
    name   = "test_problems_eval",
    xmin   = real_lower_bounds(experiment_data$algorithm@parameters, fixed = FALSE),
    xmax   = real_upper_bounds(experiment_data$algorithm@parameters, fixed = FALSE),
    m      = n_distinct(experiment_data$eval_problems$meta_objective)
  )
  
  result <- moead(
    preset   = preset_moead("original"),
    problem  = moead_problem,
    variation = make_variation(experiment_data, name, folder),
    decomp = experiment_data$moead_decomp,
    showpars = list(show.iters = "numbers", showevery = 1),
    neighbors = experiment_data$moead_neighbors,
    stopcrit = list(list(name  = "maxiter",
                         maxiter  = experiment_data$moead_max_iter)),
    seed     = 42
  )
  
  population_to_configs(result$X, experiment_data$algorithm@parameters)
}