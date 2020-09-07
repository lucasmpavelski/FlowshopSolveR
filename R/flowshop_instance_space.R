
#' Get instance data from instance file name.
#'
#' @param instance_name instance file name, as stored in data/instances/flowshop folder.
#'
#' @return Data frame containing instance data with distribution, correlation, number of jobs and machines.
#' @export
#'
#' @examples
instance_data_from_filename <- function(instance_name) {
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

#' Get info from all instances in data/instances/flowshop folder.
#'
#' @return Data frame with all instances data.
#' @export
#'
#' @examples
all_instances <- function() {
  list.files("data/instances/flowshop/") %>%
    map_df(instance_data_from_filename)
}