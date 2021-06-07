make_performance_sample_evaluation <-
  function(algorithm,
           problem_space,
           solve_function,
           aggregate_performances,
           no_samples = 10,
           parallel = FALSE,
           cache_folder = NA) {
    eval_count <- 1
    
    if (!is.na(cache_folder)) {
      dir.create(cache_folder, recursive = TRUE, showWarnings = FALSE)
    }
    
    write_cache_input <- function(configs) {
      if (is.na(cache_folder))
        return()
      input_cache_fn <- file.path(cache_folder,
                                  sprintf("eval_input_%d.csv", eval_count))
      write_csv(configs, file = input_cache_fn)
    }
    
    output_cache_fn <- function() {
      file.path(cache_folder,
                sprintf("eval_output_%d.csv", eval_count))
    }
    
    write_cache_output <- function(sampled_perfs) {
      if (is.na(cache_folder))
        return()
      write_csv(sampled_perfs, file = output_cache_fn())
    }
    
    read_cache <- function() {
      if (is.na(cache_folder) ||
          !file.exists(output_cache_fn()))
        return(NA)
      read_csv(output_cache_fn())
    }
    
    compute_sample_performance <- function(configs) {
      perfs <- configs %>%
        mutate(config = pmap(., c)) %>%
        mutate(
          perf = future_map(
            config,
            sample_performance,
            algorithm = algorithm,
            problemSpace = problem_space,
            solve_function = solve_function,
            no_samples = no_samples,
            parallel = parallel,
            .options = furrr_options(seed = 654987981)
          )
        )
      
      perfs <- aggregate_performances(perfs)
      
      perfs %>%
        arrange(meta_objective, conf_id) %>%
        pivot_wider(names_from = "meta_objective", values_from = "performance")
    }
    
    test_problems_eval <- function(x) {
        configs <- population_to_configs(x, algorithm@parameters)
        write_cache_input(configs)
        sampled_perfs <- read_cache()
        if (is.na(sampled_perfs)) {
          sampled_perfs <- compute_sample_performance(configs)
          write_cache_output(sampled_perfs)
        }
        eval_count <<- eval_count + 1
        
        sampled_perfs <- sampled_perfs %>%
          select(-conf_id) %>%
          as.matrix()
        
        colnames(sampled_perfs) <- NULL
        sampled_perfs
      }
    
    test_problems_eval
  }