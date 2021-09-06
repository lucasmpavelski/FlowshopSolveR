wrap_irace_target_runner <- function(solve) {
  function(experiment, scenario, ...) {
    problem <- scenario$targetRunnerData$problem
    instance <- experiment$instance
    if (length(problem) > 1) {
      problem <- Filter(function(p) any(p@instances == instance), scenario$targetRunnerData$problem)[[1]]
    }
    solve(
      algorithm = scenario$targetRunnerData$algorithm,
      problem = problem,
      config = experiment$configuration,
      instance = instance,
      seed = experiment$seed
    )
  }
}

wrap_irace_target_runner_parallel <- function(solve, ncores) {
  wrap_solve <- wrap_irace_target_runner(solve)
  function(experiments, scenario, ...) {
    tibble(experiment = experiments, scenario = list(scenario)) %>%
      furrr::future_pmap(wrap_solve)
  }
}
