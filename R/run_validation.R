

run_validation <- function(experiment_data, name, configs, folder, aggregation_function, no_samples = 100) {
  validation_dir <- file.path(folder, name, "validation")
  dir.create(validation_dir, showWarnings = F, recursive = T)
  results <- configs
  if (!('conf_id' %in% colnames(results))) {
    results <- mutate(results, conf_id = row_number())
  }
  results <- results %>%
    mutate(
      perf = pmap(., function(...) {
        config <- df_to_character(list(...))
        sample_performance(
          algorithm = experiment_data$algorithm,
          config = config,
          problemSpace = ProblemSpace(problems = experiment_data$eval_problems$problem_space),
          solve_function = experiment_data$solve_function,
          no_samples = no_samples,
          cache = file.path(validation_dir, paste0(config['conf_id'], '.rds')),
          parallel = T,
          unnest_instances = F
        )
      })
    )
  list(
    results,
    perf = aggregation_function(results)
  )
}