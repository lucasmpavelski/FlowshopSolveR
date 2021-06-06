library(FlowshopSolveR)
library(readr)
library(dplyr)

# build_aclib2_experiments(
#   base_dir = here('aclib2', 'fsp'),
#   instance_attrs = LARGE_INSTANCES_ATTRS,
#   model_attrs = MODELS_ATTRS,
#   mhs = c('NEH')
# )

results_dir <- here('aclib2', 'results')
validation_files <- list.files(
  results_dir, 
  recursive = T,
  full.names = F,
  pattern = 'validationPerformanceDebug'
)




path <- validation_data_path[1]

folders <- str_split(path, '/', simplify = T)
scenario <- folders[,1]
tunner <- folders[,2]
conf <- switch(folders[,4], 'validate-def-test' = 'default', 'validade-inc-test' = 'best')

scenario_dt <- str_split(scenario, '_', 2, simplify = T)
algo <- scenario_dt[,1]
problem <- scenario_dt[,2]

data <- read_csv(file.path(results_dir, path)) %>%
  filter(!str_starts(Instance, 'Overall')) %>%
  select(
    instance = Instance,
    fitness = OverallObjective
  )

tibble(
  scenario = scenario,
  tunner = tunner,
  conf = conf,
  algo = algo,
  problem = problem,
  data = list(data)
)


mh <- sp_mh[1,1]
sp_dirs <- str_split(sp_mh[1,2], '/', simplify = T)[1,]
