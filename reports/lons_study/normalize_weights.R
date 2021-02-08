library(FlowshopSolveR)
library(here)
library(tidygraph)
library(furrr)

lon_configs <- read_rds(here('reports/lons_study/lon_configs.rds'))
CONFIG_ID <- 9
lons_folder <- here("data", "lons_cache")

normalize_weights <- function(lons_folder, problem_config, lon_config) {
  save_path <- sprintf("%s/%s_%s_wfull.rds", lons_folder, lon_config, problem_config)
  if (!file.exists(save_path)) {
    full_lon <- readRDS(sprintf("%s/%s_%s_full.rds", lons_folder, lon_config, problem_config))
    full_lon$edges <- full_lon$edges %>% 
      group_by(from) %>% 
      mutate(weight = weight / sum(weight)) %>%
      ungroup()
    saveRDS(full_lon, file = save_path)
    rm(full_lon)
  }
  save_path
}

problems <- all_problems_df() %>%
  filter(
    problem == 'vrf-small',
    budget == 'low',
    type == 'PERM',
    objective == 'MAKESPAN'
  ) %>%
  unnest(cols = instances) %>%
  mutate(budget = 'med', stopping_criterion = 'EVALS') %>%
  mutate(problem_id = pmap(., function(problem, type, objective, budget, instance, stopping_criterion, ...) {
    paste(c(problem, type, objective, budget, instance, stopping_criterion), collapse = ";", sep = "_")
  }))


library(progressr)
handlers(global = T)
handlers("progress")

# plan(multisession(workers = 2))

normalize_all_weights <- function() {
  p <- progressor(along = seq_along(problems$problem_id))
  y <- map(seq_along(problems$problem_id), function(i) {
    prob <- problems$problem_id[[i]]
    lon <- lon_configs[[CONFIG_ID]]$id
    normalize_weights(lons_folder, prob, lon)
    p(sprintf("x=%g", i))
  })
}

normalize_all_weights()

