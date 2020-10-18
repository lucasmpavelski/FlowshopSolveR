

get_algorithm <- function(name) {
  Algorithm(
    name = name,
    parameters = readParameters(here('data', 'specs', paste0(name, '.txt')))
  )
}

# TODO: read csv data
default_configs <- function(name) {
  if (name == 'NEH') {
    tibble(
      NEH.Init='neh',
      NEH.Init.NEH.Ratio='0',
      NEH.Init.NEH.First.Priority='sum_pij',
      NEH.Init.NEH.First.PriorityWeighted='no',
      NEH.Init.NEH.First.PriorityOrder='decr',
      NEH.Init.NEH.Priority='sum_pij',
      NEH.Init.NEH.PriorityOrder='decr',
      NEH.Init.NEH.PriorityWeighted='no',
      NEH.Init.NEH.Insertion='first_best'
    )
  } else {
    tibble()
  }
}