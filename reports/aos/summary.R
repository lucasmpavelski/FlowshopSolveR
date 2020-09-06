library(tidyverse)
library(here)

# FSP,PERM,MAKESPAN,med,taillard_rand_100_5_01.dat,FIXEDTIME_1,0,9.9999,0,3,2,0.5,2,4,0,0,0,3

problems <- read_csv(here("data", 'taillard_instances.csv')) %>%
  crossing(
    problem = 'FSP',
    type = 'PERM',
    objective = 'MAKESPAN',
    budget = 'med',
    stopping_criterion = 'FIXEDTIME'
  )

configs <- crossing(
  IG.Init.Strat         = 0,
  IG.Comp.Strat         = 0,
  IG.Neighborhood.Size  = 9.9999,
  IG.Neighborhood.Strat = 0,
  IG.Local.Search       = 3,
  IG.Accept             = 2,
  IG.Accept.Temperature = 0.5,
  IG.Algo               = c(0, 2),
  IG.Destruction.Size = c(2,4,8),
  IG.LS.Single.Step = 0,
  IG.LSPS.Local.Search = 0,
  IG.LSPS.Single.Step = 0,
  IG.AOS.Strategy = c(0, 1, 2, 3, 4)
) %>%
  filter(IG.Algo == 0 & IG.AOS.Strategy == 0 |
         IG.Algo == 2 & IG.Destruction.Size == 4)

params <- colnames(configs)

experiments <- crossing(problems, configs)

experiments <- experiments %>%
  mutate(
    instance = sprintf("taillard_rand_%d_%d_%02d.dat", no_jobs, no_machines, inst_n),
    problem_model = paste(problem, type, objective, budget, instance, stopping_criterion, sep = ','),
    config = paste(
      IG.Init.Strat,
      IG.Comp.Strat,
      IG.Neighborhood.Size,
      IG.Neighborhood.Strat,
      IG.Local.Search,
      IG.Accept,
      IG.Accept.Temperature,
      IG.Algo,
      IG.Destruction.Size,
      IG.LS.Single.Step,
      IG.LSPS.Local.Search,
      IG.LSPS.Single.Step,
      IG.AOS.Strategy,
      sep = ','
    ),
    filename = here("runs", "ig-random-fast", paste0('1_', problem_model, '_', config, '.out')),
    valid_result = file.exists(filename)
  ) %>%
  filter(valid_result)

loadResults <- function(fn) {
  lines <- read_lines(fn, skip = 2)
  lines <- lines[-length(lines)]
  lines <- str_split_fixed(lines, "\\s+", 3)
  tibble(
    runtime = as.integer(lines[,1]),
    fitness = as.integer(lines[,2])
  )
}

results <- experiments %>%
  mutate(
    results = map(filename, loadResults)
  ) %>%
  unnest(cols = c("results"))

summ_results <- results %>%
  mutate(
    config = case_when(
      config == '1,0,9.9999,0,3,2,0.5,0,2,0,0,0,0' ~ 'IG (d = 2)',
      config == '1,0,9.9999,0,3,2,0.5,0,4,0,0,0,0' ~ 'IG (d = 4)',
      config == '1,0,9.9999,0,3,2,0.5,0,8,0,0,0,0' ~ 'IG (d = 8)',
      config == '1,0,9.9999,0,3,2,0.5,2,4,0,0,0,0' ~ 'PM',
      config == '1,0,9.9999,0,3,2,0.5,2,4,0,0,0,1' ~ 'MAB',
      config == '1,0,9.9999,0,3,2,0.5,2,4,0,0,0,2' ~ 'LinUCB',
      config == '1,0,9.9999,0,3,2,0.5,2,4,0,0,0,3' ~ 'TS',
      config == '1,0,9.9999,0,3,2,0.5,2,4,0,0,0,4' ~ 'Rand',
      T ~ config
    )
  ) %>%
  group_by(problem_model, config) %>%
  filter(runtime == max(runtime)) %>%
  mutate(rpd = (fitness - lower_bound) / lower_bound)

library(PMCMRplus)
r1 <- summ_results %>% filter(inst_n == 1)
frdAllPairsConoverTest(r1$rpd, as.factor(r1$config), blocks = as.factor(paste(r1$no_jobs, r1$no_machines)))

  
ggplot(summ_results) +
  geom_boxplot(aes(x = config, y = rpd)) 

summ_results %>%
  group_by(config, no_jobs, no_machines) %>%
  summarise(rpd = mean((fitness - lower_bound) / lower_bound)) %>%
  View()


View(results)

ggplot(results) +
  facet_wrap(~instance, scales = "free") +
  geom_line(aes(x = runtime, y = fitness, color = config))
  
