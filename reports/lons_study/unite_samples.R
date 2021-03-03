library(tidyverse)
library(here)
library(FlowshopSolveR)
library(optparse)

lon_configs <- read_rds(here('reports/lons_study/lon_configs.rds'))

option_list <- list( 
    make_option(c("-c", "--config_id"), action="dist", default=10, type='integer',
                help="Print extra output [default]")
)

opt <- parse_args(OptionParser(option_list=option_list))
CONFIG_ID <- opt$config_id

lons_folder <- here("data", "lons_cache")

nodes_df <- function(nodes, lonn) {
    bind_rows(map(nodes, function(x) { names(x) <- 1:length(x); x })) %>% 
        rowid_to_column() %>% 
        mutate(lon = lonn)
}

join_nodes_df <- function(s1df, s2df, sol_size) {
    s1df %>% full_join(s2df, by = as.character(1:sol_size)) %>% rowid_to_column()
}


unite_lons <- function(lons_folder, problem_config, lon_config) {
    savePath <- sprintf("%s/%s_%s_wfull.rds", lons_folder, lon_config, problem_config)
    if (!file.exists(savePath)) {
        s1 <- read_rds(sprintf("%s/%s_%s_1.rds", lons_folder, lon_config, problem_config))
        s1$nodes$solutions <- map(s1$nodes$solutions, as.integer)
        
        allnodes <- nodes_df(s1$nodes$solutions, 1)
        sol_size <- length(s1$nodes$solutions[[1]])
        all_fitness <- s1$nodes$fitness
        all_edges <- s1$edges
        
        for (i in 2:50) {
          print(sprintf("%s/%s_%s_%s.rds", lons_folder, lon_config, problem_config, i))
            s2 <-read_rds(sprintf("%s/%s_%s_%s.rds", lons_folder, lon_config, problem_config, i))
            s2$nodes$solutions <- map(s2$nodes$solutions, as.integer)
            
            
            inodes <- nodes_df(s2$nodes$solutions, i)
            allnodes <- join_nodes_df(allnodes, inodes, sol_size)
            
            nodes_map_s2 <- allnodes %>% filter(!is.na(rowid.y)) %>% arrange(rowid.y) %>% pull(rowid)
            new_nodes_s2 <- allnodes %>% filter(is.na(rowid.x)) %>% pull(rowid.y)
            
            allnodes <- allnodes %>% select(-rowid.x, -rowid.y, lon.x, lon.y) %>% 
                mutate(lon = 1)
            
            all_fitness <- c(all_fitness, s2$nodes$fitness[new_nodes_s2])
            all_edges <- s2$edges %>%
                mutate(
                    from = map_dbl(from, ~nodes_map_s2[.x]),
                    to = map_dbl(to, ~nodes_map_s2[.x])
                ) %>%
                bind_rows(all_edges) %>%
                group_by(from, to) %>%
                summarise(weight = sum(weight), .groups = 'drop')
            
            rm(s2)
            rm(inodes)
            rm(nodes_map_s2)
            rm(new_nodes_s2)
        }
        new_nodes <- allnodes %>%
            select(as.character(1:sol_size)) %>%
            pmap(c)
        all_edges <- all_edges %>%
          group_by(from) %>%
          mutate(weight = weight / sum(weight)) %>%
          ungroup()
        merged_lon <- list(
            nodes = list(
                solutions = new_nodes,
                fitness = all_fitness
            ),
            edges = all_edges
        )
        saveRDS(merged_lon, savePath)
        rm(new_nodes)
        rm(merged_lon)
    }
    savePath
}

problems <- all_problems_df() %>%
    filter(
        problem == 'vrf-large',
        budget == 'low',
        type == 'PERM',
        objective == 'MAKESPAN',
        no_jobs <= 400
    ) %>%
    unnest(cols = instances) %>%
    mutate(budget = 'med', stopping_criterion = 'EVALS') %>%
    mutate(problem_id = pmap(., function(problem, type, objective, budget, instance, stopping_criterion, ...) {
        paste(c(problem, type, objective, budget, instance, stopping_criterion), collapse = ";", sep = "_")
    }))

# library(progressr)
# handlers(global = TRUE)
# handlers("progress")

unite_all_lons <- function() {
    # p <- progressor(along = seq_along(problems$problem_id))
    y <- map(seq_along(problems$problem_id), function(i) {
        prob <- problems$problem_id[[i]]
        lon <- lon_configs[[CONFIG_ID]]$id
        unite_lons(here('data', 'lons_cache'), prob, lon)
        # p(sprintf("x=%g", i))
    })
}

unite_all_lons()


