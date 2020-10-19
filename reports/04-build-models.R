library(rpart)
library(rpart.plot)

as_metaopt_problem <- function(model, instance_features, instances, ...) {
  Problem(
    name = paste(model, instance_features, sep = ','),
    instances = as.list(instances$instance),
    data = list(...)
  )
}


problems_dt <- train_problems_df() %>%
  filter(budget == 'low')
problem_space <- ProblemSpace(problems = pmap(problems_dt, as_metaopt_problem))
algorithm <- get_algorithm('NEH')
default_neh <- default_configs('NEH')
algorithm_space <- AlgorithmSpace(algorithms = list(algorithm))

cache_folder <- here('runs', 'neh')
dir.create(cache_folder, showWarnings = F)
irace_trained <- build_performance_data(
  problem_space = problem_space,
  algorithm_space = algorithm_space,
  solve_function = fsp_solver_performance,
  irace_scenario = defaultScenario(list(
    deterministic = 1,
    maxExperiments = 5000,
    initConfigurations = default_neh
  )),
  cache_folder = cache_folder,
  parallel = 7
)

params <- algorithm@parameters$names

output_dt <- irace_trained %>% 
  select(problems, results) %>%
  mutate(results = map(results, ~.x[1,])) %>%
  unnest(results) %>%
  mutate(problems_dt = map(problems, ~as_tibble(.x@data))) %>%
  unnest(problems_dt) %>%
  select(id, params)
  
  
param <- params[9]

input_features_dt <- train_features()
input_features <- names(input_features_dt %>% select(-id))
  
train_dt <- output_dt %>%
  select(id, param) %>%
  inner_join(input_features_dt, by = 'id') %>%
  select(param, all_of(input_features))

formula <- as.formula(paste(param, '~', paste(input_features, collapse = '+')))

model <- rpart(formula, data = train_dt)

rpart.plot(model, box.palette="Blues")

