library(FlowshopSolveR)
library(tidyverse)
library(here)
library(wrapr)
library(irace)
library(purrr)

NCORES <- 32
options(parallelly.debug = TRUE)
# plan(sequential)
plan(remote, workers = rep("linode2", NCORES), persistent = TRUE)

experiments <- c(
  "01-adaptive_destruction_size",
  "02-adaptive_best_insertion",
  "03-adaptive_destruction_position",
  "04-adaptive_local_search",
  "05-adaptive_perturb",
  "06-adaptive_neighborhood_size"
)

load_config <- function(config_path) {
  read_rds(config_path)[1,] %>%
    df_to_character()
}

load_configs <- function(irace_folder, path = "all") {
  tibble(path = path,
         full_path = file.path(irace_folder, path)) %>%
    select(path, full_path) %>%
    mutate(configs = map(full_path, function(path) {
      tibble(algo = dir(path)) %>%
        filter(!str_detect(algo, "_log|_rec")) %>%
        mutate(config = map(file.path(path, algo), load_config))
    })) %>%
    unnest(configs)
}

configs_tidy <- map_dfr(experiments, ~ {
  here("reports", "aos", "data", .x) %>%
    file.path("irace") %>%
    load_configs() %>%
    mutate(exp = .x) %>%
    select(exp, algo, config)
})

adaptive_perturb_config <- configs_tidy %>%
  filter(exp == "05-adaptive_perturb", algo == "igts.rds") %>%
  pull(config) %>%
  unlist()
merged_config <- adaptive_perturb_config

# adaptive_ds_config <- configs_tidy %>%
#   filter(exp == "01-adaptive_destruction_size", algo == "iglinucb.rds") %>%
#   pull(config) %>%
#   unlist()

adaptive_ds_config <- c(
  IG.AOS.Strategy                    = "random",
  IG.AOS.WarmUp                      = "0",
  IG.AOS.WarmUp.Strategy             = "random",
  IG.AOS.RewardType                  = "0",
  IG.AOS.RewardType                  = "0",
  IG.AOS.Options = "4_8"
)

merged_config[names(adaptive_ds_config)] <- adaptive_ds_config

adapt_perturb_ds <- merged_config
adapt_perturb_ds['IG.Perturb.DestructionSizeStrategy'] <- 'adaptive'
adapt_perturb_ds['IG.Perturb.DestructionSizeStrategy.AOS.Strategy'] <- 'random'
adapt_perturb_ds['IG.Perturb.DestructionSizeStrategy.AOS.Options'] <- '4_8'

adaptive_ls_config <- configs_tidy %>%
  filter(exp == "04-adaptive_local_search", algo == "igts.rds") %>%
  pull(config) %>%
  unlist()

merged_config[names(adaptive_ls_config)] <- adaptive_ls_config

adaptive_ns_config <- configs_tidy %>%
  filter(exp == "06-adaptive_neighborhood_size", algo == "igfrrmab.rds") %>%
  pull(config) %>%
  unlist()
adapt_perturb_ds['IG.AdaptiveNeighborhoodSize.AOS.Strategy'] <- 'random'
adapt_perturb_ds['IG.AdaptiveNeighborhoodSize.AOS.NoArms'] <- '2'

merged_config[names(adaptive_ns_config)] <- adaptive_ns_config

adaptive_dp_config <- configs_tidy %>%
  filter(exp == "03-adaptive_destruction_position", algo == "igts.rds") %>%
  pull(config) %>%
  unlist()

merged_config[names(adaptive_dp_config)] <- adaptive_dp_config

adaptive_bi_config <- configs_tidy %>%
  filter(exp == "02-adaptive_best_insertion", algo == "igepsilon_greedy.rds") %>%
  pull(config) %>%
  unlist()

merged_config[names(adaptive_bi_config)] <- adaptive_bi_config


merged_config['IG.Perturb.DestructionSizeStrategy'] <- 'adaptive'
merged_config['IG.Perturb'] <- 'adaptive'
merged_config['IG.DestructionStrategy'] <- 'adaptive_position'
merged_config['IG.Neighborhood.Strat'] <- 'adaptive'
merged_config['IG.Local.Search'] <- 'adaptive_with_adaptive_best_insertion'

merged_config['IG.Init.NEH.PriorityOrder'] <- 'decr'
merged_config['IG.Accept.Temperature'] <- '0.5'

adapt_all <- merged_config

adapt_no_ls <- merged_config
adapt_no_ls['IG.Local.Search'] <- 'best_insertion'


train_test_sets_df <- read_rds(here("reports", "aos", "data", "train_test_sets_df.rds"))

test_configs <- tribble(
  ~ig_variant, ~adapt_variant, ~best_config,
  'ig', 'adapt-no-local-search-final', as_tibble_row(adapt_no_ls),
  'ig', 'adapt-all-arpd-final', as_tibble_row(adapt_all),
  'ig', 'default', tibble(
    IG.Init                            = "neh",
    IG.Init.NEH.Ratio                  = "0",
    IG.Init.NEH.Priority               = "sum_pij",
    IG.Init.NEH.PriorityOrder          = "incr",
    # IG.Init.NEH.PriorityOrder          = "decr",
    IG.Init.NEH.PriorityWeighted       = "no",
    IG.Init.NEH.Insertion              = "first_best",
    IG.Comp.Strat                      = "strict",
    IG.Neighborhood.Size               = "1.0",
    IG.Neighborhood.Strat              = "ordered",
    IG.LS.Single.Step                  = "0",
    IG.Accept                          = "temperature",
    IG.Accept.Better.Comparison        = "strict",
    IG.Accept.Temperature              = "0.25",
    # IG.Accept.Temperature              = "0.5",
    IG.Perturb.Insertion               = "random_best",
    IG.Perturb                         = "rs",
    IG.Perturb.DestructionSizeStrategy = "fixed",
    IG.Perturb.DestructionSize         = "4",
    IG.DestructionStrategy             = "random",
    IG.Local.Search                    = "best_insertion"
  )
) %>% 
  expand_grid(train_test_sets_df %>%
                filter(set_type == "test"))

exp_folder <- here("reports", "aos", "data", "09-final_comparison")
perf_folder <- file.path(exp_folder, "perf")

dir.create(perf_folder, recursive = T, showWarnings = F)



tuned_perf <- test_configs %>%
  select(path, best_config, ig_variant, adapt_variant) %>%
  mutate(
    best_config = map(best_config, removeConfigurationsMetaData),
    best_config = map(best_config, ~df_to_character(.x[1,])),
    name = here(perf_folder, path, paste0(ig_variant, adapt_variant, ".rds"))
  ) %>%
  inner_join(
    train_test_sets_df %>%
      filter(set_type == "test"),
    by = "path"
  ) %>%
  mutate(
    sampled_performance = pmap(., function(name, problems, best_config, ...) {
      dir.create(dirname(name), recursive = T, showWarnings = F)
      set.seed(79879874)
      sample_performance(
        algorithm = get_algorithm("IG"),
        problemSpace = ProblemSpace(problems = problems$problem_space),
        config = best_config,
        solve_function = fsp_solver_performance,
        no_samples = 10,
        cache = name,
        parallel = TRUE
      )
    }
    )
  )

test_configs <- test_configs %>%
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
        parallel = TRUE
      )
    }
    )
  )
