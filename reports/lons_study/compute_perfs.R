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
    problem == 'vrf-small',
    budget == 'low',
    type == 'PERM',
    objective == 'MAKESPAN'
  ) %>%
  mutate(metaopt_problem = pmap(., as_metaopt_problem))

config_perf <- function(problem, instance, algorithm, config) {
  seed <- as.integer(runif(1) * 1000000)
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

lower_bounds <- read_csv(here("data", "instances", "vrf-small", "lower_bounds.csv")) %>%
  rename(instance = Instance) %>%
  mutate(instance = paste0(instance, '.txt')) %>%
  mutate(instance = str_replace(instance, 'VFR', 'VRF'))


relative_perfs <- perfs %>%
  inner_join(lower_bounds, by = "instance") %>%
  mutate(
    ig_rs_rpd = 100 * ((ig_rs_perf - UB) / UB),
    ig_lsps_rpd = 100 * ((ig_lsps_perf - UB) / UB)
  )

View(relative_perfs)

saveRDS(relative_perfs, here('reports/lons_study/relative_perfs.rds'))
