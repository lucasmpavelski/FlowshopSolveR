

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
  } else if (name == 'IG') {
    tibble(   
      IG.Init                            = "neh",
      IG.Init.NEH.Ratio                  = "0",
      IG.Init.NEH.Priority               = "sum_pij",
      IG.Init.NEH.PriorityOrder          = "decr",
      IG.Init.NEH.PriorityWeighted       = "no",
      IG.Init.NEH.Insertion              = "random_best",
      IG.Comp.Strat                      = "strict",
      IG.Neighborhood.Size               = "1.0",
      IG.Neighborhood.Strat              = "ordered",
      IG.LS.Single.Step                  = "0",
      IG.Accept                          = "temperature",
      IG.Accept.Better.Comparison        = "strict",
      IG.Accept.Temperature              = "0.5",
      IG.Perturb.Insertion               = "random_best",
      IG.Perturb                         = "rs",
      IG.Perturb.DestructionSizeStrategy = "fixed",
      IG.Perturb.DestructionSize         = "4",
      IG.DestructionStrategy             = "random",
      IG.Local.Search                    = "best_insertion",
      IG.NumberOfSwapsStrategy           = "default"
    )
  } else {
    tibble()
  }
}