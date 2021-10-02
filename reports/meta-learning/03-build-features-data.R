library(tidyverse)
library(FlowshopSolveR)
library(here)
library(future)
library(schedulingInstancesGeneratoR)
library(metaOpt)

FEATURES_FOLDER <- here('data', 'features')
DATA_FOLDER <- here('data')

summarize_instance_features <- function(inst_dt) {
  summ <- inst_dt %>%
    unnest(instance_calc_features) %>%
    select(-instance) %>%
    summarise(across(everything(), mean))
  summ
}

generate_instance_features <- function(problem_space = all_problems_df()) {
  all_instance_features <- problem_space %>%
    unnest(instances) %>%
    select(problem, instance_features, instance) %>%
    distinct() %>%
    mutate(
      instance_path = here('data', 'instances', problem, instance),
      instance_calc_features = map(instance_path, 
                                   ~as_tibble(instance_statistics(read_txt(.x))))
    ) %>%
    select(-instance_path)
  
  summ_instance_features <- all_instance_features %>%
    group_nest(problem, instance_features) %>%
    mutate(data = map(data, summarize_instance_features)) %>%
    rename(instance_calc_features = data)
  
  problem_space %>%
    inner_join(summ_instance_features, by = c('problem', 'instance_features')) %>%
    mutate(metaopt_problem = pmap(., as_metaopt_problem)) %>%
    mutate(features = map2(metaopt_problem, instance_calc_features, function(pr, ins) {
      append_problem_features(pr, ins, features_folder = here('data', 'features'))
    }))
}

# generate_instance_features()


computeFLA <- function(problem_data, compute_func, feature_names, parallel = TRUE, use_cache = TRUE) {
  computedFLA <- problem_data %>%
    mutate(metaopt_problem = pmap(., as_metaopt_problem)) %>% 
    unnest(instances) %>%
    mutate(
      features = map2(
        metaopt_problem,
        instance, 
        load_problem_features, 
        features_folder = FEATURES_FOLDER
      )
    ) %>%
    filter(!use_cache | !map_lgl(features, ~all(feature_names %in% names(.x)))) %>%
    mutate(
      fla = pmap(., function(metaopt_problem, instance, ...) {
        if (parallel) {
          future({
            append_problem_features(
              compute_func(metaopt_problem, instance, ...),
              metaopt_problem, 
              instance, 
              features_folder = FEATURES_FOLDER
            )
          }, seed = TRUE)
        } else {
          append_problem_features(
            compute_func(metaopt_problem, instance, ...),
            metaopt_problem, 
            instance, 
            features_folder = FEATURES_FOLDER
          )
        }
      })
    )
  if (parallel) {
    value(computedFLA$fla)
  }
}


instanceFeaturesFLA <- function(metaopt_problem, instance, ...) {
  problem <- metaopt_problem@data$problem
  instance_path <- here('data', 'instances', problem, instance)
  as_tibble(instance_statistics(read_txt(instance_path)))
}

computeSolutionStatisticsFLA <- function(metaopt_problem, instance, ...) {
  prob_dt <- metaopt_problem@data
  prob_dt$instance <- instance
  stats <- sampleSolutionStatisticsFLA(
    DATA_FOLDER,
    rproblem = df_to_character(prob_dt),
    noSamples = case_when(
      prob_dt$no_jobs <= 100 ~ 100,
      prob_dt$no_jobs <= 300 ~ 50,
      prob_dt$no_jobs <= 500 ~ 30
    ),
    seed = as.integer(runif(1, 0, 2147483647))
  )
  as_tibble(stats)
}


computeRandomWalkFLA <- function(metaopt_problem, instance, ...) {
  prob_dt <- metaopt_problem@data
  prob_dt$instance <- instance
  random_walk <- sampleRandomWalk(
    DATA_FOLDER,
    rproblem = df_to_character(prob_dt),
    noSamples = 10000,
    samplingStrat = "RANDOM",
    seed = as.integer(runif(1, 0, 2147483647))
  )
  ensamble = computeEnsamble(random_walk, epsilon = 1)
  rw_fla <- tibble(
    rw_autocorr_1 = autocor(random_walk, k = 1),
    entropy = entropy(ensamble),
    partial_inf = partialInformation(ensamble),
    inf_stability = informationStability(random_walk),
    density_basin = densityBasinInformation(ensamble)
  )
}


autocor <- function(v, k = 1) {
  n <- length(v)
  cor(v[1:(n-k)], v[(k+1):n])
}

computeEnsamble <- function(rw, epsilon, ...) {
  n <- length(rw)
  ret <- rw[2:n] - rw[1:n - 1]
  case_when(
    ret < -epsilon ~ -1,
    ret > epsilon ~ 1,
    TRUE ~ 0
  )
}

frequencyOfTuple <- function(v, a, b) {
  n <- length(v)
  sum(v[1:n-1] == a & v[2:n] == b)
}

entropy <- function(v) {
  n <- length(v)
  res <- 0
  for (p in c(-1, 0, 1)) {
    for (q in c(-1, 0, 1)) {
      if (p != q) {
        p_qp = frequencyOfTuple(v, p, q) / n
        if (p_qp != 0) {
          res <- res + p_qp * log(p_qp, base = 6)
        }
      }
    }
  }
  -res
}

countSlopes <- function(s) {
  n <- length(s)
  k <- 0
  j <- 0
  for (i in seq(1, n)) {
    if (j == 0 && s[i] != 0) {
      k <- k + 1
      j <- i
    } else if (j >= 0 && s[i] != 0 && s[i] != s[j]) {
      k <- k + 1
      j <- i
    }
  }
  k
}

partialInformation <- function(s) {
  countSlopes(s) / length(s)
}

