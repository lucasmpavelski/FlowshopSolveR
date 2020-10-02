
# auxiliar functions
parseParams <- function(params) {
  params <- str_split(params, "\\s", simplify = T) %>%
    str_split("=", simplify = T)
  values <- params[, 2]
  names(values) <- params[, 1]
  values
}

fsp_solve_process <- function(problem, mh, config, seed, core) {
  exe_bin <- "taskset"
  args <- c(
    "-c",
    core,
    here("_install", "main", "fsp_solver"),
    paste0("--data_folder=", here("data")),
    paste0("--mh=", mh),
    paste0("--seed=", seed),
    paste0("--", names(problem), "=", problem),
    paste0("--", names(config), "=", config),
    "--printBestFitness"
  )
  data <- system2(exe_bin, args, stdout = TRUE)
  dt <- str_split(last(data), ",", simplify = T)
  if (ncol(dt) < 2) {
    write_lines(paste('"', args, '",'), "errors.txt", append = T)
    write_lines(paste("output:", data), "errors.txt", append = T)
    write_lines(paste(args, collapse = " "), "errors.txt", append = T)
    return(999999999999)
  } else {
    as.integer(dt[, 2])
  }
}

fspTargetRunnerCmdSequential <- function(experiments_dt) {
  experiments_dt %>%
    pmap(., solveCmd)
}

fspTargetRunnerCmdParallel <- function(experiments, scenario, config, ncores, ...) {
  problems <- scenario$targetRunnerData
  expetiments_dt <- tibble(
    problem = map(experiments, ~ problems[.x$instance, ]),
    config = map(experiments, ~ .x$config),
    seed = map_int(experiments, ~ .x$seed),
    core = rep(1:ncores, length.out = length(experiments))
  )
  experiments_futures <- expetiments_dt %>%
    group_split(core) %>%
    map(function(exp_dt) {
      # fspTargetRunnerCmdSequential(exp_dt)
      futureCall(fspTargetRunnerCmdSequential,
        args = list(experiments_dt = exp_dt)
      )
    })
  experiments_futures %>%
    map(value) %>%
    unlist() %>%
    map(~ list(time = 0, cost = .x))
}
