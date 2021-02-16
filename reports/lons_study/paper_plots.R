library(here)
library(tidyverse)
library(tidygraph)
library(corrr)
library(wrapr)
library(FlowshopSolveR)
library(viridis)

problems <- all_problems_df() %>%
  crossing(sample_n = 1:50) %>%
  filter(
    problem %in% c('vrf-small', 'vrf-large'),
    budget == 'low',
    type == 'PERM',
    objective == 'MAKESPAN',
    no_jobs <= 200
  ) %>%
  unnest(cols = instances) %>%
  mutate(budget = 'med', stopping_criterion = 'EVALS') %>%
  mutate(lon_metrics = pmap(., function(problem, type, objective, budget, instance, stopping_criterion, sample_n, ...) {
    prob <- c(
      problem = problem,
      type = type,
      objective = objective,
      budget = budget,
      instance = instance,
      stopping_criterion = stopping_criterion
    )
  }))


lon_configs <- read_rds(here('reports/lons_study/lon_configs.rds'))[c(1,2,9,10)]

all_fla <- tibble(
    sample_type = map_chr(lon_configs, ~.x$sample_type),
    local_search = map_chr(lon_configs, ~.x$local_search),
    perturbation = map_chr(lon_configs, ~.x$perturbation),
    metrics_path = map_chr(lon_configs, ~here(sprintf('data/lons_cache/%s_metrics.rds', .x$id)))
  ) %>%
  filter(file.exists(metrics_path)) %>%
  mutate(metrics = map(metrics_path, readRDS)) %>%
  select(-metrics_path) %>%
  unnest(cols = c(metrics)) %>%
  mutate(
    compress_rate = neutral_no_groups / no_nodes
  )

prob_params <- qc(problem, dist, corr, type, objective, no_jobs, no_machines, inst_n, corv, 
                  stopping_criterion, budget, model, instance, instance_features, id, metrics_path,
                  sample_type, local_search, perturbation)
metrics <- all_fla %>% select(-all_of(prob_params)) %>% colnames()

relative_perfs <- readRDS("~/dev/FlowshopSolveR/reports/lons_study/relative_perfs.rds")

all_fla_tidy <- all_fla %>%
  select(-budget, -stopping_criterion) %>%
  pivot_longer(all_of(metrics), names_to = "metric", values_to = "value") %>%
  inner_join(relative_perfs, by = c("dist", "corr", "no_jobs", "no_machines",
                                    "problem", "corv", "objective", "type",
                                    "model", "inst_n", "instance", 
                                    "instance_features", "id")) %>%
  mutate(
    local_search = factor(
      local_search,
      levels = c('II', 'FI', 'BI'),
      labels = c('Iterative', 'First', 'Best')
    ),
    perturbation = factor(
      perturbation,
      levels = c('RS', 'LSPS')
    ),
    no_machines = as.factor(no_machines),
    no_jobs = as.factor(no_jobs)
  )



histogram <- function(dt) {
  dt %>%
    ggplot() +
      facet_grid(local_search ~ perturbation) +
      geom_histogram(aes(x = value, fill = no_jobs)) +
      scale_fill_viridis(discrete = T) +
      theme_bw() +
      labs(fill = 'J') +
      theme(axis.title = element_blank(),
            text = element_text(size = 8))
}

histogram_machine <- function(dt) {
  dt %>%
    ggplot() +
    facet_grid(local_search ~ perturbation) +
    geom_histogram(aes(x = value, fill = no_machines)) +
    scale_fill_viridis(discrete = T) +
    theme_bw() +
    labs(fill = 'M') +
    theme(axis.title = element_blank(),
          text = element_text(size = 8))
}

plt <- all_fla_tidy %>%
  filter(
    sample_type == 'markov',
    metric == 'no_nodes'
  ) %>%
  histogram()
plt

ggsave(
  here('reports/lons_study/plots/no_nodes.pdf'),
  plt,
  width = 6.6,
  height = 2,
  units = 'in'
)


