library(here)
library(future)
library(tidymodels)
library(tidyverse)
library(FlowshopSolveR)


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

build_split <- function(features, outputs, problems_dt, param, summ_features = T,
                        filter_na = T) {
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
    dplyr::filter(!filter_na | !across(all_of(param), is.na)) %>%
    dplyr::filter(name %in% problem_set) %>%
    dplyr::inner_join(features, by = "name") %>%
    dplyr::select(all_of(c(names(features), param)), -name) %>%
    dplyr::mutate(across(all_of(param), factor))
}

collect_perf <- function(dt_pred) {
  possible_values <- unique(c(
    as.character(dt_pred$pred_estimate),
    as.character(dt_pred$pred_truth)
  ))
  dt_pred <- dt_pred %>%
    dplyr::mutate(
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
          tree_depth = tune()
        ) %>%
          set_engine("rpart") %>%
          set_mode("classification"),
      tune_grid = grid_regular(
        tree_depth(range = c(1L, 3L)),
        levels = 5
      )
    )
  } else if (model_name == "rand_forest") {
    list(
      model = rand_forest(
          mtry = tune(),
          trees = 10,
          min_n = 1
        ) %>%
          set_engine("ranger") %>%
          set_mode("classification"),
      tune_grid = grid_regular(
        mtry(c(6, 6)),
        levels = 1
      )
    )
  }
}


decision_tree_param_model_experiment <- function(
  train_dt, 
  test_dt, 
  param, 
  model_name, 
  exp_name,
  dependencies,
  cache = T) {
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
  
  depends_on <- dependencies %>%
    filter(from == param) %>%
    pull(to)
  
  predict_dt <- NULL
  if (length(depends_on) > 0) {
    extra_train <- depends_on %>% 
      map(~file.path(TASK_FOLDER, .x, paste0(model_name, ",", exp_name), 'train_dt')) %>%
      map(readRDS) %>% reduce(left_join)
    train_dt <- train_dt %>%
      left_join(extra_train)
    for (dep_param in depends_on) {
      pred_path <- file.path(TASK_FOLDER, dep_param, 
                             paste0(model_name, ",", exp_name), 
                             'test_predict')
      pred_dt <- readRDS(pred_path)
      test_dt[dep_param] <- pred_dt
    }
  }

  inputs <- names(train_dt %>% select(-all_of(param)))
  formula <- as.formula(paste(param, "~", paste(inputs, collapse = " + ")))
  set.seed(123)
  
  param_rec <- recipe(formula, data = train_dt) %>% 
    step_nzv(all_predictors()) %>%
    themis::step_downsample(all_of(param)) %>%
    step_corr(all_numeric())
  
  train_dt_preprocessing <- prep(param_rec, training = train_dt)
  
  # preprocessed_train_dt <- juice(train_dt_preprocessing)
  # preprocessed_test_dt <- bake(train_dt_preprocessing, test_dt)

  model_params <- get_model_by_name(model_name)
  
  dt_wflow <-
    workflow() %>%
    add_model(model_params$model) %>%
    add_recipe(train_dt_preprocessing)

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
    pred_estimate = predict(final_tree, train_dt)$.pred_class,
    pred_truth = train_dt %>% pull(param)
  )

  exp_result <- list(
    train_dt = train_dt,
    test_dt = test_dt,
    tune_metrics = collect_metrics(dt_fit),
    test_perf = collect_perf(test_dt_pred),
    train_perf = collect_perf(train_dt_pred),
    train_predict = as.character(train_dt_pred$pred_estimate),
    test_predict = as.character(test_dt_pred$pred_estimate),
    depends_on = depends_on,
    model = final_tree
  )

  # if (cache) {
    walk(exp_names, function(.x) {
      saveRDS(exp_result[[.x]], file = file.path(exp_path, .x))
    })
  # }
  
  rm(list = ls())
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

all_cores <- parallel::detectCores(logical = FALSE)
library(doFuture)
registerDoFuture()
cl <- parallel::makeCluster(all_cores - 6)
plan(cluster, workers = cl)

algorithm <- get_algorithm("NEH")
params <- algorithm@parameters$names
exp_name <- "instance-based-c2"
model_name <- 'decision_tree'
dependencies <- F
cache <- F

features <- compute_features_data(problems_dt, use_cache = T)
outputs <- compute_outputs_data(problems_dt, algorithm, 
                                folder = here("runs", "neh"), use_cache = T)


