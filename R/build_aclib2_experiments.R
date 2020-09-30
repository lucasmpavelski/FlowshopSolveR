# ex FSP,PERM,FLOWTIME,low,FIXEDTIME_15,taill-like,rand,50,10
aclib2_set_id <-
  function(model,
           dist,
           corr,
           no_jobs,
           no_machines,
           ...) {
    paste('FSP', model, dist, corr, no_jobs, no_machines, sep = ',')
  }

# ex FSP,PERM,FLOWTIME,low,FIXEDTIME_15,taill-like_rand_50_10_05.dat
aclib2_instance_id <- function(model, instance, ...) {
  paste('FSP', model, instance, sep = ',')
}

write_list_lines <- function(lines, file) {
  if (file.exists(file))
    warning("file exists")
  lines %>%
    as.character() %>%
    writeLines(file)
}

write_aclib2_instances_group <- function(data, instances_dir, ...) {
  instance_group <- tibble(...)
  
  group_dir <- file.path(instances_dir, instance_group$aclib_set)
  dir.create(group_dir, F, T)
  
  data %>%
    filter(inst_n <= 25) %>%
    pull(aclib_instance) %>%
    write_list_lines(file.path(group_dir, 'training.txt'))
  
  data %>%
    filter(inst_n >= 25) %>%
    pull(aclib_instance) %>%
    write_list_lines(file.path(group_dir, 'test.txt'))
}

write_aclib2_instances <- function(instances, instances_dir) {
  instances %>%
    pwalk(write_aclib2_instances_group, instances_dir = instances_dir)
}

generate_aclib2_instances <-
  function(inst_atttrs = TEST_INSTANCES_ATTRS,
           model_attrs = MODELS_ATTRS) {
    set.seed(75768161)
    
    all_problems <-
      crossing(instances_attrs_df(TEST_INSTANCES_ATTRS),
               models_attrs_df(MODELS_ATTRS))
    
    sampled_instances <- all_problems %>%
      select(-inst_n, -instance) %>%
      unique() %>%
      sample_frac(0.1) %>%
      inner_join(all_problems) %>%
      mutate(
        aclib_instance = pmap_chr(., aclib2_instance_id),
        aclib_set = pmap_chr(., aclib2_set_id)
      ) %>%
      group_nest(across(!c(inst_n, instance, aclib_instance))) %>%
      ungroup()
    
    sampled_instances
  }

aclib2_scenario_name <- function(mh, set) {
  paste(mh, set, sep = '_')
}

generate_aclib2_scenarios <- function(instances, mh) {
  crossing(set = unique(instances$aclib_set),
           mh = mh) %>%
    mutate(aclib_scenario = pmap_chr(., aclib2_scenario_name))
}

write_aclib2_scenario <- function(set, mh, aclib_scenario, scenarios_dir, ...) {
  scenario_dir <-
    file.path(scenarios_dir, aclib_scenario)
  dir.create(scenario_dir, F, T)
  
  c(
    sprintf("algo = python3 ./target_algorithms/fsp/%s/wrapper.py", mh),
    sprintf("paramfile = ./target_algorithms/fsp/%s/params.pcs", mh),
    sprintf("instance_file = instances/fsp/sets/%s/training.txt", set),
    sprintf("test_instance_file = instances/fsp/sets/%s/test.txt", set),
    "execdir = .",
    "deterministic = 1",
    "run_obj = quality",
    "runcount_limit = 5000",
    "cutoff_time = 1e30",
    "tuner-timeout = 1e30",
    "wallclock_limit = 1e30"
  ) %>%
    writeLines(file.path(scenario_dir, 'scenario.txt'))
}

write_aclib2_scenarios <- function(scenarios, scenarios_dir) {
  scenarios %>%
    pmap(write_aclib2_scenario, scenarios_dir = scenarios_dir)
}


write_aclib2_experiments <- function(scenarios, base_dir) {
  scenarios %>%
    pull(aclib_scenario) %>%
    writeLines(file.path(base_dir, 'experiments.txt'))
}

build_aclib2_experiments <- function(base_dir, instance_attrs, model_attrs, mhs) {
  instances_dir <- file.path(base_dir, 'instances', 'fsp', 'sets')
  scenarios_dir <- file.path(base_dir, 'scenarios', 'fsp')
  instances <- generate_aclib2_instances(
    inst_atttrs = instance_attrs,
    model_attrs = model_attrs
  )
  write_aclib2_instances(instances, instances_dir)
  scenarios <- generate_aclib2_scenarios(instances, mhs)
  write_aclib2_scenarios(scenarios, scenarios_dir)
  write_aclib2_experiments(scenarios, base_dir)
}