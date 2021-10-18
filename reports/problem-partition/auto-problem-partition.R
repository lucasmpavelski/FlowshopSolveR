require("FlowshopSolveR")
require("FactoMineR")
require("factoextra")

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
    select(-problem, -dist, -corr, -inst_n, -instance, -best_bound)
}

# select problems
problems <- all_problems_df() %>% 
  filter(
    problem == "meta-learning", 
    budget == "high",
    type = "PERM"
  )

# compute FLAs
fla_metrics <- load_fla_metrics() %>% select(-type, -objective)

# build PCA
pca_model <- PCA(
  fla_metrics,
  ncp = 3,
  graph = F
)

# formulate problem partition

# solve problem partition

# test on new instances