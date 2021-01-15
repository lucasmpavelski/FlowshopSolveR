library(FlowshopSolveR)
library(here)
library(tidygraph)

plotLON <- function(lon) {
  library(ggraph)
  set.seed(2017)
  tidygraph::tbl_graph(
    edges = lon$edges, 
    nodes = as_tibble(lon$nodes) %>%
      mutate(label = row_number()) #map_chr(solutions, ~paste0(.x, collapse = ",")))
  ) %>%
    ggraph() +
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
  cor(neighbors_fit$fitFrom, neighbors_fit$w_fit_avg)
}

avarege_out_degree <- function(lon) {
  dt <- lon$edges %>%
    filter(from != to) %>%
    group_by(from) %>%
    count()
  mean(dt$n)
}

average_disparity <- function(lon) {
  dt <- lon$edges %>% 
    filter(from != to) %>%
    group_by(from) %>%
    mutate(s = sum(weight)) %>%
    summarise(y2 = sum((weight / s)^2))
  mean(dt$y2)
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
  mean(cws$cwi, na.rm = T)
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
  
  append(edges_stats, node_stats) %>%
    append(groups_stats)
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
  
  g %>%
    activate(nodes) %>%
    mutate(
      clustering = local_transitivity(weights = lon$edges$weight),
      clique = graph_clique_num(),
      graph_reciprocity = graph_reciprocity(),
      graph_mean_dist = graph_mean_dist(),
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
}

lonFLAMetics <- function(type, problem, sampling, folder) {
  dir.create(folder, F, T)
  savePath <- file.path(folder, sprintf("%s_%s.Rdata",
                                        paste(sampling, collapse = ";", sep = "_"),
                                        paste(problem, collapse = ";", sep = "_")
  ))
  
  # if (!file.exists(savePath)) {
  print(savePath)
  # }
  # return(list())
  
  if (file.exists(savePath)) {
    load(savePath)
  } else {
    cat('Sampling...', savePath, '\n')
    lon <- sampleLON(
      type,
      rproblem = problem,
      rsampling = sampling,
      seed = 123
    )
    
    metrics <- lonMetrics(lon) %>%
      append(fdcMetrics(lon)) %>%
      append(meanDistanceBetweenLocalOptimas(lon)) %>%
      append(neutralMetrics(lon)) %>%
      append(graphMetrics(lon))
    
    save(lon, metrics, file = savePath)
  }
  
  if (!any(str_starts(names(metrics), 'graph_'))) {
    metrics <- metrics %>% append(graphMetrics(lon))
    save(lon, metrics, file = savePath)
  }
  
  if (!any(str_starts(names(metrics), 'neutral_'))) {
    metrics <- metrics %>% append(neutralMetrics(lon))
    save(lon, metrics, file = savePath)
  }
  
  rm(lon)
  
  metrics
}



initFactories(here("data"))

# lon <- sampleLON(sampleType = type,
#                  rproblem = prob,
#                  rsampling = sample,
#                  123)
# metrics <- lonFLAMetics(type, prob, sample, here('data', 'lons_cache'))



all_problems_df() %>%
  filter(
    problem == 'flowshop',
    budget == 'low',
    dist == 'uniform',
    no_jobs %in% c(30, 50, 100),
    no_machines == 20
  ) %>%
  unnest(cols = instances) %>%
  filter(inst_n == 1) %>%
  mutate(budget = 'med', stopping_criterion = 'EVALS') %>%
  mutate(lon_metrics = pmap(., function(problem, type, objective, budget, instance, stopping_criterion, ...) {
    prob <- c(
      problem = problem, 
      type = type,
      objective = objective, 
      budget = budget, 
      instance = instance, 
      stopping_criterion = stopping_criterion
    )
    sample_type <- 'markov'
    sample = c()
    sample["MarkovChainLONSampling.Init"] = "random"
    sample["MarkovChainLONSampling.Comp.Strat"] = "strict"
    sample["MarkovChainLONSampling.Neighborhood.Size"] = "1"
    sample["MarkovChainLONSampling.Neighborhood.Strat"] = "ordered"
    sample["MarkovChainLONSampling.Local.Search"] = "best_insertion"
    sample["MarkovChainLONSampling.LS.Single.Step"] = "0"
    sample["MarkovChainLONSampling.Perturb"] = "rs"
    sample["MarkovChainLONSampling.Perturb.DestructionSizeStrategy"] = "fixed"
    sample["MarkovChainLONSampling.Perturb.DestructionSize"] = "4"
    sample["MarkovChainLONSampling.Perturb.Insertion"] = "first_best"
    sample["MarkovChainLONSampling.Accept"] = "better"
    sample["MarkovChainLONSampling.Accept.Better.Comparison"] = "equal"
    sample["MarkovChainLONSampling.NumberOfIterations"] = "10000"
    lon <- sampleLON(sampleType = sample_type,
                     rproblem = prob,
                     rsampling = sample,
                     123)
    lon_metrics <- lonFLAMetics(sample_type, prob, sample, here('data', 'lons_cache'))
    print(lon_metrics)
    lon_metrics
  }))
#   mutate(metrics = map(savePath, function(savePath) {
#     print(savePath)
#     load(savePath)
#     1
#   }))
# select(savePath)


