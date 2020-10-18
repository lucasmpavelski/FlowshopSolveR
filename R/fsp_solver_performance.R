df_to_character <- function(dataFrame) {
  values <- as.character(dataFrame)
  names(values) <- names(dataFrame)
  values
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