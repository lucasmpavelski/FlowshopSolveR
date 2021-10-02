require("FlowshopSolveR")

read_features <- function(...) {
  problem = list(...)
  browser()
}

# select problems
problem_data <- all_problems_df() %>%
  filter(
    no_jobs %in% c(30),
    no_machines %in% c(20),
    budget %in% c('high'),
    stopping_criterion %in% c('TIME')
  )

# compute FLAs
problem_data <- problem_data %>%
  mutate(features = map2(
    
    load_problem_features,
    features_folder = here("data", "features")
  ))


# build PCA

# formulate problem partition

# solve problem partition

# test on new instances