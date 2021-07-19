make_performance_sample_evaluation <- function(algorithm,
                                               problem_space,
                                               solve_function,
                                               aggregate_performances,
                                               no_samples = 10,
                                               parallel = FALSE,
                                               cache_folder = NA,
                                               seed = 357357873) {
  eval_count <- 1
  arquive <- NULL
  
  if (!is.na(cache_folder)) {
    dir.create(cache_folder,
               recursive = TRUE,
               showWarnings = FALSE)
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
    dt <- read_csv(output_cache_fn())
    dt %>%
      mutate_all(as.double)
  }
  
  compute_sample_performance <- function(configs) {
    configs <- configs %>%
      mutate(key = arquive_eval_config_names(.))
    
    computed_from_arquive <- configs %>%
      filter(key %in% names(arquive)) %>%
      mutate(performance = map(key, ~arquive[[.x]])) %>%
      select(conf_id, performance) %>% 
      mutate(meta_objective = map(performance, seq_along)) %>%
      unnest(cols = c(performance, meta_objective))
    
    no_objs <- n_distinct(computed_from_arquive$meta_objective)
    no_solutions_to_eval <- nrow(configs) - nrow(computed_from_arquive) / no_objs
    cat('\n', no_solutions_to_eval, 'to be computed\n')
    
    if (no_solutions_to_eval == 0) {
      perfs <- computed_from_arquive %>%
        arrange(meta_objective, conf_id) %>%
        pivot_wider(
          names_from = "meta_objective",
          values_from = "performance"
        )
      return(perfs)
    }
    
    perfs <- configs %>%
      filter(!(key %in% names(arquive))) %>%
      mutate(config = pmap(., c)) %>%
      mutate(
        perf = map(
          config,
          ~{
            set.seed(42)
            sample_performance(
              config = .x,
              algorithm = algorithm,
              problemSpace = problem_space,
              solve_function = solve_function,
              no_samples = no_samples,
              parallel = parallel
            )
          }
        )
      )
    
    perfs <- aggregate_performances(perfs) %>%
      bind_rows(computed_from_arquive) %>%
      arrange(meta_objective, conf_id) %>%
      pivot_wider(
        names_from = "meta_objective",
        values_from = "performance"
      )
    
    perfs
  }
  
  arquive_eval_config_names <- function(configs) {
    configs %>%
      select(-conf_id) %>%
      mutate_if(is.numeric, ~sprintf("%.2f", .x)) %>%
      pmap(paste)
  }
  
  archive_evals <- function(configs, sampled_perfs) {
    keys <- arquive_eval_config_names(configs)
    values <- sampled_perfs %>%
      select(-conf_id) %>%
      pmap(c)
    for (i in seq_along(keys)) {
      arquive[[keys[[i]]]] <<- values[[i]]
    }
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
    
    archive_evals(configs, sampled_perfs)
    sampled_perfs <- sampled_perfs %>%
      select(-conf_id) %>%
      as.matrix()
    
    colnames(sampled_perfs) <- NULL
    sampled_perfs
  }
  
  test_problems_eval
}