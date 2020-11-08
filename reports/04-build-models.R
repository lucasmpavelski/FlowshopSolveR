library(tidymodels)
library(FlowshopSolveR)

problems_dt <- all_problems_df() %>%
  filter(budget == 'low', no_jobs <= 500) %>%
  mutate(
    metaopt_problem = pmap(., as_metaopt_problem),
    name = map(metaopt_problem, ~as_tibble(.x@data)),
    performance_exists = map_lgl(metaopt_problem,
                                 ~file.exists(here('runs', 'neh', .x@name, 'NEH', 'result.rds')))
  ) %>%
  filter(performance_exists) %>%
  unnest(instances) %>%
  mutate(features = map2(metaopt_problem, instance, load_problem_features, 
                         features_folder = here('data', 'features')))

problem_space <- ProblemSpace(problems = problems_dt$metaopt_problem)
algorithm <- get_algorithm('NEH')
default_neh <- default_configs('NEH')
algorithm_space <- AlgorithmSpace(algorithms = list(algorithm))

cache_folder <- here('runs', 'neh')
dir.create(cache_folder, showWarnings = F)
irace_trained <- build_performance_data(
  problem_space = problem_space,
  algorithm_space = algorithm_space,
  solve_function = fsp_solver_performance,
  irace_scenario = defaultScenario(list(
    deterministic = 1,
    maxExperiments = 5000,
    initConfigurations = default_neh
  )),
  cache_folder = cache_folder,
  parallel = 7
)


params <- algorithm@parameters$names
params_types <- algorithm@parameters$types
params_domains <- algorithm@parameters$domain

output_dt <- irace_trained %>% 
  select(problems, results) %>%
  mutate(
    results = map(results, ~.x[1,]),
    name = map_chr(problems, ~.x@name)
  ) %>%
  unnest(results) %>%
  mutate(problems_dt = map(problems, ~as_tibble(.x@data))) %>%
  unnest(problems_dt) #%>%
  # select(name, all_of(params))


input_dt <- problems_dt %>%
  select(type, objective, features) %>%
  unnest(features)
  
param_idx <- 3
param <- params[param_idx]
print(param)
param_type <- params_types[[param]]
param_domain <- params_domains[[param]]

input_features <- names(input_dt %>% select(-name))
  
train_dt <- output_dt %>%
  select(name, all_of(param)) %>%
  inner_join(input_dt, by = 'name') %>%
  select(all_of(input_features), all_of(param))

train_dt <- train_dt[!is.na(train_dt[, param]),]

formula <- as.formula(paste(param, '~', paste(input_features, collapse = '+')))

set.seed(123)
param_split <- initial_split(train_dt, prop = 3/4)
param_train <- training(param_split)
param_test  <- testing(param_split)

param_rec <- 
  recipe(formula, data = param_train)

dt_model <- decision_tree(
    cost_complexity = tune(),
    tree_depth = tune()
  ) %>%
    set_engine("rpart") %>% 
    set_mode("classification")

tree_grid <- grid_regular(cost_complexity(),
                          tree_depth(),
                          levels = 5)

train_folds <- vfold_cv(param_train)

dt_wflow <- 
  workflow() %>% 
  add_model(dt_model) %>%
  add_recipe(param_rec)

dt_fit <- 
  dt_wflow %>% 
  tune_grid(
    resamples = train_folds,
    grid = tree_grid
  )

dt_fit %>%
  collect_metrics() %>%
  mutate(tree_depth = factor(tree_depth)) %>%
  ggplot(aes(cost_complexity, mean, color = tree_depth)) +
  geom_line(size = 1.5, alpha = 0.6) +
  geom_point(size = 2) +
  facet_wrap(~ .metric, scales = "free", nrow = 2) +
  scale_x_log10(labels = scales::label_number()) +
  scale_color_viridis_d(option = "plasma", begin = .9, end = 0)

best_dt <- dt_fit %>%
  select_best("roc_auc")

final_wf <- 
  dt_wflow %>% 
  finalize_workflow(best_dt)

final_tree <- 
  final_wf %>%
  fit(data = param_train)

dt_pred <- tibble(
  .pred_class = predict(final_tree, param_test)$.pred_class,
  .pred_truth = param_test %>% pull(param)
)

if (param_type %in% c('c', 'o')) {
  dt_pred <- dt_pred %>%
    mutate(
      .pred_class = factor(.pred_class, levels = param_domain),
      .pred_truth = factor(.pred_truth, levels = param_domain)
    )
}

dt_acc <- accuracy(dt_pred, truth = .pred_truth, estimate = .pred_class)
print(dt_acc)

dt_pred <- tibble(
  .pred_class = predict(final_tree, param_train)$.pred_class,
  .pred_truth = param_train %>% pull(param)
)

if (param_type %in% c('c', 'o')) {
  dt_pred <- dt_pred %>%
    mutate(
      .pred_class = factor(.pred_class, levels = param_domain),
      .pred_truth = factor(.pred_truth, levels = param_domain)
    )
}

dt_acc <- accuracy(dt_pred, truth = .pred_truth, estimate = .pred_class)
print(dt_acc)

# 
# rf_model <- rand_forest(
#   mode = 'classification'
# ) %>%
#   set_engine("ranger")
# 
# rf_wflow <- 
#   workflow() %>% 
#   add_model(rf_model) %>% 
#   add_recipe(param_rec)
# 
# rf_fit <- fit(rf_wflow, param_train)
# 
# rf_pred <- tibble(
#   .pred_class = predict(rf_fit, param_test)$.pred_class,
#   .pred_truth = param_test %>% pull(param)
# )
# 
# if (param_type %in% c('c', 'o')) {
#   rf_pred <- rf_pred %>%
#     mutate(
#       .pred_class = factor(.pred_class, levels = param_domain),
#       .pred_truth = factor(.pred_truth, levels = param_domain)
#     )
# }
# 
# rf_acc <- accuracy(rf_pred, truth = .pred_truth, estimate = .pred_class)
# 
# 

