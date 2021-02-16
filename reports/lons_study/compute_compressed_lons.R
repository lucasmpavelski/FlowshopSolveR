library(FlowshopSolveR)
library(here)
library(tidygraph)
library(furrr)
library(optparse)

lon_configs <- read_rds(here('reports/lons_study/lon_configs.rds'))

option_list <- list( 
  make_option(c("-c", "--config_id"), action="dist", default=2, type='integer',
              help="Print extra output [default]")
)

opt <- parse_args(OptionParser(option_list=option_list))
CONFIG_ID <- opt$config_id

lons_folder <- here("data", "lons_cache")

compute_compressed_lon <- function(full_lon) {
  neutral_edges <- full_lon$edges %>% 
    filter(full_lon$nodes$fitness[from] == full_lon$nodes$fitness[to])
  
  g <- tbl_graph(
    edges = neutral_edges,
    nodes = as_tibble(full_lon$nodes)
  )
  
  neutral_components <- g %>%
    activate(nodes) %>%
    mutate(
      component = group_components()
    ) %>%
    as_tibble()
  
  rm(g)
  
  isolated_nodes <- neutral_components %>% 
    mutate(node = row_number()) %>%
    group_by(component, fitness) %>% 
    filter(n() == 1) %>%
    nest() %>%
    mutate(nodes = map(data, ~.x$node)) %>%
    select(component, nodes, fitness)
  
  neutral_groups <- full_lon$edges %>%
    mutate(
      component_from = neutral_components$component[from],
      component_to = neutral_components$component[to],
      fitness = full_lon$nodes$fitness[from]
    ) %>%
    filter(component_from == component_to) %>%
    group_by(component_from, fitness) %>%
    nest() %>%
    mutate(
      nodes = map(data, ~unique(c(.x$from, .x$to)))
    ) %>%
    select(
      component = component_from, nodes, fitness
    ) %>% 
    bind_rows(isolated_nodes) %>%
    unnest(nodes)
  
  clon <- list()
  clon$edges <- full_lon$edges %>%
    inner_join(neutral_groups, by = c("from" = "nodes")) %>%
    rename(component_from = component, fitness = NULL) %>%
    inner_join(neutral_groups, by = c("to" = "nodes")) %>%
    rename(component_to = component, fitness = NULL) %>%
    group_by(component_from, component_to) %>%
    summarise(weight = sum(weight), .groups = 'drop') %>%
    group_by(component_from) %>%
    mutate(weight = weight / sum(weight)) %>%
    rename(
      from = component_from,
      to = component_to
    )
  
  clon$nodes <- neutral_groups %>%
    group_by(component, fitness) %>%
    nest() %>%
    arrange(component)
  
  clon
}

compute_metrics <- function(lons_folder, problem_config, lon_config, sample_type) {
  save_path <- sprintf("%s/%s_%s_clon.rds", lons_folder, lon_config, problem_config)
  lon_path <- ifelse(sample_type == 'snowball',
                     sprintf("%s/%s_%s_1.rds", lons_folder, lon_config, problem_config),
                     sprintf("%s/%s_%s_wfull.rds", lons_folder, lon_config, problem_config)
  )
  # !file.exists(save_path) && 
  if (file.exists(lon_path)) {
    print(lon_path)
    lon <- read_rds(lon_path)
    write_rds(compute_compressed_lon(lon), save_path, compress = "xz")
    rm(lon)
  }
  save_path
}

problems <- all_problems_df() %>%
  filter(
    problem %in% c('vrf-small', 'vrf-large'),
    budget == 'low',
    type == 'PERM',
    objective == 'MAKESPAN',
    no_jobs <= 300
  ) %>%
  unnest(cols = instances) %>%
  mutate(budget = 'med', stopping_criterion = 'EVALS') %>%
  mutate(problem_id = pmap(., function(problem, type, objective, budget, instance, stopping_criterion, ...) {
    paste(c(problem, type, objective, budget, instance, stopping_criterion), collapse = ";", sep = "_")
  }))


library(progressr)
handlers(global = T)
handlers("progress")

plan(multisession(workers = 4))

compute_all_compressed_lons <- function() {
  p <- progressor(along = seq_along(problems$problem_id))
  y <- future_map(seq_along(problems$problem_id), function(i) {
    prob <- problems$problem_id[[i]]
    lon <- lon_configs[[CONFIG_ID]]$id
    sample_type <- lon_configs[[CONFIG_ID]]$sample_type
    compute_metrics(lons_folder, prob, lon, sample_type)
    p(sprintf("x=%g", i))
  })
}

compute_all_compressed_lons()


lon <- list()
lon$edges <- tribble(
  ~from, ~to, ~weight,
  1, 2, 1,
  2, 3, 1,
  3, 4, 1,
  4, 5, 1,
  1, 5, 1,
  1, 6, .5
)

lon$nodes <- list(
  solutions = c('a', 'b', 'c', 'd', 'e', 'f'),
  fitness = c(1, 1, 2, 2, 1, 2)
)

clon <- compute_compressed_lon(lon)
print(clon$edges)
print(clon$nodes %>%
        unnest(data))
