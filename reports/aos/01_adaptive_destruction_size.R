library(FlowshopSolveR)
library(tidyverse)
library(here)
library(wrapr)
library(irace)
library(furrr)

NCORES <- 16
options(parallelly.debug = TRUE)
plan(remote, workers = rep("linode2", NCORES), persistent = TRUE)
# plan(sequential)

run_irace <- function(name, params, problems, ...) {
  dir.create(dirname(name), recursive = T, showWarnings = F)
  set.seed(65487873)
  train_best_solver(
    problem_space = ProblemSpace(problems = problems$problem_space),
    algorithm = Algorithm(
      name = 'IG',
      parameters = readParameters(text = params)
    ),
    solve_function = fsp_solver_performance,
    irace_scenario = defaultScenario(list(
      maxExperiments = 5000
    )),
    parallel = 8,
    cache = name,
    recover = T
  )
}

ig_variants <-     tribble(
  ~ig_variant, ~base_config,
  'ig', '
  IG.Init                            "" c (neh)
  IG.Init.NEH.Ratio                  "" c (0)
  IG.Init.NEH.Priority               "" c (sum_pij)
  IG.Init.NEH.PriorityOrder          "" c (incr)
  IG.Init.NEH.PriorityWeighted       "" c (no)
  IG.Init.NEH.Insertion              "" c (first_best)
  IG.Comp.Strat                      "" c (strict)
  IG.Neighborhood.Size               "" c (1.0)
  IG.Neighborhood.Strat              "" c (ordered)
  IG.LS.Single.Step                  "" c (0)
  IG.Accept                          "" c (temperature)
  IG.Accept.Better.Comparison        "" c (strict)
  IG.Accept.Temperature              "" c (0.25)
  IG.Perturb.Insertion               "" c (random_best)
  IG.Perturb                         "" c (rs)
  
  IG.DestructionStrategy             "" c (random)
  
  IG.Local.Search                      "" c (best_insertion)
  
  IG.Perturb.DestructionSize         "" c (4)
  IG.Perturb.DestructionSizeStrategy "" c (adaptive)
  
  IG.AOS.WarmUp                "" c (0, 1000, 2000)
  IG.AOS.WarmUp.Strategy       "" c (random)
  IG.AOS.RewardType            "" c (0,1,2,3) 
  IG.AOS.Options               "" c (2_4, 4_6, 2_4_6, 4_8)
      '
  # 'ig-lsps', '
  # IG.Init                            "" c (neh)
  # IG.Init.NEH.Ratio                  "" c (0)
  # IG.Init.NEH.Priority               "" c (sum_pij)
  # IG.Init.NEH.PriorityOrder          "" c (incr)
  # IG.Init.NEH.PriorityWeighted       "" c (no)
  # IG.Init.NEH.Insertion              "" c (last_best)
  # IG.Comp.Strat                      "" c (strict)
  # IG.Neighborhood.Size               "" c (1.0)
  # IG.Neighborhood.Strat              "" c (ordered)
  # IG.LS.Single.Step                  "" c (0)
  # IG.Accept                          "" c (temperature)
  # IG.Accept.Better.Comparison        "" c (strict)
  # IG.Accept.Temperature              "" c (0.25)
  # IG.Perturb.Insertion               "" c (first_best)
  # IG.Perturb                         "" c (lsps)
  # 
  # IG.LSPS.Local.Search               "" c (best_insertion)
  # IG.LSPS.Single.Step                "" c (0)
  # 
  # IG.DestructionStrategy             "" c (random)
  # 
  # IG.Local.Search                      "" c (best_insertion)
  # 
  # IG.Perturb.DestructionSize         "" c (4)
  # IG.Perturb.DestructionSizeStrategy "" c (adaptive)
  # 
  # IG.AOS.WarmUp                "" i (0,2000) | IG.Perturb.DestructionSizeStrategy == "adaptive"
  # IG.AOS.WarmUp.Strategy       "" c (random, fixed) | IG.Perturb.DestructionSizeStrategy == "adaptive"
  # IG.AOS.RewardType            "" c (0,1,2,3) | IG.Perturb.DestructionSizeStrategy == "adaptive"  & IG.AOS.Strategy != "random"
  # IG.AOS.Options               "" c (2_4, 4_6, 2_4_6, 4_8) | IG.Perturb.DestructionSizeStrategy == "adaptive" & IG.AOS.Strategy != "random"
  # '
)


