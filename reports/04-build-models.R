library(tidymodels)
library(tidyselect)
library(tidyverse)
library(FlowshopSolveR)
library(here)
library(future)

MODEL_FOLDER <- here("data", "models")
TASK_NAME <- "NEH_recommendation"
TASK_FOLDER <- file.path(MODEL_FOLDER, TASK_NAME)

dir.create(MODEL_FOLDER, showWarnings = F, recursive = T)
dir.create(TASK_FOLDER, showWarnings = F, recursive = T)

compute_features_data <- function(problems_dt, use_cache = T) {
  path <- file.path(TASK_FOLDER, "features.csv")
  if (use_cache & file.exists(path)) {
    return(read_csv(path))
  }
  features_dt <- problems_dt %>%
    unnest(instances) %>%
    mutate(features = map2(metaopt_problem, instance, load_problem_features,
      features_folder = here("data", "features")
    )) %>%
    select(type, objective, features) %>%
    unnest(features)
  if (use_cache) {
    write_csv(features_dt, path)
  }
  features_dt
}

compute_outputs_data <- function(problems_dt, algorithm, folder, use_cache = T) {
  path <- file.path(TASK_FOLDER, "outputs.csv")
  if (use_cache & file.exists(path)) {
    return(read_csv(path))
  }
  problem_space <- ProblemSpace(problems = problems_dt$metaopt_problem)
  algorithm_space <- AlgorithmSpace(algorithms = list(algorithm))
  dir.create(cache_folder, showWarnings = F)
  performance_dt <- read_performance_data(
    problem_space,
    algorithm_space,
    cache_folder = folder,
    only_best = T
  )
  outputs_dt <- performance_dt %>%
    mutate(name = map_chr(problem, ~ .x@name)) %>%
    select(name, all_of(algorithm@parameters$names))
  if (use_cache) {
    write_csv(outputs_dt, path)
  }
  outputs_dt
}

build_split <- function(features, outputs, problems_dt, param, summ_features = T) {
  features <- features %>%
    select(-instance)
  if (summ_features) {
    features <- features %>%
      group_by(across(where(is.character))) %>%
      summarise(across(everything(), mean)) %>%
      ungroup()
  }
  problem_set <- problems_dt %>%
    pmap(as_metaopt_problem) %>%
    map_chr(~ .x@name)
  outputs %>%
    filter(!across(all_of(param), is.na)) %>%
    filter(name %in% problem_set) %>%
    inner_join(features, by = "name") %>%
    select(all_of(c(names(features), param)), -name) %>%
    mutate(across(all_of(param), factor))
}

collect_perf <- function(dt_pred) {
  possible_values <- unique(c(
    as.character(dt_pred$pred_estimate),
    as.character(dt_pred$pred_truth)
  ))
  dt_pred <- dt_pred %>%
    mutate(
      pred_estimate = factor(pred_estimate, levels = possible_values),
      pred_truth = factor(pred_truth, levels = possible_values)
    )
  bind_rows(
    accuracy(dt_pred, pred_truth, pred_estimate),
    bal_accuracy(dt_pred, pred_truth, pred_estimate),
    detection_prevalence(dt_pred, pred_truth, pred_estimate),
    f_meas(dt_pred, pred_truth, pred_estimate),
    j_index(dt_pred, pred_truth, pred_estimate),
    kap(dt_pred, pred_truth, pred_estimate),
    mcc(dt_pred, pred_truth, pred_estimate),
    npv(dt_pred, pred_truth, pred_estimate),
    ppv(dt_pred, pred_truth, pred_estimate),
    precision(dt_pred, pred_truth, pred_estimate),
    recall(dt_pred, pred_truth, pred_estimate),
    sensitivity(dt_pred, pred_truth, pred_estimate),
    specificity(dt_pred, pred_truth, pred_estimate)
  )
}


