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


problem_space <- all_problems_df()

plan(multisession)

use_cache <- TRUE

solution_statistics_fla <- problem_space %>%
  filter(budget == 'low', no_jobs > 300, no_jobs <= 500) %>%
  mutate(
    metaopt_problem = pmap(., as_metaopt_problem),
    name = map_chr(metaopt_problem, ~.x@name)
  ) %>% 
  unnest(instances) %>%
  mutate(
    features = map2(
      metaopt_problem,
      instance, 
      load_problem_features, 
      features_folder = FEATURES_FOLDER
    )
  ) %>%
  filter(use_cache & !map_lgl(features, ~"up" %in% names(.x))) %>%
  mutate(
    solution_statistics_fla = pmap(., function(metaopt_problem, instance, ...) {
      future({
        prob_dt <- df_to_character(metaopt_problem@data)
        prob_dt$instance <- instance
        stats <- solutionStatisticsFLA(
          DATA_FOLDER,
          rproblem = df_to_character(prob_dt),
          noSamples = case_when(
            prob_dt$no_jobs <= 100 ~ 100,
            prob_dt$no_jobs <= 300 ~ 50,
            prob_dt$no_jobs <= 500 ~ 30
          ),
          seed = as.integer(runif(1, 0, 2147483647))
        )
        append_problem_features(
          as_tibble(stats),
          metaopt_problem, 
          instance, 
          features_folder = FEATURES_FOLDER
        )
      }, seed = TRUE)
    })
  ) %>%
    pull(solution_statistics_fla) %>%
    values()
#   rowwise() %>%
#   do({
#     dt <- .
#     print(paste(.$instance, .$objective))
#     print(.$seed)
#     solution_statistics <- solutionStatisticsFLA(
#       rproblem = c(
#         "problem" = .$problem,
#         "instance" = .$instance,
#         "type" = .$type,
#         "objective" = .$objective,
#         "budget" = .$budget,
#         "stopping_criterium" = .$stopping_criterium
#       ),
#       rsampling = c(
#         No.Samples = as.character(.$No.Samples)
#       ),
#       seed = .$seed
#     )
#     dt$fla_measure <- names(solution_statistics)
#     dt$fla_value <- solution_statistics
#     as_tibble(dt)
#   }) %>% 
#   unnest() %>%
#   spread(fla_measure, fla_value) %>%
#   mutate(
#     total_edges = down + up + side,
#     down = down / total_edges,
#     up = up / total_edges,
#     side = side / total_edges,
#     iplat = iplat / No.Samples,
#     ledge = ledge / No.Samples,
#     lmax  = lmax  / No.Samples,
#     lmin  = lmin  / No.Samples,
#     slmax = slmax / No.Samples,
#     slmin = slmin / No.Samples,
#     slope = slope / No.Samples
#   ) %>%
#   select(-total_edges)
# 
# 
# 