# build_split(features, outputs, train_problems_df(), param, summ_features = F)

if (!dependencies) {
  dependencies <- tribble(~from, ~to)
  train_order <- params[2:length(params)]

  walk(train_order, function(param) {
    decision_tree_param_model_experiment(
      build_split(features, outputs, train_problems_df(), param, summ_features = F),
      build_split(features, outputs, test_problems_df(), param, summ_features = F, filter_na = F),
      param,
      model_name,
      exp_name,
      dependencies = tribble(~from, ~to),
      cache = cache
    )
  })
} else {
  dependencies <- tribble(
    ~from, ~to,
    "NEH.Init.NEH.First.PriorityWeighted", "NEH.Init.NEH.Ratio",

    "NEH.Init.NEH.First.PriorityOrder"   , "NEH.Init.NEH.Ratio",
    "NEH.Init.NEH.First.PriorityOrder"   , "NEH.Init.NEH.First.PriorityWeighted",

    "NEH.Init.NEH.First.Priority"        , "NEH.Init.NEH.Ratio",
    "NEH.Init.NEH.First.Priority"        , "NEH.Init.NEH.First.PriorityWeighted",
    "NEH.Init.NEH.First.Priority"        , "NEH.Init.NEH.First.PriorityOrder",

    "NEH.Init.NEH.PriorityWeighted"      , "NEH.Init.NEH.Ratio",
    # "NEH.Init.NEH.PriorityWeighted"      , "NEH.Init.NEH.First.PriorityWeighted",
    # "NEH.Init.NEH.PriorityWeighted"      , "NEH.Init.NEH.First.PriorityOrder",
    # "NEH.Init.NEH.PriorityWeighted"      , "NEH.Init.NEH.First.Priority",

    "NEH.Init.NEH.PriorityOrder"         , "NEH.Init.NEH.Ratio",
    # "NEH.Init.NEH.PriorityOrder"         , "NEH.Init.NEH.First.PriorityWeighted",
    # "NEH.Init.NEH.PriorityOrder"         , "NEH.Init.NEH.First.PriorityOrder",
    # "NEH.Init.NEH.PriorityOrder"         , "NEH.Init.NEH.First.Priority",
    "NEH.Init.NEH.PriorityOrder"         , "NEH.Init.NEH.PriorityWeighted",

    "NEH.Init.NEH.Priority"              , "NEH.Init.NEH.Ratio",
    # "NEH.Init.NEH.Priority"              , "NEH.Init.NEH.First.PriorityWeighted",
    # "NEH.Init.NEH.Priority"              , "NEH.Init.NEH.First.PriorityOrder",
    # "NEH.Init.NEH.Priority"              , "NEH.Init.NEH.First.Priority",
    "NEH.Init.NEH.Priority"              , "NEH.Init.NEH.PriorityWeighted",
    "NEH.Init.NEH.Priority"              , "NEH.Init.NEH.PriorityOrder",

    "NEH.Init.NEH.Insertion"             , "NEH.Init.NEH.Ratio",
    # "NEH.Init.NEH.Insertion"             , "NEH.Init.NEH.First.PriorityWeighted",
    # "NEH.Init.NEH.Insertion"             , "NEH.Init.NEH.First.PriorityOrder",
    # "NEH.Init.NEH.Insertion"             , "NEH.Init.NEH.First.Priority",
    "NEH.Init.NEH.Insertion"             , "NEH.Init.NEH.PriorityWeighted",
    "NEH.Init.NEH.Insertion"             , "NEH.Init.NEH.PriorityOrder",
    "NEH.Init.NEH.Insertion"             , "NEH.Init.NEH.Priority",
  )

  train_order <- dependencies %>%
    as.matrix() %>%
    igraph::graph_from_edgelist() %>%
    igraph::topo_sort("in") %>%
    names()

  walk(train_order, function(param) {
    train_dt <- build_split(features, outputs, train_problems_df(), param, summ_features = F)
    test_dt <- build_split(features, outputs, test_problems_df(), param, summ_features = F, filter_na = F)
    decision_tree_param_model_experiment(
      train_dt,
      test_dt,
      param,
      model_name,
      paste0(exp_name, '-', 'dependencies'),
      dependencies = dependencies,
      cache = cache
    )
  })

}



