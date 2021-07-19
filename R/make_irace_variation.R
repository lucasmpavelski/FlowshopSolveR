make_irace_variation <- function(algorithm,
                                 problem_space,
                                 solve_function,
                                 irace_scenario,
                                 no_samples = 10,
                                 cache_folder = NA) {
  if (!is.na(cache_folder)) {
    dir.create(cache_folder,
               recursive = TRUE,
               showWarnings = FALSE)
  }
  
  write_input_cache <- function(configs, iter) {
    if (is.na(cache_folder))
      return(NA)
    input_cache_fn = file.path(cache_folder,
                               sprintf("variation_input_%d.csv", iter))
    write_csv(configs, input_cache_fn)
  }
  
  output_cache_fn <- function(iter) {
    file.path(cache_folder,
              sprintf("variation_output_%d.csv", iter))
  }
  
  output_objs_cache_fn <- function(iter) {
    file.path(cache_folder,
              sprintf("variation_input_objs_%d.csv", iter))
  }
  
  sample_instances_by_objectives <- function(objective_weights) {
    problems_per_obj <- as.integer(objective_weights * no_samples)
    sampled_problems <- NULL
    all_problems <- problem_space@problems
    objectives <- unique(map_int(all_problems, ~ .x@data$meta_objective))
    for (obj in objectives) {
      obj_problems <- keep(all_problems, ~ .x@data$meta_objective == obj)
      sampled_problems <- c(sampled_problems,
                            sample(obj_problems, problems_per_obj[obj]))
    }
    sampled_problems
  }
  
  read_output_cache <- function(iter) {
    if (is.na(cache_folder) ||
        !file.exists(output_cache_fn(iter)))
      return(NA)
    read_csv(output_cache_fn(iter))
  }
  
  write_output_cache <- function(results, iter) {
    if (is.na(cache_folder))
      return(NA)
    write_csv(results, output_cache_fn(iter))
  }
  
  irace_variation <- function(configs, iter, W, B, ...) {
    future_map(unique(configs$conf_id), function(i, ...) {
      set.seed(iter * i * 31)
      instances <- sample_instances_by_objectives(W[i, ])
      init_configs <-
        configs %>% filter(conf_id %in% B[i, ]) %>% select(-conf_id) %>% distinct()
      if (nrow(init_configs) == 3) {
        init_configs <- init_configs[1:2, ]
      }
      var_irace_scenario <- irace_scenario
      var_irace_scenario$initConfigurations <- init_configs
      var_irace_scenario$seed <- iter * i * 91
      result <- train_best_solver(
        problem_space = ProblemSpace(problems = instances),
        algorithm = algorithm,
        solve_function = solve_function,
        irace_scenario = var_irace_scenario,
        cache = NA,
        quiet = F,
        parallel = 1
      )
      result[1,]
    }, .options = furrr_options(seed = TRUE)) %>%
      bind_rows()
  }
  
  variation_irace <- function(X, iter, problem, Y, ...) {
    write_csv(as_tibble(Y), output_objs_cache_fn(iter))
    configs <- denormalize_population(X, problem) %>%
      population_to_configs(algorithm@parameters)
    write_input_cache(configs, iter)
    results <- read_output_cache(iter)
    if (is.na(results[1])) {
      results <- irace_variation(configs, iter, ...)
      write_output_cache(results, iter)
    }
    results %>%
      configs_to_population(algorithm@parameters) %>%
      normalize_population(problem)
  }
  
  variation_irace
}