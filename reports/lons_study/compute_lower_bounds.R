library(here)
library(tidyverse)
library(wrapr)
library(FlowshopSolveR)

unite_lons <- function(lons_folder, problem_config, lon_config) {
  savePath <- sprintf("%s/%s_%s_wfull.rds", lons_folder, lon_config, problem_config)
  savePath
}

problems <- all_problems_df() %>%
  filter(
    problem %in% c('vrf-large'), #, 'vrf-small'),
    budget == 'low',
    type == 'PERM',
    objective == 'MAKESPAN',
    no_jobs <= 300
  ) %>%
  unnest(cols = instances) %>%
  mutate(budget = 'med', stopping_criterion = 'EVALS') %>%
  mutate(problem_id = pmap(., function(problem, type, objective, budget, instance, stopping_criterion, ...) {
    paste(c(problem, type, objective, budget, instance, stopping_criterion), collapse = ";", sep = "_")
  }))

read_best_fitness <- function(clon_path) {
  clon <- read_rds(clon_path)
  print(clon_path)
  min(clon$nodes$fitness)
}

lon_configs <- read_rds(here('reports/lons_study/lon_configs.rds'))[c(1, 2, 9, 10)]

lons_folder <- here("data", "lons_cache")


best_fitness_per_sample <- problems %>%
  crossing(sample = map_chr(lon_configs, ~.x$id)) %>%
  mutate(
    lon_path = file.path(lons_folder, sprintf("%s_%s_clon.rds", sample, problem_id)),
    best_fitness = map_dbl(lon_path, read_best_fitness)
  )


best_fitness_per_problem <- best_fitness_per_sample %>%
  select(-sample, -lon_path) %>%
  group_by(across(-all_of("best_fitness"))) %>%
  summarise(best_fitness = min(best_fitness))


write_rds(best_fitness_per_problem, here('reports/lons_study/best-fitness.rds'))

