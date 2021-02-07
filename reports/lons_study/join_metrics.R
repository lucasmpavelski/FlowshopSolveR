library(FlowshopSolveR)
library(here)
library(tidyverse)

lon_configs <- read_rds(here('reports/lons_study/lon_configs.rds'))
CONFIG_ID <- 2
lon_config <- lon_configs[[CONFIG_ID]]
lons_folder <- here("data", "lons_cache")

lon_config <- lon_configs[[CONFIG_ID]]$id

all_metrics <- all_problems_df() %>%
  filter(
    problem %in% c('vrf-small', 'vrf-large'),
    budget == 'low',
    type == 'PERM',
    objective == 'MAKESPAN'
  ) %>%
  unnest(cols = instances) %>%
  mutate(budget = 'med', stopping_criterion = 'EVALS') %>%
  mutate(metrics_path = pmap_chr(., function(problem, type, objective, budget, instance, stopping_criterion, ...) {
    problem_config <- paste(c(problem, type, objective, budget, instance, stopping_criterion), collapse = ";", sep = "_")
    sprintf("%s/%s_%s_metrics.rds", lons_folder, lon_config, problem_config)
  })) %>%
  filter(file.exists(metrics_path)) %>%
  mutate(metrics = map(metrics_path, function(metric_path) {
    print(metrics_path)
    metrics <- readRDS(metric_path)
    as_tibble(metrics)
  })) %>%
  unnest(metrics)

saveRDS(all_metrics, sprintf("%s/%s_metrics.rds", lons_folder, lon_config))
