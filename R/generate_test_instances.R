filename_from_instance_data <-
  function(no_jobs,
           no_machines,
           dist,
           corr,
           inst_n,
           ...) {
    sprintf('%s_%s_%d_%d_%02d.dat',
            dist,
            corr,
            no_jobs,
            no_machines,
            inst_n)
  }

generate_test_instance <-
  function(no_jobs, no_machines, dist, corr, corv, ...) {
    generateFSPInstance(
      no_jobs = no_jobs,
      no_machines = no_machines,
      distribution_type = switch(dist,
                                 'taill-like' = 'uniform',
                                 'exp' = 'exponential',
                                 dist),
      correlation_type = switch(
        corr,
        'rand' = 'random',
        'jcorr' = 'job-correlated',
        'mcorr' = 'machine-correlated'
      ),
      correlation = ifelse(corr == 'rand', 0, corv)
    )
  }

save_test_instance <- function(instance_data, path, ...) {
  appendFile <- function(...)
    write(..., file = path, append = T)
  appendFile(nrow(instance_data))
  appendFile(ncol(instance_data))
  appendFile(instance_data@seed)
  for (job in 1:nrow(instance_data)) {
    appendFile(job - 1)
    appendFile(0)
    appendFile(instance_data[job, ], ncolumns = ncol(instance_data))
  }
  path
}

generate_test_instances <- function(no_jobs = c(20, 50, 100, 200),
                                    no_machines = c(10, 20, 40),
                                    dist = c('taill-like', 'erlang', 'exp'),
                                    corr = c('rand', 'jcorr', 'mcorr'),
                                    corv = 0.95,
                                    no_samples = 30) {
  set.seed(6549871)
  dir.create(here('data', 'instances', 'flowshop'), F)
  crossing(
    no_jobs = no_jobs,
    no_machines = no_machines,
    dist = dist,
    corr = corr,
    corv = corv,
    inst_n = 1:no_samples
  ) %>%
    mutate(
      instance = pmap_chr(., filename_from_instance_data),
      path = here('data', 'instances', 'flowshop', instance)
    ) %>%
    filter(!file.exists(path)) %>%
    mutate(instance_data = pmap(., generate_test_instance)) %>%
    pmap(save_test_instance)
}

update_instances_tars <- function() {
  userwd <- getwd()
  on.exit(setwd(userwd))
  setwd(here('data', 'instances'))
  list.dirs(recursive = F, full.names = F) %>%
    walk( ~ tar(
      paste0(.x, '.tar.gz'),
      .x,
      compression = 'gzip',
      tar = 'tar'
    ))
}