adapt_variants <- tribble(
  ~adapt_variant, ~params,
  'ts', '
  IG.AOS.Strategy              "" c (thompson_sampling)
  IG.AOS.TS.Strategy           "" c (static, dynamic) | IG.Perturb.DestructionSizeStrategy == "adaptive" & IG.AOS.Strategy == "thompson_sampling"
  IG.AOS.TS.C                  "" i (1,500)  | IG.Perturb.DestructionSizeStrategy == "adaptive" & IG.AOS.Strategy == "thompson_sampling" & IG.AOS.TS.Strategy == "dynamic"
  ' ,
  'pm', '
  IG.AOS.Strategy              "" c (probability_matching)
  IG.AOS.PM.RewardType         "" c (avgabs,avgnorm,extabs,extnorm)  | IG.Perturb.DestructionSizeStrategy == "adaptive" & IG.AOS.Strategy == "probability_matching"
  IG.AOS.PM.Alpha              "" r (0.1, 0.9) | IG.Perturb.DestructionSizeStrategy == "adaptive" & IG.AOS.Strategy == "probability_matching"
  IG.AOS.PM.PMin               "" r (0.05, 0.2) | IG.Perturb.DestructionSizeStrategy == "adaptive" & IG.AOS.Strategy == "probability_matching"
  IG.AOS.PM.UpdateWindow       "" i (1,500) | IG.Perturb.DestructionSizeStrategy == "adaptive" & IG.AOS.Strategy == "probability_matching"
  ',
  'frrmab', '
  IG.AOS.Strategy              "" c (frrmab)
  IG.AOS.FRRMAB.WindowSize     "" i (10, 500) | IG.Perturb.DestructionSizeStrategy == "adaptive" & IG.AOS.Strategy == "frrmab"
  IG.AOS.FRRMAB.Scale          "" r (0.01, 100) | IG.Perturb.DestructionSizeStrategy == "adaptive" & IG.AOS.Strategy == "frrmab"
  IG.AOS.FRRMAB.Decay          "" r (0.25, 1.0) | IG.Perturb.DestructionSizeStrategy == "adaptive" & IG.AOS.Strategy == "frrmab"
  ',
  'linucb', '
  IG.AOS.Strategy              "" c (linucb)
  IG.AOS.LINUCB.Alpha          "" r (0.0, 1.5) | IG.Perturb.DestructionSizeStrategy == "adaptive" & IG.AOS.Strategy == "linucb"
  ',
  'epsilon_greedy', '
  IG.AOS.Strategy              "" c (epsilon_greedy)
  IG.AOS.EpsilonGreedy.Epsilon "" r (0.0, 1.0)
  ' #,
  # 'random', '
  # IG.AOS.Strategy              "" c (random)
  # '
)

exp_folder <- here("reports", "aos", "data", "01-adaptive_destruction_size")
perf_folder <- file.path(exp_folder, "perf")
irace_folder <- file.path(exp_folder, "irace")

dir.create(perf_folder, recursive = T, showWarnings = F)
dir.create(irace_folder, recursive = T, showWarnings = F)

train_test_sets_df <- read_rds(here("reports", "aos", "data", "train_test_sets_df.rds"))

strategy_params <- train_test_sets_df %>%
  filter(set_type == "train") %>%
  expand_grid(
    ig_variants,
    adapt_variants
  ) %>%
  mutate(
    params = paste(base_config, params),
    name = here(irace_folder, path, paste0(ig_variant, adapt_variant, ".rds"))
  ) %>%
  mutate(best_config = pmap(., run_irace,
                                   .progress = TRUE,
                                   .options = furrr_options(seed = TRUE)))
  # mutate(best_config = pmap(., run_irace))