informationStability <- function(rw) {
  n <- length(rw)
  max(abs(rw[1:n-1] - rw[2:n]))
}

densityBasinInformation <- function(s) {
  n <- length(s)
  res <- 0
  for (p in c(-1, 0, 1)) {
    p_pp = frequencyOfTuple(s, p, p) / n
    if (p_pp != 0) {
      res <- res + p_pp * log(p_pp, base = 3)
    }
  }
  -res
}

computeAdaptiveWalks <- function(metaopt_problem, instance, ...) {
  aw_fla <- crossing(
      sample = 1:30,
      Init.Strat = "RANDOM",
      Sampling.Strat = "IG"
    ) %>%
    mutate(
      adaptive_walk = pmap(., function(...) {
        cfs <- list(...)
        seed <- floor(runif(1, 1, 2147483647))
        problem_params <- df_to_character(metaopt_problem@data)
        problem_params['instance'] <- instance
        sampling_params <- c(
          AdaptiveWalk.Init = "NEH",
          Sampling.Strat = "IG"
        )
        adaptiveWalk(problem_params, sampling_params, seed)
      })
    )
  aw_fla
}

calculateDistances <- function(aw) {
  n <- length(aw$fitness)
  local_optima <- unlist(aw$solutions[n])
  dt <- as_tibble(aw)[1,] %>%
    mutate(
      adj = map_dbl(solutions, adjacencyDistance, b = local_optima),
      prec = map_dbl(solutions, precedenceDistance, b = local_optima),
      abs_pos = map_dbl(solutions, absolutePositionDistance, b = local_optima),
      dev = map_dbl(solutions, deviationDistance, b = local_optima),
      shift = map_dbl(solutions, shiftDistance, b = local_optima),
      swap = map_dbl(solutions, aproximatedSwapDistance, b = local_optima),
      aw_length = map_int(solutions, ~ length(.x))
    )
  dt$aw_length <- length(aw$fitness)
  dt$local_optima <- list(local_optima)
  dt[1,]
}

computeAllDistances <- function(metaopt_problem, instance, ...) {
  aw_fla <- computeAdaptiveWalks(metaopt_problem, instance, ...) %>%
    mutate(
      adaptive_walk = map(adaptive_walk, calculateDistances)
    )
  aw_fla
}

meanWalkLength <- function(walk_data) {
  # walk_data <- walk_data %>% 
  #   group_by(sample) %>%
  #   summarise(wl = n() - 1)
  mean(walk_data$aw_length)
}

cor0 <- function(a, b) {
  c <- cor(a, b)
  if (is.na(c)) 0 else c
}

computeFDCs <- function(metaopt_problem, instance, ...) {
  aw_fla <- computeAllDistances(metaopt_problem, instance, ...) %>%
    select(-stopping_criterium, -mh) %>%
    spread(Sampling.Strat, adaptive_walk) %>%
    group_by(problem, budget, dist, corr, type, objective, no_jobs, no_machines, inst_n, instance) %>%
    nest() %>%
    ungroup() %>%
    mutate(
      adaptive_walks = map(data, unnest),
      fdc_adj = map_dbl(adaptive_walks, ~ cor0(.x$fitness, .x$adj)),
      fdc_prec = map_dbl(adaptive_walks, ~ cor0(.x$fitness, .x$prec)),
      fdc_abs_pos = map_dbl(adaptive_walks, ~ cor0(.x$fitness, .x$abs_pos)),
      fdc_dev = map_dbl(adaptive_walks, ~ cor0(.x$fitness, .x$dev)),
      fdc_shift = map_dbl(adaptive_walks, ~ cor0(.x$fitness, .x$shift)),
      fdc_swap = map_dbl(adaptive_walks, ~ cor0(.x$fitness, .x$swap)),
      mean_walk_length = map_dbl(adaptive_walks, meanWalkLength)
    )
  aw_fla
}

problem_space <- all_problems_df() %>%
  filter(budget == 'low', no_jobs == 30)

# plan(multisession)

# computeFLA(
#   problem_space,
#   computeFDCs,
#   c("fdc_adj"),
#   parallel = F,
#   use_cache = T
# )

# 
# computeFLA(
#   problem_space,
#   instanceFeaturesFLA,
#   c("no_jobs", "no_machines", "ratio", "pt_sd", "mean_sd_per_machine",
#     "mean_sd_per_job", "mean_skew_per_machine", "mean_skew_per_job",
#     "mean_kurt_per_machine", "mean_kurt_per_job"),
#   parallel = F,
#   use_cache = T
# )
# 
# set.seed(345346454)
# computeFLA(
#   problem_space,
#   computeSolutionStatisticsFLA,
#   c("up", "down", "side", "slmin", "lmin", "iplat", "ledge", "slope",
#                                   "lmax", "slmax"),
#   parallel = F,
#   use_cache = T
# )
# 
# # set.seed(87987987)
# computeFLA(
#   problem_space,
#   computeRandomWalkFLA,
#   c("rw_autocorr_1", "entropy", "partial_inf", "inf_stability", "density_basin"),
#   parallel = F,
#   use_cache = T
# )


# initFactories("data")
# 
# pp <- c(dist = "erlang", corr = "random", no_jobs = "30", no_machines = "5",
#   problem = "flowshop", corv = "0", objective = "FLOWTIME", type = "NOIDLE",
#   stopping_criterion = "TIME", budget = "low", id = "98", instance = "erlang_random_30_5_01.txt"
# )
# sp <- c(
#   AdaptiveWalk.Init = "neh", 
#   AdaptiveWalk.Init.NEH.Ratio = "1",
#   Sampling.Strat = "IG"
# )
# adaptiveWalk(pp, sp, 987)

