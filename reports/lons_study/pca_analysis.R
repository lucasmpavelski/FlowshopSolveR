library(here)
library(tidyverse)
library(tidygraph)
library(corrr)
library(wrapr)
library(FlowshopSolveR)
library(viridis)
library(FactoMineR)
library("FactoInvestigate")



problems <- all_problems_df() %>%
  crossing(sample_n = 1:50) %>%
  filter(
    problem %in% c('vrf-small', 'vrf-large'),
    budget == 'low',
    type == 'PERM',
    objective == 'MAKESPAN',
    no_jobs <= 300
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


lon_configs <- read_rds(here('reports/lons_study/lon_configs.rds'))[c(1)]

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
    compress_rate =  no_nodes / clon_no_nodes
  )

relative_perfs <- readRDS("~/dev/FlowshopSolveR/reports/lons_study/relative_perfs_best.rds") %>%
  select(all_of(
    c("dist", "corr", "no_jobs", "no_machines",
      "problem", "corv", "objective", "type",
      "model", "inst_n", "instance",
      "instance_features", "id", "ig_rs_rpd", "ig_lsps_rpd")
  )) %>%
  group_by(
    no_jobs, no_machines, problem, dist, corr, type, objective, inst_n, corv, 
    model, instance, instance_features, id
  ) %>%
  summarise(
    ig_rs_rpd = mean(ig_rs_rpd),
    ig_lsps_rpd = mean(ig_lsps_rpd),
    .groups = 'drop'
  )

prob_params <- qc(problem, dist, corr, type, objective, inst_n, corv, 
                  stopping_criterion, budget, model, instance, instance_features, id,
                  sample_type, local_search, perturbation)

metrics_data <- all_fla %>%
  inner_join(relative_perfs, by = c("dist", "corr", "no_jobs", "no_machines",
                                    "problem", "corv", "objective", "type",
                                    "model", "inst_n", "instance",
                                    "instance_features", "id")) %>%
  select(-all_of(prob_params))




colnames(metrics_data) <- qc(
  J,
  M,
  Nnodes,
  Nedges,
  Wmean,
  Wstd,
  AvgFit,
  Wii,
  ffc,
  AvgOut,
  AvgDisp,
  AvgWWC,
  NEdgesNeut,
  RelEdgesNeut,
  WEdgesNeut,
  NoutEdgesNeut,
  NeutSize,
  Clique,
  Recip,
  Clust,
  Assort,
  RelWii,
  MDadj,
  MDprec,
  MDabs,
  MDdev,
  MDshift,
  CLONNnodes,
  CLONNedgesA,
  CLONWmean,
  CLONWstd,
  CLONAvgFit,
  CLONWii,
  CLONffc,
  CLONAvgOut,
  CLONAvgDisp,
  CLONAvgWWC,
  CLONClique,
  CLONRecip,
  CLONClust,
  CLONAssort,
  CLONRelWii,
  Compress,
  RSPerf,
  LSPSPerf
)

quanti_sup <- which(colnames(metrics_data) %in% qc(RSPerf, LSPSPerf))

metrics_pca <- PCA(metrics_data, graph = F, quanti.sup = quanti_sup)

Investigate(metrics_pca, file = here("reports/lons_study/PCA.Rmd"), document = "pdf_document", 
            parallel = TRUE, keepRmd = T
            )

library("corrplot")

cor.mtest <- function(mat, ...) {
  mat <- as.matrix(mat)
  n <- ncol(mat)
  p.mat<- matrix(NA, n, n)
  diag(p.mat) <- 0
  for (i in 1:(n - 1)) {
    for (j in (i + 1):n) {
      tmp <- cor.test(mat[, i], mat[, j], ...)
      p.mat[i, j] <- p.mat[j, i] <- tmp$p.value
    }
  }
  colnames(p.mat) <- rownames(p.mat) <- colnames(mat)
  p.mat
}
# matrix of the p-value of the correlation



metrics_data_pv <- cor.mtest(metrics_data)
metrics_data_corr <- cor(metrics_data)

col <- colorRampPalette(c("#BB4444", "#EE9988", "#FFFFFF", "#77AADD", "#4477AA"))
corrplot(metrics_data_corr, method="color", col=col(200),  
         type="upper", order="hclust", 
         tl.col="black", #Text label color and rotation
         # Combine with significance
         p.mat = metrics_data_pv, sig.level = 0.01, insig = "blank", 
         # hide correlation coefficient on the principal diagonal
         diag=FALSE 
)


