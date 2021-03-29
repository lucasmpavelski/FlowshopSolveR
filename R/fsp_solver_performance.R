df_to_character <- function(dataFrame) {
  values <- as.character(dataFrame)
  names(values) <- names(dataFrame)
  values
}


print_cpp <- function(config, var = "params") {
  names(config) %>%
    walk(~cat(paste0(var, '["', .x, '"] = "', config[.x], '"'), ';\n'))
}

fsp_solver_performance <- function(algorithm, config, instance, problem, seed, ...) {
  initFactories(here('data'))
  cf <- df_to_character(config)
  names(cf) <- str_replace_all(names(cf), '_', '.')
  prob <- unlist(problem@data)
  prob['instance'] <- instance
  res <- solveFSP(
    mh = algorithm@name,
    rproblem = prob,
    rparams = cf,
    seed = seed,
    verbose = F
  )
  list(
    cost = res$fitness,
    time = res$time
  )
}

fsp_solver_performance_debug <- function(algorithm, config, instance, problem, seed, ...) {
  initFactories(here('data'))
  cf <- df_to_character(config)
  names(cf) <- str_replace_all(names(cf), '_', '.')
  prob <- unlist(problem@data)
  prob['instance'] <- instance
  cat("seed = ", seed, ";\n")
  print_cpp(config, "params")
  print_cpp(prob, "prob")
  res <- solveFSP(
    mh = algorithm@name,
    rproblem = prob,
    rparams = cf,
    seed = seed,
    verbose = F
  )
  list(
    cost = res$fitness,
    time = res$time
  )
}