library(FlowshopSolveR)
library(here)
library(tidygraph)
library(furrr)
library(optparse)

lon_configs <- read_rds(here('reports/lons_study/lon_configs.rds'))

option_list <- list( 
  make_option(c("-c", "--config_id"), action="dist", default=1, type='integer',
              help="Print extra output [default]")
)

opt <- parse_args(OptionParser(option_list=option_list))
CONFIG_ID <- opt$config_id

lons_folder <- here("data", "lons_cache")

plotLON <- function(lon) {
  library(ggraph)
  set.seed(2017)
  tidygraph::tbl_graph(
    edges = lon$edges, 
    nodes = as_tibble(lon$nodes) %>%
      mutate(label = row_number()) #map_chr(solutions, ~paste0(.x, collapse = ",")))
  ) %>%
    ggraph(layout = "fr") +
    geom_edge_link(aes(edge_alpha = weight)) +
    geom_node_point(aes(color = fitness)) +
    geom_node_label(aes(label = label)) +
    theme_graph()
}

row2CharVector <- function(row) {
  rnames <- colnames(row)
  res <- as.character(row)
  names(res) <- rnames
  res
}

no_nodes <- function(lon) {
  length(lon$nodes$fitness)
}

no_edges <- function(lon) {
  nrow(lon$edges)
}

average_fitness <- function(lon) {
  mean(lon$nodes$fitness)
}

average_weight_of_self_loops <- function(lon) {
  mean(lon$edges %>% filter(from == to) %>% pull(weight))
}

fitness_fitness_correlation <- function(lon) {
  neighbors_fit <- lon$edges %>%
    mutate(fitFrom = lon$nodes$fitness[from],
           fitTo = lon$nodes$fitness[to])  %>%
    filter(from != to) %>%
    group_by(from, fitFrom) %>%
    summarise(
      w_fit_avg = mean(weight * fitTo) / sum(weight)
    )
  result <- cor(neighbors_fit$fitFrom, neighbors_fit$w_fit_avg)
  rm(neighbors_fit)
  result
}

avarege_out_degree <- function(lon) {
  dt <- lon$edges %>%
    filter(from != to) %>%
    group_by(from) %>%
    count()
  result <- mean(dt$n)
  rm(dt)
  result
}

average_disparity <- function(lon) {
  dt <- lon$edges %>% 
    filter(from != to) %>%
    group_by(from) %>%
    mutate(s = sum(weight)) %>%
    summarise(y2 = sum((weight / s)^2))
  result <- mean(dt$y2)
  rm(dt)
  result
}

average_weighted_clustering_coefficient <- function(lon) {
  edges <- lon$edges
  eij <- edges %>% rename(i = from, j = to, wij = weight)
  ejh <- edges %>% rename(j = from, h = to, wjh = weight)
  ehi <- edges %>% rename(h = from, i = to, whi = weight)
  eih <- edges %>% rename(i = from, h = to, wih = weight)
  node_stats <- edges %>%
    filter(from != to) %>%
    group_by(from) %>%
    summarise(
      si = sum(weight),
      ki = n()
    ) %>%
    rename(i = from)
  triplets <- eij %>%
    filter(i != j) %>%
    inner_join(ejh, by = "j") %>%
    filter(j != h, h != i) %>%
    inner_join(ehi, by = c("i", "h")) %>%
    inner_join(eih, by = c("i", "h"))
  cws <- triplets %>%
    group_by(i) %>%
    summarise(cwi = sum(wij + wih) / 2) %>%
    right_join(node_stats, by = "i") %>%
    # TODO: discuss oriented and assimetric calculation
    mutate(cwi = if_else(ki == 1, 0, cwi / (si * (ki  - 1))))
  result <- mean(cws$cwi, na.rm = T)
  rm(edges, eij, ejh, ehi, eih, node_stats, triplets, cws)
  result
}

