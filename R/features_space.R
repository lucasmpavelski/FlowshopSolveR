
instance_model_features_df <- function(problems) {
  instances_dts <- problems %>%
    unnest(instances) %>%
    mutate(instance_fn = here('data', 'instances', problem, instance)) %>%
    mutate(instance_data = map(instance_fn, read_txt))
  
  instance_stats <- instances_dts %>%
    mutate(instance_stats = map(instance_data, ~as_tibble(instance_statistics(.x)))) %>%
    unnest(instance_stats)
  
  instance_features <- c(
    "pt_sd",
    "mean_sd_per_machine",
    "mean_sd_per_job",
    "mean_skew_per_machine",
    "mean_skew_per_job",
    "mean_kurt_per_machine",
    "mean_kurt_per_job" 
  )
  
  instance_stats %>%
    group_by(id, no_jobs, no_machines, objective, type) %>%
    summarise(across(all_of(instance_features), mean))
} 


generate_train_test_features <- function() {
  features_folder <- here('data', 'features')
  dir.create(features_folder, showWarnings = F, recursive = T)
  train_problems_df() %>%
    instance_model_features_df() %>%
    write_csv(file.path(features_folder, 'train.csv'))
  test_problems_df() %>%
    instance_model_features_df() %>%
    write_csv(file.path(features_folder, 'test.csv'))
}

train_features <- function() {
  read_csv(here('data', 'features', 'train.csv'))
}

test_features <- function() {
  read_csv(here('data', 'features', 'test.csv'))
}