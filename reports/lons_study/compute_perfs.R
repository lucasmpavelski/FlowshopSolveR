library(tidyverse)
library(FlowshopSolveR)

IG_RS_CONFIG <- c(
  IG.Init                            = "random",
  IG.Init.NEH.Priority               = "sum_pij",
  IG.Init.NEH.PriorityOrder          = "incr",
  IG.Init.NEH.PriorityWeighted       = "0",
  IG.Init.NEH.Insertion              = "first_best",
  IG.Comp.Strat                      = "strict",
  IG.Neighborhood.Size               = "1",
  IG.Neighborhood.Strat              = "ordered",
  IG.Local.Search                    = "best_insertion",
  IG.LS.Single.Step                  = "0",
  IG.Accept                          = "better",
  IG.Accept.Better.Comparison        = "strict",
  IG.Accept.Temperature              = "0.25",
  IG.Perturb                         = "rs",
  IG.Perturb.DestructionSizeStrategy = "fixed",
  IG.Perturb.DestructionSize         = "4",
  IG.Perturb.Insertion               = "first_best"
)

IG_LSPS_CONFIG <- c(
  IG.Init                            = "random",
  IG.Init.NEH.Priority               = "sum_pij",
  IG.Init.NEH.PriorityOrder          = "incr",
  IG.Init.NEH.PriorityWeighted       = "0",
  IG.Init.NEH.Insertion              = "first_best",
  IG.Comp.Strat                      = "strict",
  IG.Neighborhood.Size               = "1",
  IG.Neighborhood.Strat              = "ordered",
  IG.Local.Search                    = "best_insertion",
  IG.LS.Single.Step                  = "0",
  IG.Accept                          = "better",
  IG.Accept.Better.Comparison        = "strict",
  IG.Accept.Temperature              = "0.25",
  IG.Perturb                         = "lsps",
  IG.Perturb.DestructionSizeStrategy = "fixed",
  IG.Perturb.DestructionSize         = "2",
  IG.Perturb.Insertion               = "first_best",
  IG.LSPS.Local.Search               = "best_insertion",
  IG.LSPS.Single.Step                = "0"
)

problems <- all_problems_df() %>%
  crossing(sample_n = 1:100) %>%
  filter(
    problem %in% c('vrf-small', 'vrf-large'),
    budget == 'low',
    type == 'PERM',
    objective == 'MAKESPAN',
    no_jobs == 300
  ) %>%
  mutate(metaopt_problem = pmap(., as_metaopt_problem))

config_perf <- function(problem, instance, algorithm, config) {
  seed <- as.integer(runif(1) * 1000000)
  print(instance)
  fsp_solver_performance(algorithm, config, instance, problem, seed)$cost
}

set.seed(659878)
perfs <- problems %>%
  unnest(instances) %>%
  mutate(
    ig_rs_perf = map2_dbl(
      metaopt_problem, 
      instance,
      config_perf,
      algorithm = get_algorithm('IG'),
      config = IG_RS_CONFIG
    ),
    ig_lsps_perf = map2_dbl(
      metaopt_problem, 
      instance,
      config_perf,
      algorithm = get_algorithm('IG'),
      config = IG_LSPS_CONFIG
    )
  )

perfs_large <- perfs

perfs <- read_rds(here('reports/lons_study/relative_perfs_all2.rds'))

perfs <- bind_rows(perfs, perfs_large)


best_fitness <- read_rds(here('reports/lons_study/best-fitness.rds')) %>%
  mutate(budget = 'low', stopping_criterion = "TIME")

relative_perfs <- perfs %>%
  right_join(best_fitness) %>%
  mutate(
    ig_rs_rpd = 100 * ((ig_rs_perf - best_fitness) / best_fitness),
    ig_lsps_rpd = 100 * ((ig_lsps_perf - best_fitness) / best_fitness)
  )


# lower_bounds <- bind_rows(
#     read_csv(here("data", "instances", "vrf-small", "lower_bounds.csv")),
#     read_csv(here("data", "instances", "vrf-large", "lower_bounds.csv"))
#   ) %>%
#   rename(instance = Instance) %>%
#   mutate(instance = paste0(instance, '.txt')) %>%
#   mutate(instance = str_replace(instance, 'VFR', 'VRF'))
# 
# 
# relative_perfs <- perfs %>%
#   inner_join(lower_bounds, by = "instance") %>%
#   mutate(
#     ig_rs_rpd = 100 * ((ig_rs_perf - UB) / UB),
#     ig_lsps_rpd = 100 * ((ig_lsps_perf - UB) / UB)
#   )
# 
# # View(relative_perfs)
# 
saveRDS(relative_perfs, here('reports/lons_study/relative_perfs_best.rds'))
