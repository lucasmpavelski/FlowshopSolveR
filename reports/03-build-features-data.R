library(tidyverse)
library(FlowshopSolveR)
library(here)
library(metaOpt)
library(schedulingInstancesGeneratoR)

summarize_instance_features <- function(inst_dt) {
  summ <- inst_dt %>%
    unnest(instance_calc_features) %>%
    select(-instance) %>%
    summarise(across(everything(), mean))
  print(summ$no_machines)
  summ
}

problem_space <- all_problems_df()

all_instance_features <- problem_space %>%
  unnest(instances) %>%
  select(problem, instance_features, instance) %>%
  distinct() %>%
  mutate(
    instance_path = here('data', 'instances', problem, instance),
    instance_calc_features = map(instance_path, function(.x) {
      if (str_detect(.x, '500,60')) {
        
      }
      as_tibble(instance_statistics(read_txt(.x)))
    })
  ) %>%
  select(-instance_path)

summ_instance_features <- all_instance_features %>%
  group_nest(problem, instance_features) %>%
  mutate(data = map(data, summarize_instance_features)) %>%
  rename(instance_calc_features = data)

problem_instance_features <- problem_space %>%
  left_join(summ_instance_features, by = c('problem', 'instance_features'))



# problem_instance_features %>%
#   mutate(
#     metaopt_problem = pmap(., as_metaopt_problem),
#     features = map2(metaopt_problem, instance_calc_features, append_problem_features)
#   )







