train_best_solver <- function(
                              problem_space,
                              algorithm,
                              solve_function,
                              irace_scenario = irace::defaultScenario(),
                              parallel = 1,
                              quiet = FALSE,
                              cache = NA,
                              recover = FALSE) {
  inst_scenario <- irace_scenario
  inst_scenario$instances <- unlist(map(problem_space@problems, ~ .x@instances))
  inst_scenario$targetRunnerData <- list(
    problem = problem_space@problems,
    algorithm = algorithm
  )
  if (parallel <= 1) {
    inst_scenario$targetRunner <- wrap_irace_target_runner(solve_function)
  } else {
    inst_scenario$targetRunnerParallel <- wrap_irace_target_runner_parallel(solve_function, parallel)
  }
  parameters <- algorithm@parameters

  if (recover && file.exists(paste0(cache, "_log.Rdata"))) {
    inst_scenario$recoveryFile <- paste0(cache, "_rec.Rdata")
    file.copy(paste0(cache, "_log.Rdata"), inst_scenario$recoveryFile, overwrite = T)
  }

  result_file <- NULL
  if (!is.na(cache)) {
    inst_scenario$logFile <- paste0(cache, "_log.Rdata")
    if (file.exists(cache)) {
      return(readRDS(cache))
    }
  } else {
    inst_scenario$logFile <- "/dev/null"
  }

  if (quiet) {
    tunning_result <- quietly(irace)(inst_scenario, parameters)$result
  } else {
    tunning_result <- irace(inst_scenario, parameters)
  }

  if (!is.na(cache)) {
    saveRDS(tunning_result, file = cache)
  }

  tunning_result
}
