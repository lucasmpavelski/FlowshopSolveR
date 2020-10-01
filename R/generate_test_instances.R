filename_from_instance_data <-
  function(no_jobs,
           no_machines,
           dist,
           corr,
           inst_n,
           ...) {
    sprintf('%s_%s_%d_%d_%02d.txt',
            dist,
            corr,
            no_jobs,
            no_machines,
            inst_n)
  }

generate_test_instance <-
  function(no_jobs, no_machines, dist, corr, corv, ...) {
    generate_fsp_instance(
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
  write_txt(instance_data, path)
}

GENERATED_INSTANCES_ATTRS <- list(
  no_jobs = c(50, 100, 200),
  no_machines = c(20, 40),
  dist = c('uniform', 'erlang', 'exponential'),
  corr = c('random', 'job-correlated', 'machine-correlated'),
  corv = 0.90,
  no_samples = 10
)

generate_test_instances <- function(attrs = GENERATED_INSTANCES_ATTRS) {
  set.seed(6549871)
  dir.create(here('data', 'instances', 'flowshop'), F)
  instances_attrs_df(attrs) %>%
    mutate(
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

instances_attrs_df <- function(attrs = GENERATED_INSTANCES_ATTRS) {
  crossing(
    no_jobs = attrs$no_jobs,
    no_machines = attrs$no_machines,
    dist = attrs$dist,
    corr = attrs$corr,
    corv = attrs$corv,
    inst_n = 1:attrs$no_samples
  ) %>%
    filter(no_jobs != 50 | no_machines == 20) %>%
    mutate(instance = pmap_chr(., filename_from_instance_data))
}
