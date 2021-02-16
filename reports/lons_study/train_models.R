library(tidyverse)
library(tidymodels)
library(here)
library(FlowshopSolveR)
library(vip)

lon_configs <- read_rds(here('reports/lons_study/lon_configs.rds'))
CONFIG_ID <- 10
lons_folder <- here("data", "lons_cache")

set.seed(6547)

perfs <- read_rds(here('reports/lons_study/relative_perfs_best.rds')) %>%
  select(-metaopt_problem, -budget, -stopping_criterion) %>%
  group_by(dist, corr, no_jobs, no_machines, problem, corv, 
           objective, type, instance, model, instance_features, id) %>%
  summarise(
    ig_rs_rpd = mean(ig_rs_rpd),
    ig_lsps_rpd = mean(ig_lsps_rpd),
    .groups = 'drop'
  )

metrics <- read_rds(sprintf("%s/%s_metrics.rds", lons_folder, lon_configs[[CONFIG_ID]]
                            $id)) %>%
  mutate(
    stopping_criterion = 'TIME'
  )

problem_set <- 'all'
all_data <- perfs %>% 
  filter((problem == problem_set) | (problem_set == 'all')) %>%
  inner_join(metrics) %>%
  select(
    -dist,
    -corr,
    -problem,
    -corv,
    -objective,
    -type,
    -instance,
    -model,
    -instance_features,
    -id,
    -stopping_criterion,
    -budget,
    -inst_n
  ) %>%
  mutate(
    perf = log(ig_lsps_rpd + 1e-6),
    compress_rate = clon_no_nodes / no_nodes,
    average_weight_of_self_loops = if_else(is.nan(average_weight_of_self_loops), 0, average_weight_of_self_loops),
    graph_assortativity_degree = if_else(is.nan(graph_assortativity_degree), 0, graph_assortativity_degree),
    fitness_fitness_correlation = if_else(is.nan(fitness_fitness_correlation) | is.na(fitness_fitness_correlation), 0, fitness_fitness_correlation)
  ) %>%
  select(-ig_lsps_rpd, -ig_rs_rpd)


model <- rand_forest(
  mtry = tune(),
  trees = tune(),
  min_n = tune()
) %>%
  set_engine("ranger", importance = "impurity") %>%
  set_mode("regression")

tune_grid <- grid_regular(
  mtry(c(1, 10)),
  trees(),
  min_n(),
  levels = 3
)

splitted_data <- initial_split(all_data, 0.8, strata = perf)

train_dt <- training(splitted_data)

param_rec <- recipe(perf ~ ., data = train_dt) %>% 
  step_nzv(all_predictors()) %>%
  # step_BoxCox(all_numeric()) %>%
  # step_lincomb(all_numeric()) %>%
  step_corr(all_numeric())

train_dt_preprocessing <- prep(param_rec, training = train_dt)

dt_wflow <-
  workflow() %>%
  add_model(model) %>%
  add_recipe(train_dt_preprocessing)

dt_fit <-
  dt_wflow %>%
  tune_grid(
    resamples = vfold_cv(train_dt),
    grid = tune_grid
  )

best_dt <- dt_fit %>%
  select_best("rsq")

final_wf <-
  dt_wflow %>%
  finalize_workflow(best_dt)

final_tree <-
  final_wf %>%
  fit(data = train_dt)

last_rf_fit <- last_fit(final_tree, splitted_data) 

metrics <- last_rf_fit %>%
  collect_metrics() %>%
  print()

last_rf_fit %>% 
  pluck(".workflow", 1) %>%   
  pull_workflow_fit() %>% 
  vip(num_features = 20)


test_dt <- testing(splitted_data)

write_rds(last_rf_fit, here('reports/lons_study/models/ig_rs/',
  sprintf('last_rf_fit_%s_%s.rds', lon_configs[[CONFIG_ID]]$id, problem_set)))

# test_dt_bake <- bake(train_dt_preprocessing, new_data = test_dt)
# 
# test_dt_pred <- tibble(
#   pred_estimate = predict(final_tree, test_dt)$.pred,
#   pred_truth = test_dt %>% pull(ig_rs_rpd)
# )
# 
# print(rsq_vec(test_dt_pred$pred_estimate, test_dt_pred$pred_truth))

# train_dt_pred <- tibble(
#   pred_estimate = predict(final_tree, train_dt)$.pred_class,
#   pred_truth = train_dt %>% pull(param)
# )