ig_default_random <- tibble(
  IG.Init                            = "neh",
  IG.Init.NEH.Ratio                  = "0",
  IG.Init.NEH.Priority               = "sum_pij",
  IG.Init.NEH.PriorityOrder          = "incr",
  IG.Init.NEH.PriorityWeighted       = "no",
  IG.Init.NEH.Insertion              = "first_best",
  IG.Comp.Strat                      = "strict",
  IG.Neighborhood.Size               = "1.0",
  IG.Neighborhood.Strat              = "ordered",
  IG.LS.Single.Step                  = "0",
  IG.Accept                          = "temperature",
  IG.Accept.Better.Comparison        = "strict",
  IG.Accept.Temperature              = "0.25",
  IG.Perturb.Insertion               = "random_best",
  IG.Perturb                         = "rs",
  IG.DestructionStrategy             = "random",
  IG.Perturb.DestructionSize         = "4",
  IG.Local.Search                    = "best_insertion",
  IG.Perturb.DestructionSizeStrategy = "adaptive",
  IG.AOS.Strategy                    = "random",
  IG.AOS.WarmUp                      = "0",
  IG.AOS.WarmUp.Strategy             = "random",
  IG.AOS.RewardType                  = "0"
)

ig_default_configs <- tribble(
  ~ig_variant, ~adapt_variant, ~best_config,
  'ig', 'default', tibble(
    IG.Init                            = "neh",
    IG.Init.NEH.Ratio                  = "0",
    IG.Init.NEH.Priority               = "sum_pij",
    IG.Init.NEH.PriorityOrder          = "incr",
    IG.Init.NEH.PriorityWeighted       = "no",
    IG.Init.NEH.Insertion              = "first_best",
    IG.Comp.Strat                      = "strict",
    IG.Neighborhood.Size               = "1.0",
    IG.Neighborhood.Strat              = "ordered",
    IG.LS.Single.Step                  = "0",
    IG.Accept                          = "temperature",
    IG.Accept.Better.Comparison        = "strict",
    IG.Accept.Temperature              = "0.25",
    IG.Perturb.Insertion               = "random_best",
    IG.Perturb                         = "rs",
    IG.Perturb.DestructionSizeStrategy = "fixed",
    IG.Perturb.DestructionSize         = "4",
    IG.DestructionStrategy             = "random",
    IG.Local.Search                    = "best_insertion"
  ),
  'ig', 'random-2_4', mutate(ig_default_random, IG.AOS.Options = "2_4"),
  'ig', 'random-4_6', mutate(ig_default_random, IG.AOS.Options = "4_6"),
  'ig', 'random-2_4_6', mutate(ig_default_random, IG.AOS.Options = "2_4_6"),
  'ig', 'random-4_8', mutate(ig_default_random, IG.AOS.Options = "4_8")
)






# tuned_perf <- test_configs %>%
#   inner_join(
#     train_test_sets_df %>%
#       filter(set_type == "test"),
#     by = "path"
#   ) %>%
#   mutate(
#     sampled_performance = pmap(., function(name, problems, best_config, ...) {
#         dir.create(dirname(name), recursive = T, showWarnings = F)
#         set.seed(79879874)
#         cat("Running ", name, "\n")
#         sample_performance(
#           algorithm = get_algorithm("IG"),
#           problemSpace = ProblemSpace(problems = problems$problem_space),
#           config = best_config,
#           solve_function = fsp_solver_performance,
#           no_samples = 10,
#           cache = name,
#           parallel = NCORES
#         )
#       }
#     )
#   )
test_configs <- strategy_params %>%
  bind_rows(ig_default_configs) %>%
  select(ig_variant, adapt_variant, best_config) %>%
  mutate(path = "all") %>%
  mutate(
    best_config = map(best_config, removeConfigurationsMetaData),
    best_config = map(best_config, ~df_to_character(.x[1,])),
    name = here(exp_folder, "extra", path, paste0(ig_variant, adapt_variant, ".rds"))
  ) %>%
  mutate(best_config = map(best_config, ~{
    .x['IG.Init.NEH.PriorityOrder'] <- 'decr'
    .x['IG.Accept.Temperature'] <- '0.5'
    .x
  }))

tuned_perf <- test_configs %>%
  inner_join(
    train_test_sets_df %>%
      filter(set_type == "extra"),
    by = "path"
  ) %>%
  mutate(
    sampled_performance = pmap(., function(name, problems, best_config, ...) {
      dir.create(dirname(name), recursive = T, showWarnings = F)
      set.seed(79879874)
      cat("Running ", name, "\n")
      sample_performance(
        algorithm = get_algorithm("IG"),
        problemSpace = ProblemSpace(problems = problems$problem_space),
        config = best_config,
        solve_function = fsp_solver_performance,
        no_samples = 10,
        cache = name,
        parallel = NCORES
      )
    }
    )
  )

