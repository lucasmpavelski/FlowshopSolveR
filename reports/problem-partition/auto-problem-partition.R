require("FlowshopSolveR")
require("FactoMineR")
require("factoextra")
require("wrapr")

load_fla_metrics <- function() {
  load("/home/lucasmp/dev/evolutionary_tunners/data/fla/rw_fla_all.Rdata")
  rw_fla <- rw_fla %>% select(-id, -dist_id, -stopping_criterium, -mh, -budget, -sample, -ensamble, -random_walk)
  
  load("/home/lucasmp/dev/evolutionary_tunners/data/fla/aw_fla_all.Rdata")
  aw_fla <- aw_fla %>% select(-budget)
  
  load("/home/lucasmp/dev/evolutionary_tunners/data/fla/instance_fla_all.Rdata")
  instance_fla
  
  load("/home/lucasmp/dev/evolutionary_tunners/data/fla/solution_statistics_fla_all.Rdata")
  solution_statistics_fla <- solution_statistics_fla %>% select(-id, -dist_id, -stopping_criterium, -best_bound, -budget,
                                                                -mh, -No.Samples, -seed)
  
  join_features <- c("problem", "dist", "corr", "type", "objective", "no_jobs", "no_machines", "inst_n", "instance")
  
  rw_fla %>%
    left_join(aw_fla, by = join_features) %>%
    left_join(solution_statistics_fla, by = join_features) %>%
    left_join(instance_fla, by = "instance") %>%
    select(-best_bound)
}

# select problems
problems <- all_problems_df() %>% 
  filter(
    problem == "meta-learning",
    type == "PERM",
    stopping_criterion == "TIME",
    budget == "high"
  ) %>%
  select(-stopping_criterion, -budget) %>%
  crossing(
    stopping_criterion = c("TIME", "EVALS"),
    budget = c("low", "med", "high")
  )

# compute FLAs
problems_data <- read_csv("data/models/problems.csv") %>%
  mutate(
    problem = "meta-learning",
    dist = case_when(
      dist == "binom" ~ "binomial",
      dist == "taill-like" ~ "uniform",
      dist == "exp" ~ "exponential"
    ),
    corr = case_when(
      corr == "jcorr" ~ "job-correlated",
      corr == "mcorr" ~ "machine-correlated",
      corr == "rand" ~ "random"
    )
  )
train_problems <- problems_data %>%
  filter(dataset_1_folds != 0)
test_problems <- problems_data %>%
  filter(dataset_1_folds == 0)

fla_data <- load_fla_metrics()

# train_dt <- train_problems %>%
#   left_join(fla_data, by = qc(
#     problem,
#     dist,
#     corr,
#     type,
#     objective,
#     no_jobs,
#     no_machines,
#     inst_n,
#     instance
#   ))

recomendation_data <- read_csv("data/models/dataset_1.csv") %>%
  select(-starts_with("rec_"))
train_dt <- recomendation_data %>%
  filter(folds != 0) %>%
  select(-folds)
test_dt <- recomendation_data %>%
  filter(folds == 0) %>%
  select(-folds)

# build PCA
set.seed(42)
pca_model <- PCA(
  train_dt,
  ncp = 3,
  graph = F
)


# formulate problem partition
## select main instances

no_evals <- nrow(train_problems) * 5000

instances_per_objective <- 200
eval_instances <- train_problems %>%
  bind_cols(as_tibble(pca_model$ind$cos2)) %>%
  mutate(
    meta_objective = case_when(
      rank(-Dim.1, ties.method = "first") <= instances_per_objective ~ 1,
      rank(-Dim.2, ties.method = "first") <= instances_per_objective ~ 2,
      rank(-Dim.3, ties.method = "first") <= instances_per_objective ~ 3
    )
  ) %>%
  filter(!is.na(meta_objective))

config <- list(
  strategy = "moead",
  # parameters
  algorithm = algorithm,
  # problems
  eval_problems = eval_instances,
  solve_function = fsp_solver_performance,
  aggregation_function = arpf_by_objective,
  eval_no_samples = 2,
  # moead parameters
  moead_variation = "irace",
  moead_decomp = list(name = "MSLD", H = c(3,2), tau=c(1,.5), .nobj = 3),
  moead_neighbors = list(name = "lambda", T = 2, delta.p = 1),
  moead_max_iter = 50,
  # irace variation
  irace_variation_problems = type_problems,
  irace_variation_no_evaluations = 100,
  irace_variation_no_samples = 4
)

# solve problem partition

# test on new instances