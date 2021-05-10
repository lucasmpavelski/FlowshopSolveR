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
  IG.Perturb.Insertion               = "first_best",
  IG.DestructionStrategy             = "random"
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
  IG.LSPS.Single.Step                = "0",
  IG.DestructionStrategy             = "random"
)

problems <- all_problems_df() %>%
  crossing(sample_n = 1:10) %>%
  filter(
    problem %in% c('vrf-small', 'vrf-large'),
    budget == 'low',
    type == 'PERM',
    objective == 'MAKESPAN',
    no_jobs <= 300
  ) %>%
  mutate(stopping_criterion = 'FIXEDTIME') %>%
  mutate(metaopt_problem = pmap(., as_metaopt_problem))

config_perf <- function(problem, instance, algorithm, config) {
  seed <- as.integer(runif(1) * 1000000)
  cost <- fsp_solver_performance(algorithm, config, instance, problem, seed)$cost
  # cat(
  #   paste(
  #     algorithm@name,
  #     paste0(names(config), "=", config, collapse = "&"),
  #     paste0(names(problem@data), "=", problem@data, collapse = "&"),
  #     instance,
  #     seed,
  #     cost,
  #     sep = ", "
  #   ),
  #   "\n"
  # )
  cost
}
# workers <- makeClusterPSOCK(workers = rep("localhost", 3), outfile = "log.txt")
# plan(remote, workers = rep("localhost", 3), persistent = TRUE)
plan(multisession, workers = 32)
# plan(remote, workers = workers, homogeneous = F)
# plan(sequential)

set.seed(659878)
perfs_ig <- problems %>%
  unnest(instances) %>%
  mutate(
    ig_rs_perf = future_map2_dbl(
      metaopt_problem, 
      instance,
      config_perf,
      algorithm = get_algorithm('IG'),
      config = IG_RS_CONFIG,
      .progress = F,
      .options = furrr_options(
        seed = T
      )
    )
  )

perfs_lsps <- problems %>%
  unnest(instances) %>%
  mutate(
    ig_lsps_perf = future_map2_dbl(
      metaopt_problem, 
      instance,
      config_perf,
      algorithm = get_algorithm('IG'),
      config = IG_LSPS_CONFIG,
      .progress = T,
      .options = furrr_options(
        seed = T
      )
    )
  )

perfs <- bind_rows(perfs_ig, perfs_lsps)

# relative_perfs <- perfs %>%
#   right_join(best_fitness) %>%
#   mutate(
#     ig_rs_rpd = 100 * ((ig_rs_perf - best_fitness) / best_fitness),
#     ig_lsps_rpd = 100 * ((ig_lsps_perf - best_fitness) / best_fitness)
#   )


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
saveRDS(perfs, here('reports/lons_study/perfs_fixedtime_low_10.rds'))