fdcMetrics <- function(lon) {
  fdc_samples <- as_tibble(lon$samples) %>% unique()
  fdc_samples$local_optima <- lon$nodes$solutions[fdc_samples$local_optima_idx]
  # fdc_samples <- fdc_samples %>% filter(map2_lgl(solutions, local_optima, ~ !all(.x == .y)))
  fdc_samples <- fdc_samples %>% mutate(
    adjacencyDistance = map2_dbl(solutions, local_optima, ~adjacencyDistance(.x, .y)),
    precedenceDistance = map2_dbl(solutions, local_optima, ~precedenceDistance(.x, .y)),
    absolutePositionDistance = map2_dbl(solutions, local_optima, ~absolutePositionDistance(.x, .y)),
    deviationDistance = map2_dbl(solutions, local_optima, ~deviationDistance(.x, .y)),
    aproximatedSwapDistance = map2_dbl(solutions, local_optima, ~aproximatedSwapDistance(.x, .y)),
    shiftDistance = map2_dbl(solutions, local_optima, ~shiftDistance(.x, .y))
  )
  mean_no_steps <- mean(fdc_samples$no_steps)
  fdcs <- fdc_samples %>%
    mutate(solution = row_number()) %>%
    select(solution, fitness, ends_with("Distance")) %>%
    group_by(solution, fitness) %>%
    gather("distance", "value", ends_with("Distance")) %>%
    group_by(distance) %>%
    summarise(correlation = if_else(all(value == 0), 0, cor(fitness, value)))
  fdcs <- structure(as.numeric(fdcs$correlation), 
                    names = paste0("fdc_", fdcs$distance))
  fdcs['mean_no_steps'] <- mean_no_steps
  fdcs
}

computeDistances <- function(sols) {
  map(ALL_DISTANCES_FUNCTIONS, ~.x(sols[[1]], sols[[2]]))
}

sampleCombinations <- function(len) {
  if (len == 1) {
    tibble(V1 = 1, V2 = 1)
  } else if (len * (len - 1) < 2000) {
    combn(1:len, 2) %>% t() %>% as_tibble()
  } else {
    res <- NULL
    repeat {
      pair <- sample.int(len, 2, replace = F)
      res <- union(res, list(pair))
      if (length(res) == 1000) {
        return(as_tibble(do.call(rbind, res)))
      }
    }
  }
}

meanDistanceBetweenLocalOptimas <- function(lon, distance_fns = ALL_DISTANCES_FUNCTIONS) {
  len <- length(lon$nodes$solutions)
  sampleCombinations(len) %>%
    mutate(
      sol_i = map(V1, ~lon$nodes$solutions[[.x]]),
      sol_j = map(V2, ~lon$nodes$solutions[[.x]]),
      dists = map2(sol_i, sol_j, function(i, j) {
        map_dfc(distance_fns, function(x) {
          x(i, j)
        })
      })
    ) %>% 
    select(dists) %>%
    unnest(cols = c(dists)) %>% 
    rename_all(~paste0("md_", .)) %>%
    summarise_all(mean) %>%
    as.list()
}

lonMetrics <- function(lon) {
  list(
    no_nodes = no_nodes(lon),
    no_edges = no_edges(lon),
    weight_mean = mean(lon$edges$weight),
    weight_std = sd(lon$edges$weight),
    average_fitness = average_fitness(lon),
    average_weight_of_self_loops = average_weight_of_self_loops(lon),
    fitness_fitness_correlation = fitness_fitness_correlation(lon),
    avarege_out_degree = avarege_out_degree(lon),
    average_disparity = average_disparity(lon),
    # clustering_w = tnet::clustering_w(tnet::symmetrise_w(lon$edges)),
    average_weighted_clustering_coefficient = average_weighted_clustering_coefficient(lon)
  )
}


