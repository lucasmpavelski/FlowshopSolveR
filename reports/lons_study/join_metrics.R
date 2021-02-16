library(FlowshopSolveR)
library(here)
library(tidyverse)

lon_configs <- read_rds(here('reports/lons_study/lon_configs.rds'))
CONFIG_ID <- 10
lon_config <- lon_configs[[CONFIG_ID]]
lons_folder <- here("data", "lons_cache")

lon_config <- lon_configs[[CONFIG_ID]]$id

all_metrics <- function(metric_type) {
  all_problems_df() %>%
    filter(
      problem %in% c('vrf-small', 'vrf-large'),
      budget == 'low',
      type == 'PERM',
      objective == 'MAKESPAN'
    ) %>%
    unnest(cols = instances) %>%
    mutate(budget = 'med', stopping_criterion = 'EVALS') %>%
    mutate(metrics = pmap_chr(., function(problem, type, objective, budget, instance, stopping_criterion, ...) {
      problem_config <- paste(c(problem, type, objective, budget, instance, stopping_criterion), collapse = ";", sep = "_")
      sprintf("%s/%s_%s_%s.rds", lons_folder, lon_config, problem_config, metric_type)
    })) %>%
    filter(file.exists(metrics)) %>%
    mutate(metrics = map(metrics, function(metric_path) {
      print(metric_path)
      metrics <- readRDS(metric_path)
      as_tibble(metrics)
    })) %>%
    unnest(metrics)
}

joined_metrics <- all_metrics("metrics") %>%
  left_join(all_metrics("clon_metrics")) %>%
  select(-all_of("neutral_no_groups"))

saveRDS(joined_metrics, sprintf("%s/%s_metrics.rds", lons_folder, lon_config))
