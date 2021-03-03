library(tidyverse)
library(tidymodels)
library(here)
library(FlowshopSolveR)

get_preprocessing <- function(train_dt) {
  param_rec <- recipe(perf ~ ., data = train_dt) %>%
    step_nzv(all_predictors()) %>%
    step_corr(all_numeric()) # %>%
    # step_BoxCox(all_numeric())
  
  prep(param_rec, training = train_dt)
}

get_model_by_name <- function(name) {
  if (name == "rand_forest") {
    rand_forest(mtry = tune(),
                trees = tune(),
                min_n = tune()) %>%
      set_engine("ranger", importance = "impurity") %>%
      set_mode("regression")
  } else if (name == "linear_reg") {
    linear_reg(
      penalty = tune(),
      mixture = tune()
    )  %>%
      set_engine("lm") %>%
      set_mode("regression")
  }
}

get_tune_grid_by_name <- function(name) {
  if (name == "rand_forest") {
    grid_regular(mtry(c(1, 10)),
                 trees(),
                 min_n(),
                 levels = 3)
  } else if (name == "linear_reg") {
    grid_regular(penalty(),
                 mixture(),
                 levels = 5)
  }
}

train_workflow <- function(splitted_data, model_name = "rand_forest") {
  train_dt <- training(splitted_data)
  train_dt_preprocessing <- get_preprocessing(train_dt)
  
  model <- get_model_by_name(model_name)
  
  dt_wflow <-
    workflow() %>%
    add_model(model) %>%
    add_recipe(train_dt_preprocessing)
  
  tune_grid <- get_tune_grid_by_name(model_name)
  
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
  
  # test_prep <- bake(train_dt_preprocessing, new_data = testing(splitted_data))
  # predict(final_tree, testing(splitted_data))
  
  last_fit(final_tree, train_dt_preprocessing, split = splitted_data)
}

evaluate_rf_model <- function(splitted_data, save_path) {
  fitted_workflow <- train_workflow(splitted_data)
  
  metrics <- fitted_workflow %>%
    collect_metrics() %>%
    print()
  
  dir.create(dirname(save_path), showWarnings = F, recursive = T)
  write_rds(fitted_workflow, save_path)
}

load_data_for_config <- function(lon_config, problem_set, perf_task, perfs) {
  metrics <-
    read_rds(sprintf("%s/%s_metrics.rds", lons_folder, lon_config$id)) %>%
    mutate(stopping_criterion = "TIME")
  
  all_data <- perfs %>%
    filter((problem == problem_set) | (problem_set == "all")) %>%
    inner_join(metrics) %>%
    select(
      -dist,-corr,-problem,-corv,-objective,-type,-instance,-model,
      -instance_features,-id,-stopping_criterion,-budget,-inst_n
    )
  all_data["perf"] <- log(all_data[perf_task] + 1e-6)
  
   all_data %>%
    mutate(
      compress_rate = no_nodes / clon_no_nodes,
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
      neutral_mean_weight = if_else(
        is.nan(neutral_mean_weight),
        0,
        neutral_mean_weight
      ),
      neutral_mean_out_degree = if_else(
        neutral_mean_out_degree == 0,
        1e-6,
        neutral_mean_out_degree
      ),
      fitness_fitness_correlation = if_else(
        is.nan(fitness_fitness_correlation) |
          is.na(fitness_fitness_correlation),
        0,
        fitness_fitness_correlation
      )
    ) %>%
    mutate(perf = (perf - min(perf)) / (max(perf) - min(perf))) %>%
    select(-ig_lsps_rpd,-ig_rs_rpd)
}

lon_configs <- read_rds(here("reports/lons_study/lon_configs.rds"))
lons_folder <- here("data", "lons_cache")

perfs <-
  read_rds(here("reports/lons_study/relative_perfs_best.rds")) %>%
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
    .groups = "drop"
  )

problem_set <- "vrf-large"
perf_task <- "ig_lsps_rpd"
model_name <- "rand_forest"

# for (CONFIG_ID in c(1,2,9,10)) {
#   all_data <- load_data_for_config(lon_configs[[CONFIG_ID]], problem_set, perf_task, perfs)
#   
#   set.seed(6547)
#   external_cv_splits <- vfold_cv(all_data, v = 5, strata = perf) %>%
#     mutate(
#       save_path = here(
#         "reports",
#         "lons_study",
#         "models-5-fold",
#         model_name,
#         lon_configs[[CONFIG_ID]]$id,
#         problem_set,
#         perf_task,
#         id
#       )
#     )
#   
#   walk2(external_cv_splits$splits, external_cv_splits$save_path, evaluate_rf_model)
# }
# 
# 
# metrics <-
#   read_rds(sprintf("%s/%s_metrics.rds", lons_folder, lon_configs[[1]]$id)) %>%
#   mutate(stopping_criterion = "TIME")
# 
# 
# for (CONFIG_ID in c(2,9,10)) {
#   metrics <- 
# }
# 
# d1 <- load_data_for_config(lon_configs[[1]], problem_set, perf_task, perfs)
# colnames(d1) <- paste0(colnames(d1), "_1")
# 
# d2 <- load_data_for_config(lon_configs[[2]], problem_set, perf_task, perfs)
# colnames(d2) <- paste0(colnames(d2), "_2")
# 
# d9 <- load_data_for_config(lon_configs[[9]], problem_set, perf_task, perfs)
# colnames(d9) <- paste0(colnames(d9), "_9")
# 
# d10 <- load_data_for_config(lon_configs[[10]], problem_set, perf_task, perfs)
# colnames(d10) <- paste0(colnames(d10), "_10")
# 
# all_data <- 
#   bind_cols(d1, d2, d9, d10) %>%
#   rename(perf = perf_1) %>%
#   select(-perf_2, -perf_9, -perf_10)
# 
# set.seed(6547)
# external_cv_splits <- vfold_cv(all_data, v = 5, strata = perf) %>%
#   mutate(
#     save_path = here(
#       "reports",
#       "lons_study",
#       "models-5-fold",
#       model_name,
#       "all",
#       problem_set,
#       perf_task,
#       id
#     )
#   )
#  
# walk2(external_cv_splits$splits, external_cv_splits$save_path, evaluate_rf_model)
