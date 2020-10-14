# ex FSP,PERM,FLOWTIME,low,FIXEDTIME_15,taill-like,rand,50,10
aclib2_set_id <-
  function(problem,
           model,
           dist,
           corr,
           no_jobs,
           no_machines,
           ...) {
    paste(problem, model, dist, corr, no_jobs, no_machines, sep = ",")
  }

# ex FSP,PERM,FLOWTIME,low,FIXEDTIME_15,taill-like_rand_50_10_05.dat
aclib2_instance_id <- function(problem, model, instance, ...) {
  paste(problem, model, instance, sep = ",")
}

write_list_lines <- function(lines, file) {
  if (file.exists(file)) {
    warning("file exists")
  }
  lines %>%
    as.character() %>%
    writeLines(file)
}

write_aclib2_instances_group <- function(data, instances_dir, ...) {
  instance_group <- tibble(...)

  group_dir <- file.path(instances_dir, instance_group$aclib_set)
  dir.create(group_dir, F, T)

  data %>%
    filter(inst_n <= 8) %>%
    pull(aclib_instance) %>%
    write_list_lines(file.path(group_dir, "training.txt"))

  data %>%
    filter(inst_n > 8) %>%
    pull(aclib_instance) %>%
    write_list_lines(file.path(group_dir, "test.txt"))
}

write_aclib2_instances <- function(instances, instances_dir) {
  instances %>%
    pwalk(write_aclib2_instances_group, instances_dir = instances_dir)
}

generate_aclib2_instances <- function() {
    set.seed(75768161)

    all_problems <- train_problems_df()  %>%
      filter(budget == 'low')

    sampled_instances <- all_problems %>%
      sample_frac(0.2) %>%
      unnest(cols = c(instances)) %>%
      mutate(
        aclib_instance = pmap_chr(., aclib2_instance_id),
        aclib_set = pmap_chr(., aclib2_set_id)
      ) %>%
      group_nest(across(!c(inst_n, instance, aclib_instance))) %>%
      ungroup()

    sampled_instances
  }

aclib2_scenario_name <- function(mh, set) {
  paste(mh, set, sep = "_")
}

generate_aclib2_scenarios <- function(instances, mh) {
  crossing(
    set = unique(instances$aclib_set),
    mh = mh
  ) %>%
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
    "cutoff_time = 21474836",
    "tuner-timeout = 21474836",
    "wallclock_limit = 21474836"
  ) %>%
    writeLines(file.path(scenario_dir, "scenario.txt"))
}

write_aclib2_scenarios <- function(scenarios, scenarios_dir) {
  scenarios %>%
    pmap(write_aclib2_scenario, scenarios_dir = scenarios_dir)
}


write_aclib2_experiments <- function(scenarios, base_dir) {
  scenarios %>%
    pull(aclib_scenario) %>%
    writeLines(file.path(base_dir, "experiments.txt"))
}

build_aclib2_experiments <- function(base_dir, mhs) {
  instances_dir <- file.path(base_dir, "instances", "fsp", "sets")
  scenarios_dir <- file.path(base_dir, "scenarios", "fsp")
  instances <- generate_aclib2_instances()
  write_aclib2_instances(instances, instances_dir)
  scenarios <- generate_aclib2_scenarios(instances, mhs)
  write_aclib2_scenarios(scenarios, scenarios_dir)
  write_aclib2_experiments(scenarios, base_dir)
}
