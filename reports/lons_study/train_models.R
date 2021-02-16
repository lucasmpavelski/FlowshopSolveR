library(tidyverse)
library(tidymodels)
library(here)
library(FlowshopSolveR)

train_workflow <- function(splitted_data) {
  train_dt <- training(splitted_data)
  
  param_rec <- recipe(perf ~ ., data = train_dt) %>%
    step_nzv(all_predictors()) %>%
    step_corr(all_numeric())
  
  train_dt_preprocessing <- prep(param_rec, training = train_dt)
  
  
  model <- rand_forest(mtry = tune(),
                       trees = tune(),
                       min_n = tune()) %>%
    set_engine("ranger", importance = "impurity") %>%
    set_mode("regression")
  
  dt_wflow <-
    workflow() %>%
    add_model(model) %>%
    add_recipe(train_dt_preprocessing)
  
  tune_grid <- grid_regular(mtry(c(1, 10)),
                            trees(),
                            min_n(),
                            levels = 3)
  
  dt_fit <-
    dt_wflow %>%
    tune_grid(resamples = vfold_cv(train_dt, strata = "perf"),
              grid = tune_grid)
  
  best_dt <- dt_fit %>%
    select_best("rsq")
  
  final_wf <-
    dt_wflow %>%
    finalize_workflow(best_dt)
  
  final_tree <-
    final_wf %>%
    fit(data = train_dt)
  
  last_rf_fit <- last_fit(final_tree, splitted_data)
}

evaluate_rf_model <- function(splitted_data, save_path) {
  fitted_workflow <- train_workflow(splitted_data)
  
  # metrics <- last_rf_fit %>%
  #   collect_metrics() %>%
  #   print()
  # 
  # last_rf_fit %>%
  #   pluck(".workflow", 1) %>%
  #   pull_workflow_fit() %>%
  #   vip(num_features = 5)
  
  dir.create(dirname(save_path), showWarnings = F, recursive = T)
  write_rds(last_rf_fit, save_path)
}

lon_configs <- read_rds(here('reports/lons_study/lon_configs.rds'))
CONFIG_ID <- 1
lons_folder <- here("data", "lons_cache")

perfs <-
  read_rds(here('reports/lons_study/relative_perfs_best.rds')) %>%
  select(-metaopt_problem,-budget,-stopping_criterion) %>%
  group_by(
    dist,
    corr,
    no_jobs,
    no_machines,
    problem,
    corv,
    objective,
    type,
    instance,
    model,
    instance_features,
    id
  ) %>%
  summarise(
    ig_rs_rpd = mean(ig_rs_rpd),
    ig_lsps_rpd = mean(ig_lsps_rpd),
    .groups = 'drop'
  )

metrics <-
  read_rds(sprintf("%s/%s_metrics.rds", lons_folder, lon_configs[[CONFIG_ID]]$id)) %>%
  mutate(stopping_criterion = 'TIME')

problem_set <- 'all'
all_data <- perfs %>%
  filter((problem == problem_set) | (problem_set == 'all')) %>%
  inner_join(metrics) %>%
  select(
    -dist,-corr,-problem,-corv,-objective,-type,-instance,-model,
    -instance_features,-id,-stopping_criterion,-budget,-inst_n
  ) %>%
  mutate(
    perf = log(ig_rs_rpd + 1e-6),
    compress_rate = clon_no_nodes / no_nodes,
    average_weight_of_self_loops = if_else(
      is.nan(average_weight_of_self_loops),
      0,
      average_weight_of_self_loops
    ),
    graph_assortativity_degree = if_else(
      is.nan(graph_assortativity_degree),
      0,
      graph_assortativity_degree
    ),
    fitness_fitness_correlation = if_else(
      is.nan(fitness_fitness_correlation) |
        is.na(fitness_fitness_correlation),
      0,
      fitness_fitness_correlation
    )
  ) %>%
  select(-ig_lsps_rpd,-ig_rs_rpd)

set.seed(6547)
external_cv_splits <- vfold_cv(all_data, v = 5, strata = perf) %>%
  mutate(
    save_path = here(
      "reports",
      "lons_study",
      "models",
      lon_configs[[CONFIG_ID]]$id,
      "ig_rs_rpd",
      id
    )
  )

walk2(external_cv_splits$splits, save_path, evaluate_rf_model)
