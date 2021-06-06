library(tidymodels)
library(tidyselect)
library(tidyverse)
library(FlowshopSolveR)
library(here)
library(future)
library(metaOpt)

TASK_NAME <- "NEH_recommendation"
MODEL_FOLDER <- here("data", "models")
TASK_NAME <- "NEH_recommendation"
TASK_FOLDER <- file.path(MODEL_FOLDER, TASK_NAME)

read_exp_dt <- function(param, model_name, exp_name, dt_name) {
  path <- file.path(TASK_FOLDER, param, paste0(model_name, ',', exp_name), dt_name)
  if (file.exists(path)) {
    readRDS(path)
  } else {
    NULL
  }
}

get_result <- function(problem, config) {
  initFactories(here('data'))
  print(df_to_character(config))
  result <- solveFSP(
    mh = algorithm@name,
    rproblem = df_to_character(problem),
    rparams = df_to_character(config),
    seed = 0
  )
  result$fitness
}

get_random_param <- function(domain, no_probs) {
  sample(domain, no_probs, replace = T)
}

get_fixed <- function(param, config, no_probs) {
  config %>%
    pull(param) %>%
    rep(no_probs)
}

get_instance_best <- function(param, instance_performances) {
  instance_performances %>% pull(param)
}

write_result <- function(algorithm, test_problems, model_name, exp_name, 
                         global_best = tibble(),
                         instance_performances = tibble()) {
  no_probs <- nrow(test_problems)
  params_data <- tibble(
    param = algorithm@parameters$names,
    fixed = algorithm@parameters$isFixed,
    domain = algorithm@parameters$domain
  )
  
  if (exp_name == 'default') {
    params_data <- params_data %>%
      mutate(prediction = map(param, 
                              get_fixed, 
                              config = default_configs(algorithm@name), 
                              no_probs = no_probs))
  } else if (exp_name == 'global_best') {
    params_data <- params_data %>%
      mutate(prediction = map(param, 
                              get_fixed, 
                              config = global_best, 
                              no_probs = no_probs))
  } else if (exp_name == 'random') {
    params_data <- params_data %>%
      mutate(prediction = map(domain, 
                              get_random_param, 
                              no_probs = no_probs))
  } else if (exp_name == 'instance_best') {
    params_data <- params_data %>%
      mutate(prediction = map(param,
                              get_instance_best,
                              instance_performances = instance_performances))
  } else {
    params_data <- params_data %>%
      mutate(prediction = if_else(
        fixed,
        map(domain, ~rep(.x, no_probs)),
        map(
          param, 
          read_exp_dt,
          model_name = model_name, 
          exp_name = exp_name,
          dt_name = 'test_predict'
        )))
  }
  
  prob_data <- test_problems %>%
    group_nest(row_number(), .key = 'problem')
  
  predict_dt <- params_data %>%
    unnest(cols = prediction) %>%
    select(param, prediction) %>%
    pivot_wider(
      names_from = param,
      values_fn = list,
      values_from = prediction
    ) %>%
    unnest(cols = everything()) %>%
    group_nest(row_number(), .key = 'config') %>%
    inner_join(prob_data)
  
  results_folder <- file.path(TASK_FOLDER, 'results')
  dir.create(results_folder, showWarnings = F, recursive = T)
  
  predict_dt <- predict_dt %>%
    mutate(fitness = map2_dbl(problem, config, get_result))
  
  exp_file <- paste0(model_name, ',', exp_name, '.rda')
  file_path <- file.path(results_folder, exp_file)
  saveRDS(predict_dt, file_path)
}


algorithm <- get_algorithm("NEH")
params <- algorithm@parameters$names
exp_name <- "default"
model_name <- 'none'

test_problems <- test_problems_df() %>%
  filter(no_jobs <= 500, budget == 'low') %>%
  mutate(metaopt_problem = pmap(., as_metaopt_problem)) %>%
  unnest(instances)


library(future)
plan(multisession)

# global_best <- readRDS(here('runs/neh_best_solver/all/NEH/result.rds'))[1,]

# inst_perfs <- read_performance_data(
#   problem_space = ProblemSpace(problems = test_problems$metaopt_problem),
#   algorithm_space = AlgorithmSpace(algorithms = list(algorithm)),
#   cache_folder = here('runs/neh')
# )

# write_result(algorithm, test_problems, 'none', 'instance_best', instance_performances = inst_perfs)
# write_result(algorithm, test_problems, 'rand_forest', 'ablation', instance_performances = inst_perfs)

# 
value(c(
  # future({write_result(algorithm, test_problems, 'none', 'global_best', global_best)}),
  # future({write_result(algorithm, test_problems, 'none', 'random', global_best)})
#   future({write_result(algorithm, test_problems, 'none', 'default')}),
#   future({write_result(algorithm, test_problems, 'decision_tree', 'instance-based')}),
#   future({write_result(algorithm, test_problems, 'decision_tree', 'instance-based-dependencies')}),
#   future({write_result(algorithm, test_problems, 'rand_forest', 'instance-based')}),
    # future({write_result(algorithm, test_problems, 'rand_forest', 'instance-based-dependencies')})
  future({write_result(algorithm, test_problems, 'decision_tree', 'ablation')}),
  future({write_result(algorithm, test_problems, 'rand_forest', 'ablation')})
))

