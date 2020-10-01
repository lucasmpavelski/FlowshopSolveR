
#' Get instance data from instance file name.
#'
#' @param instance_name instance file name, as stored in data/instances/flowshop folder.
#'
#' @return Data frame containing instance data with distribution, correlation, number of jobs and machines.
#' @export
#'
#' @examples
instance_data_from_filename <- function(instance_name) {
  if (str_starts(instance_name, 'VRF')) {
    name_split <- instance_name %>%
      str_replace_all('VRF|_Gap.txt', '') %>%
      str_split('[_.]', simplify = T)
    
    return(tibble(
      dist = 'vrf',
      corr = 'hard',
      no_jobs = as.integer(name_split[,1]),
      no_machines = as.integer(name_split[,2]),
      inst_n = as.integer(name_split[,3]),
      instance = instance_name,
      problem = ifelse(no_jobs <= 60, 'vrf-small')
    ))
  }
  if (str_starts(instance_name, 'tai')) {
    name_split <- instance_name %>%
      str_replace_all('tai|.txt', '') %>%
      str_split('[_.]', simplify = T)
    return(tibble(
      dist = 'taillard',
      corr = 'hard',
      no_jobs = as.integer(name_split[,1]),
      no_machines = as.integer(name_split[,2]),
      inst_n = as.integer(name_split[,3]),
      instance = instance_name
    ))
  }
  name_split <- str_split(instance_name, '[_.]', simplify = T)
  tibble(
    dist = name_split[,1],
    corr = name_split[,2],
    no_jobs = as.integer(name_split[,3]),
    no_machines = as.integer(name_split[,4]),
    inst_n = as.integer(name_split[,5]),
    instance = instance_name
  )
}

vrf_small_instances_df <- function() {
  crossing(
    problem = 'vrf-small',
    dist = 'vrf',
    corr = 'hard',
    no_jobs = c(10, 20, 30, 40, 50, 60),
    no_machines = c(5, 10, 15, 20),
    inst_n = 1:10,
  ) %>%
    mutate(instance = sprintf('VRF%d_%d_%d_Gap.txt', no_jobs, no_machines, inst_n))
}

vrf_large_instances_df <- function() {
  crossing(
    problem = 'vrf-large',
    dist = 'vrf',
    corr = 'hard',
    no_jobs = c(100, 200, 300, 400, 500, 600, 700, 800),
    no_machines = c(20, 40, 60),
    inst_n = 1:10
  ) %>%
    mutate(instance = sprintf('VRF%d_%d_%d_Gap.txt', no_jobs, no_machines, inst_n))
}

#' Get info from all instances in data/instances/flowshop folder.
#'
#' @return Data frame with all instances data.
#' @export
#'
#' @examples
all_instances <- function(instances_folder) {
  list.files(instances_folder) %>%
    map_df(instance_data_from_filename)
}