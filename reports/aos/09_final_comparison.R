library(FlowshopSolveR)
library(tidyverse)
library(here)
library(wrapr)
library(irace)
library(purrr)

plan(remote, workers = rep("linode2", 2), persistent = TRUE)

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
  filter(exp == "05-adaptive_perturb", algo == "igfrrmab.rds") %>%
  pull(config) %>%
  unlist()
merged_config <- adaptive_perturb_config

adaptive_ds_config <- configs_tidy %>%
  filter(exp == "01-adaptive_destruction_size", algo == "igts.rds") %>%
  pull(config) %>%
  unlist()

merged_config[names(adaptive_ds_config)] <- adaptive_ds_config

adapt_perturb_ds <- merged_config
adapt_perturb_ds['IG.Perturb.DestructionSizeStrategy'] <- 'adaptive'

# Local search         & 0.302 (0.691) & 0.282 (0.575) & 0.304 (0.615) & 0.268 (0.596) & \textbf{0.245 (0.507)}\\ % ts
# Perturbation         & \textit{0.201 (0.457)} & \textit{\textbf{0.177 (0.423)}} & \textit{0.201 (0.434)} & \textit{0.191 (0.407)} & 0.271 (0.528)\\ % mab
# Destruction size     & \textit{0.210 (0.408)} & \textit{0.202 (0.492)} & \textit{0.196 (0.407)} & \textit{\textbf{0.186 (0.430)}} & 0.249 (0.487)\\ % ts
# Neighborhood Size    & 0.267 (0.538) & 0.247 (0.505) & 0.248 (0.510) & 0.262 (0.524) & \textbf{0.228 (0.476)}\\ % mab
# Destruction position & 0.251 (0.486) & 0.246 (0.498) & 0.371 (0.537) & 0.238 (0.448) & \textbf{0.228 (0.464)}\\ % ts
# Local search focus   & 0.248 (0.470) & 0.242 (0.472) & 0.245 (0.476) & 0.244 (0.470) & \textbf{0.232 (0.485)}\\ % mab

adaptive_ls_config <- configs_tidy %>%
  filter(exp == "04-adaptive_local_search", algo == "igts.rds") %>%
  pull(config) %>%
  unlist()

merged_config[names(adaptive_ls_config)] <- adaptive_ls_config

adaptive_ns_config <- configs_tidy %>%
  filter(exp == "06-adaptive_neighborhood_size", algo == "igfrrmab.rds") %>%
  pull(config) %>%
  unlist()

merged_config[names(adaptive_ns_config)] <- adaptive_ns_config

adaptive_dp_config <- configs_tidy %>%
  filter(exp == "03-adaptive_destruction_position", algo == "igts.rds") %>%
  pull(config) %>%
  unlist()

merged_config[names(adaptive_dp_config)] <- adaptive_dp_config

adaptive_bi_config <- configs_tidy %>%
  filter(exp == "02-adaptive_best_insertion", algo == "igfrrmab.rds") %>%
  pull(config) %>%
  unlist()

merged_config[names(adaptive_bi_config)] <- adaptive_bi_config


merged_config['IG.Perturb.DestructionSizeStrategy'] <- 'adaptive'
merged_config['IG.Perturb'] <- 'adaptive'
merged_config['IG.DestructionStrategy'] <- 'adaptive_position'
merged_config['IG.Neighborhood.Strat'] <- 'adaptive'
merged_config['IG.Local.Search'] <- 'adaptive_with_adaptive_best_insertion'
adapt_all <- merged_config

train_test_sets_df <- read_rds(here("reports", "aos", "data", "train_test_sets_df.rds"))

test_configs <- tribble(
  ~ig_variant, ~adapt_variant, ~best_config,
  'ig', 'adapt-perturb-ds', as_tibble_row(adapt_perturb_ds),
  'ig', 'adapt-all', as_tibble_row(adapt_all),
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
  )
) %>% 
  expand_grid(train_test_sets_df)

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
        cache = name
      )
    }
    )
  )
