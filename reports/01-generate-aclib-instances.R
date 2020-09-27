library(FlowshopSolveR)

# ex FSP,PERM,FLOWTIME,low,FIXEDTIME_15,taill-like,rand,50,10
aclib2_set_id <- function(model, dist, corr, no_jobs, no_machines, ...) {
  paste('FSP', model, dist, corr, no_jobs, no_machines, sep = ',')
}

# ex FSP,PERM,FLOWTIME,low,FIXEDTIME_15,taill-like_rand_50_10_05.dat
aclib2_instance_id <- function(model, instance, ...) {
  paste('FSP', model, instance, sep = ',')
}

write_list_lines <- function(lines, file) {
  lines %>%
    as.character() %>%
    writeLines(file)
}

write_aclib2_instances_group <- function(data, instances_dir, ...) {
  instance_group <- tibble(...)
  
  set_id <- do.call(aclib2_set_id, instance_group[1,])
  print(set_id)
  group_dir <- file.path(instances_dir, set_id)
  dir.create(group_dir, F, T)
  
  group_dt <- data %>%
    mutate(id = pmap(., aclib2_instance_id, model = instance_group$model))
  
  group_dt %>%
    filter(inst_n <= 25) %>%
    pull(id) %>%
    write_list_lines(file.path(group_dir, 'training.txt'))
  
  group_dt %>%
    filter(inst_n >= 25) %>%
    pull(id) %>%
    write_list_lines(file.path(group_dir, 'test.txt'))
}

set.seed(75768161)

base_dir <- here('aclib2', 'fsp')
instances_dir <- file.path(base_dir, 'instances', 'fsp', 'sets')

all_problems <-
  crossing(
    instances_attrs_df(TEST_INSTANCES_ATTRS),
    models_attrs_df(MODELS_ATTRS)
  )

sampled_instances <- all_problems %>%
  select(-inst_n, -instance) %>%
  unique() %>%
  sample_frac(0.1) %>%
  inner_join(all_problems)

sampled_instances %>%
  group_by(across(!c(inst_n, instance))) %>%
  nest() %>%
  pwalk(write_aclib2_instances_group, instances_dir = instances_dir)