get_model_by_name <- function(model_name) {
  if (model_name == "decision_tree") {
    list(
      model = decision_tree(
          cost_complexity = tune(),
          tree_depth = tune()
        ) %>%
          set_engine("rpart") %>%
          set_mode("classification"),
      tune_grid = grid_regular(
        cost_complexity(),
        tree_depth(),
        levels = 5
      )
    )
  } else if (model_name == "rand_forest") {
    list(
      model = rand_forest(
          mtry = tune(),
          trees = tune(),
          min_n = tune()
        ) %>%
          set_engine("ranger") %>%
          set_mode("classification"),
      tune_grid = grid_regular(
        mtry(c(1, 10)),
        trees(),
        min_n(),
        levels = 3
      )
    )
  }
}


decision_tree_param_model_experiment <- function(train_dt, test_dt, param, model_name, exp_name, cache = T) {
  exp_path <- file.path(TASK_FOLDER, param, paste0(model_name, ",", exp_name))
  dir.create(exp_path, recursive = T, showWarnings = F)
  exp_names <- c(
    "train_dt", "test_dt", "tune_metrics", "test_perf", "train_perf",
    "train_predict", "test_predict", "model"
  )
  if (cache && all(map_lgl(exp_names, ~ file.exists(file.path(exp_path, .x))))) {
    exp_result <- list()
    walk(exp_names, function(name) {
      exp_result[[name]] <- readRDS(file.path(exp_path, name))
    })
    return(exp_result)
  }

  inputs <- names(train_dt %>% select(-all_of(param)))
  formula <- as.formula(paste(param, "~", paste(inputs, collapse = " + ")))
  set.seed(123)
  
  param_rec <- recipe(formula, data = train_dt) %>%
    step_zv(all_predictors()) %>% 
    step_lincomb(all_numeric())

  model_params <- get_model_by_name(model_name)
  
  dt_wflow <-
    workflow() %>%
    add_model(model_params$model) %>%
    add_recipe(param_rec)

  dt_fit <-
    dt_wflow %>%
    tune_grid(
      resamples = vfold_cv(train_dt),
      grid = model_params$tune_grid
    )

  best_dt <- dt_fit %>%
    select_best("roc_auc")

  final_wf <-
    dt_wflow %>%
    finalize_workflow(best_dt)

  final_tree <-
    final_wf %>%
    fit(data = train_dt)

  test_dt_pred <- tibble(
    pred_estimate = predict(final_tree, test_dt)$.pred_class,
    pred_truth = test_dt %>% pull(param)
  )

  train_dt_pred <- tibble(
    pred_estimate = predict(final_tree, test_dt)$.pred_class,
    pred_truth = test_dt %>% pull(param)
  )

  exp_result <- list(
    train_dt = train_dt,
    test_dt = test_dt,
    tune_metrics = collect_metrics(dt_fit),
    test_perf = collect_perf(test_dt_pred),
    train_perf = collect_perf(train_dt_pred),
    train_predict = as.character(train_dt_pred$pred_estimate),
    test_predict = as.character(test_dt_pred$pred_estimate),
    model = final_tree
  )

  if (cache) {
    walk(exp_names, ~ saveRDS(exp_result[[.x]], file = file.path(exp_path, .x)))
  }
}


problems_dt <- all_problems_df() %>%
  filter(budget == "low", no_jobs <= 500) %>%
  mutate(
    metaopt_problem = pmap(., as_metaopt_problem),
    performance_exists = map_lgl(
      metaopt_problem,
      ~ file.exists(here("runs", "neh", .x@name, "NEH", "result.rds"))
    )
  ) %>%
  filter(performance_exists)

features <- compute_features_data(problems_dt, use_cache = T)
outputs <- compute_outputs_data(problems_dt, algorithm, folder = here("runs", "neh"), use_cache = T)

algorithm <- get_algorithm("NEH")
params <- algorithm@parameters$names
exp_name <- "instance-based"
model_name <- 'rand_forest'


walk(params[2:length(params)], function(param) {
  decision_tree_param_model_experiment(
    build_split(features, outputs, train_problems_df(), param, summ_features = F),
    build_split(features, outputs, test_problems_df(), param, summ_features = F),
    param,
    model_name,
    exp_name
  )
})
