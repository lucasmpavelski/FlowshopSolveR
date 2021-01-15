filename_from_instance_data <-
  function(no_jobs,
           no_machines,
           dist,
           corr,
           inst_n,
           ...) {
    sprintf(
      "%s_%s_%d_%d_%02d.txt",
      dist,
      corr,
      no_jobs,
      no_machines,
      inst_n
    )
  }

generate_test_instance <-
  function(no_jobs, no_machines, dist, corr, corv, ...) {
    generate_fsp_instance(
      no_jobs = no_jobs,
      no_machines = no_machines,
      distribution_type = dist,
      correlation_type = corr,
      correlation = ifelse(corr == "random", 0, corv)
    )
  }

save_test_instance <- function(instance_data, path, ...) {
  write_txt(instance_data, path)
}

generate_test_instances <- function() {
  set.seed(6549871)
  dir.create(here("data", "instances", "flowshop"), F)
  generated_instances_df() %>%
    mutate(
      path = here("data", "instances", "flowshop", instance)
    ) %>%
    filter(!file.exists(path)) %>%
    mutate(instance_data = pmap(., generate_test_instance)) %>%
    pmap(save_test_instance)
}

update_instances_tars <- function() {
  userwd <- getwd()
  on.exit(setwd(userwd))
  setwd(here("data", "instances"))
  list.dirs(recursive = F, full.names = F) %>%
    walk(~ tar(
      paste0(.x, ".tar.gz"),
      .x,
      compression = "gzip",
      tar = "tar"
    ))
}



GENERATED_INSTANCES_ATTRS <- list(
  no_jobs = c(50, 100, 200),
  no_machines = c(20, 40),
  dist = c("uniform", "erlang", "exponential"),
  corr = c("random", "job-correlated", "machine-correlated"),
  corv = 0.90,
  no_samples = 10
)

generated_instances_df <- function() {
  pts <- bind_rows(
    tidyr::crossing(
      dist = c("uniform"),
      corr = c("random", "job-correlated", "machine-correlated")
    ),
    tidyr::crossing(
      dist = c("erlang", "exponential"),
      corr = c("random")
    )
  )
  sizes <- bind_rows(
    tidyr::crossing(
      no_jobs = c(10, 20, 30, 40, 50, 60),
      no_machines = c(5, 10, 15, 20),
    ),
    tidyr::crossing(
      no_jobs = c(100, 200, 300, 400, 500, 600, 700, 800),
      no_machines = c(20, 40, 60),
    )
  )
  tidyr::crossing(
    pts,
    sizes,
    problem = "flowshop",
    inst_n = 1:10
  ) %>%
    mutate(corv = if_else(corr == 'random', 0, 0.9)) %>%
    mutate(instance = pmap_chr(., filename_from_instance_data))
}