neutralMetrics <- function(lon) {
  neutral_graph <- tidygraph::tbl_graph(
    edges = lon$edges,
    nodes = as_tibble(lon$nodes)
  ) %>%
    activate(edges) %>%
    filter(from != to, .N()$fitness[from] == .N()$fitness[to])
  
  edges_stats <- neutral_graph %>%
    activate(edges) %>%
    as_tibble() %>%
    summarise(
      neutral_no_edges = n(),
      neutral_rel_no_edges = n() / nrow(lon$edges),
      neutral_mean_weight = mean(weight)
    ) %>%
    as.list()
  
  node_stats <- neutral_graph %>%
    activate(nodes) %>%
    mutate(
      group = tidygraph::group_components(),
      out_degree = local_size(mode = 'out') - 1     
    ) %>%
    as_tibble() %>%
    summarise(
      neutral_mean_out_degree = mean(out_degree)
    )
  
  groups_stats <- neutral_graph %>%
    activate(nodes) %>%
    mutate(
      group = tidygraph::group_components(),
      out_degree = local_size(mode = 'out')     
    ) %>%
    as_tibble() %>%
    count(group) %>%
    summarise(
      neutral_mean_size = mean(n),
      neutral_no_groups = n_distinct(group)
    ) %>%
    as.list()
  
  
  result <- append(edges_stats, node_stats) %>%
    append(groups_stats)
  
  rm(neutral_graph, edges_stats, node_stats, groups_stats)
  
  result
}

selfLoopsProportion <- function(lon) {
  no_sl <- lon$edges %>% 
    filter(from == to) %>% 
    count() %>% 
    pull()
  no_sl / length(lon$nodes$fitness)
}

graphMetrics <- function(lon) {
  g <- tbl_graph(
    edges = lon$edges, 
    nodes = as_tibble(lon$nodes)
  ) 
  
  result <- g %>%
    activate(nodes) %>%
    mutate(
      clustering = local_transitivity(weights = lon$edges$weight),
      clique = graph_clique_num(),
      graph_reciprocity = graph_reciprocity(),
      graph_mean_dist = 0, #graph_mean_dist(),
      centrality_authority = centrality_authority()
    ) %>%
    as_tibble() %>%
    summarise(
      graph_clique = first(clique),
      graph_reciprocity = first(graph_reciprocity),
      graph_clustering = mean(clustering)
    ) %>%
    append(
      list(
        graph_assortativity_degree = igraph::assortativity_degree(g),
        graph_proportion_self_loops = selfLoopsProportion(lon)
      )
    )
  
  rm(g)
  
  result
}

compute_metrics <- function(lons_folder, problem_config, lon_config) {
  save_path <- sprintf("%s/%s_%s_metrics.rds", lons_folder, lon_config, problem_config)
  if (!file.exists(save_path)) {
    print(sprintf("%s/%s_%s_wfull.rds", lons_folder, lon_config, problem_config))
    full_lon <- readRDS(sprintf("%s/%s_%s_wfull.rds", lons_folder, lon_config, problem_config))
    lon_fla <- c(
      lonMetrics(full_lon),
      neutralMetrics(full_lon),
      graphMetrics(full_lon),
      meanDistanceBetweenLocalOptimas(full_lon)
    )
    rm(full_lon)
    write_rds(lon_fla, file = save_path)
  } else {
   # fla <- read_rds(save_path)
   # full_lon <- read_rds(sprintf("%s/%s_%s_wfull.rds", lons_folder, lon_config, problem_config))
   # new_fla <- c(
   #   lonMetrics(full_lon),
   #   neutralMetrics(full_lon)
   # )
   # fla[names(new_fla)] <- new_fla
   # rm(full_lon)
   # write_rds(fla, file = save_path)
  }
  save_path
}

problems <- all_problems_df() %>%
  filter(
    problem == 'vrf-large',
    budget == 'low',
    type == 'PERM',
    objective == 'MAKESPAN',
    no_jobs <= 200
  ) %>%
  unnest(cols = instances) %>%
  mutate(budget = 'med', stopping_criterion = 'EVALS') %>%
  mutate(problem_id = pmap(., function(problem, type, objective, budget, instance, stopping_criterion, ...) {
    paste(c(problem, type, objective, budget, instance, stopping_criterion), collapse = ";", sep = "_")
  }))


library(progressr)
handlers(global = T)
handlers("progress")

# plan(multisession(workers = 2))

compute_all_metrics <- function() {
  p <- progressor(along = seq_along(problems$problem_id))
  y <- map(seq_along(problems$problem_id), function(i) {
    prob <- problems$problem_id[[i]]
    lon <- lon_configs[[CONFIG_ID]]$id
    compute_metrics(lons_folder, prob, lon)
    p(sprintf("x=%g", i))
  })
}

compute_all_metrics()