plt <- all_fla_tidy %>%
  filter(
    sample_type == 'markov',
    metric == 'average_weight_of_self_loops',
    local_search != 'BS'
  ) %>%
  mutate(no_jobs = as.factor(no_jobs)) %>%
  mutate(value = if_else(is.nan(value), 0.5, value)) %>%
  histogram()
plt


ggsave(
  here('reports/lons_study/plots/wii.pdf'),
  plt,
  width = 6.6,
  height = 2,
  units = 'in'
)


all_fla_tidy %>%
  filter(
    sample_type == 'markov',
    metric == 'avarege_out_degree'
  ) %>%
  histogram_machine() +
  scale_x_log10()

ggsave(
  here('reports/lons_study/plots/avarege_out_degree.pdf'),
  last_plot(),
  width = 6.6,
  height = 2,
  units = 'in'
)

# knnWeighted assortativity.wccWeighted clustering coefficient.
# Measures “cliquishness”of a neighbourhood.fnnFitness-fitness correlation.
# Measures the correlation be-tween the fitness values of adjacent local optima.


all_fla_tidy %>%
  filter(
    sample_type == 'markov',
    metric == 'average_disparity'
  ) %>%
  histogram_machine()


ggsave(
  here('reports/lons_study/plots/average_disparity.pdf'),
  last_plot(),
  width = 6.6,
  height = 2,
  units = 'in'
)


all_fla_tidy %>%
  filter(
    sample_type == 'markov',
    metric == 'graph_assortativity_degree'
  ) %>%
  histogram_machine()

ggsave(
  here('reports/lons_study/plots/graph_assortativity_degree.pdf'),
  last_plot(),
  width = 6.6,
  height = 2,
  units = 'in'
)


all_fla_tidy %>%
  filter(
    sample_type == 'markov',
    metric == 'average_weighted_clustering_coefficient'
  ) %>%
  mutate(value = value + 0.00001) %>%
  histogram() +
  scale_x_log10()

ggsave(
  here('reports/lons_study/plots/average_weighted_clustering_coefficient.pdf'),
  last_plot(),
  width = 6.6,
  height = 2,
  units = 'in'
)



all_fla_tidy %>%
  filter(
    sample_type == 'markov',
    metric == 'fitness_fitness_correlation'
  ) %>%
  mutate(value = value) %>%
  histogram() 

ggsave(
  here('reports/lons_study/plots/fitness_fitness_correlation.pdf'),
  last_plot(),
  width = 6.6,
  height = 2,
  units = 'in'
)


all_fla_tidy %>%
  filter(
    sample_type == 'markov',
    metric == 'neutral_no_groups'
  ) %>%
  mutate(value = value) %>%
  histogram() +
  scale_x_continuous()

ggsave(
  here('reports/lons_study/plots/neutral_no_groups.pdf'),
  last_plot(),
  width = 6.6,
  height = 2,
  units = 'in'
)


all_fla_tidy %>%
  filter(
    sample_type == 'markov',
    metric == 'compress_rate',
    !is.na(local_search)
  ) %>%
  mutate(value = 1 - value) %>%
  histogram_machine() +
  scale_x_continuous() +
  scale_x_log10()

ggsave(
  here('reports/lons_study/plots/compress_rate.pdf'),
  last_plot(),
  width = 6.6,
  height = 2,
  units = 'in'
)



all_fla_tidy %>%
  filter(
    sample_type == 'markov',
    metric == 'md_shiftDistance'
  ) %>%
  mutate(value = value) %>%
  ggplot() +
  geom_violin(aes(x = value, fill = no_jobs, y = paste(local_search, perturbation))) +
  scale_fill_viridis(discrete = T) +
  coord_flip() +
  theme_bw() +
  labs(fill = 'J') +
  theme(axis.title = element_blank(),
        text = element_text(size = 8))
  # histogram() +
  scale_x_continuous()

ggsave(
  here('reports/lons_study/plots/md_shiftDistance.pdf'),
  last_plot(),
  width = 6.6,
  height = 2,
  units = 'in'
)
