
normalize_population <- function(X, problem) {
  LL <- matrix(rep(problem$xmin, nrow(X)),
               ncol = ncol(X),
               byrow = TRUE)
  UL <- matrix(rep(problem$xmax, nrow(X)),
               ncol = ncol(X),
               byrow = TRUE)
  (X - LL) / (UL - LL)
}

denormalize_population <- function(X, problem) {
  LL <- matrix(rep(problem$xmin, nrow(X)),
               ncol = ncol(X),
               byrow = TRUE)
  UL <- matrix(rep(problem$xmax, nrow(X)),
               ncol = ncol(X),
               byrow = TRUE)
  LL + X * (UL - LL)
}

configs_to_population <- function(configs, parameter_space) {
  configs <- configs[, get_not_fixed(parameter_space)]
  for (param in get_categorical_not_fixed(parameter_space)) {
    domain <- parameter_space$domain[[param]]
    configs <- configs %>%
      rowwise() %>%
      mutate_at(param, ~ which(coalesce(as.character(.x), domain[1]) == domain)) %>%
      ungroup()
  }
  as.matrix(configs)
}

population_to_configs <- function(X, parameter_space) {
  tunnable_params <- get_not_fixed(parameter_space)
  discrete_params <- get_discrete_not_fixed(parameter_space)
  categorical_params <- get_categorical_not_fixed(parameter_space)
  fixed_params <- get_fixed(parameter_space)
  no_dummy <- ncol(X) - length(tunnable_params)
  
  if (no_dummy > 0) {
    colnames(X) <- c(tunnable_params, paste0('dummy_', 1:no_dummy))
  } else {
    colnames(X) <- c(tunnable_params)
  }
  
  result <-  as_tibble(X) %>%
    mutate_at(discrete_params, as.integer)
  walk(categorical_params, ~ {
    result[, .x] <<- parameter_space$domain[[.x]][result %>% pull(.x)]
  })
  
  result %>%
    bind_cols(parameter_space$domain[fixed_params]) %>%
    mutate(conf_id = row_number())
}