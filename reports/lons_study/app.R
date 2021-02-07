library(shiny)
library(here)
library(tidyverse)
library(caret)
library(rpart.plot)
library(factoextra)
library(ggraph)
library(tidygraph)
library(wrapr)
library(corrr)
library(FlowshopSolveR)

problems <- all_problems_df() %>%
  crossing(sample_n = 1:50) %>%
  filter(
    problem == 'vrf-small',
    budget == 'low',
    type == 'PERM',
    objective == 'MAKESPAN'
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


lon_configs <- read_rds(here('reports/lons_study/lon_configs.rds'))

all_fla <- tibble(sample = map_chr(lon_configs, ~.x$id)) %>%
  mutate(metrics_path = map_chr(sample, ~here(sprintf('data/lons_cache/%s_metrics.rds', .x)))) %>%
  filter(file.exists(metrics_path)) %>%
  mutate(metrics = map(metrics_path, readRDS)) %>%
  select(-metrics_path) %>%
  unnest(cols = c(metrics))

configs_select <- map(lon_configs, ~.x$id)
names(configs_select) <- map(lon_configs, ~.x$name)

# all_fla <- lon_fla %>%
#   mutate(no_jobs = as.integer(no_jobs),
#          no_machines = as.integer(no_machines)) %>%
#   # select(-inst_n) %>%
#   mutate_if(is.character, as.factor)
# 
prob_params <- qc(sample, problem, dist, corr, type, objective, no_jobs, no_machines, inst_n, corv, 
                  stopping_criterion, budget, model, instance, instance_features, id, metrics_path)
metrics <- all_fla %>% select(-all_of(prob_params)) %>% colnames()
all_fla_tidy <- all_fla %>%
  pivot_longer(metrics, names_to = "metric", values_to = "value")

uniqueChar <- function(v) unique(as.character(v))

no_jobs <- uniqueChar(all_fla$no_jobs)
no_machs <- uniqueChar(all_fla$no_machines)
dists <- uniqueChar(all_fla$dist)
corrs <- uniqueChar(all_fla$corr)
types <- uniqueChar(all_fla$type)
objectives <- uniqueChar(all_fla$objective)



dataFilters <- function() {
  sidebarPanel(
    selectInput("sample",
                "Sample:",
                multiple = T,
                configs_select),
    selectInput("metrics",
                "Metric:",
                metrics,
                multiple = F,
                selected = "no_nodes"),
    selectInput("no_jobs",
                "Number of jobs:",
                no_jobs, no_jobs,
                multiple = T),
    selectInput("no_machs",
                "Number of machines:",
                no_machs, no_machs,
                multiple = T),
    selectInput("dists",
                "Distribution:",
                dists, dists,
                multiple = T),
    selectInput("corrs",
                "Correlation:",
                corrs, corrs,
                multiple = T),
    selectInput("types",
                "FSP types:",
                types, types,
                multiple = T),
    selectInput("objectives",
                "Objectives:",
                objectives, objectives,
                multiple = T)
  )
}

flaMetricsPanel <- function() {
  sidebarLayout(
    dataFilters(),
    mainPanel(
      tabsetPanel(
        tabPanel("Histogram",
                 fluidRow(
                   column(
                     3,
                     selectInput("fill", "Fill:", 
                                 c("all", "no_jobs", "no_machines", "dist", "corr", "objective", "type"),
                                 selected = "objective")
                   ),
                   column(
                     3,
                     selectInput("rows", "Rows:",
                                 c("all", "no_jobs", "no_machines", "dist", "corr", "objective", "type", "sample"),
                                 selected = NULL)
                   ),
                   column(
                     3,
                     selectInput("cols", "Columns:",
                                 c("all", "no_jobs", "no_machines", "dist", "corr", "objective", "type"),
                                 selected = NULL)
                   ),
                   column(
                     3,
                     textInput("limita", "Limit:", 0),
                     textInput("limitb", "Limit:", 0),
                     checkboxInput("log", "Log scale:", F)
                   )
                 ),
                 plotOutput("histogram"), #, height = "180px", width = '748px'),
                 downloadButton("histogramPdf"),
                 verbatimTextOutput("summary")
        ),
        tabPanel("Data",
                 dataTableOutput("table")
        ),
        tabPanel("Model",
                 plotOutput("rpart")
        )
      )
    )
  )
}

lonsPanel <- function() {
  sidebarLayout(
    sidebarPanel(
      selectInput("lon_sample",
                  "LON sample",
                  configs_select),
      selectInput("lon_problem",
                  "Problem",
                  unique(problems$problem)),
      selectInput("lon_type",
                  "Type",
                  unique(problems$type)),
      selectInput("lon_objective",
                  "Objective",
                  unique(problems$objective)),
      selectInput("lon_budget",
                  "Budget",
                  unique(problems$budget)),
      selectInput("lon_instance",
                  "Instance",
                  unique(problems$instance)),
      selectInput("lon_stopping_criterion",
                  "Stopping criterion",
                  unique(problems$stopping_criterion))
    ),
    mainPanel(
      selectInput("lonLayout", "Layout", c('nicely', 'tree', 'circle', 'circlepack', 'kk', 'fr'), "circle"),
      plotOutput("lonPlot", height = '100vh')
    )
  )
}

dtPanel <- function() {
  sidebarLayout(
    sidebarPanel(
      # selectInput("dt_corr",
      #             "Correlation:",
      #             c("all", corrs)),
      # selectInput("dt_type",
      #             "FSP type:",
      #             c("all", types)),
      # selectInput("dt_objective",
      #             "Objective:",
      #             c("all", objectives))
      
      selectInput("dt_no_jobs",
                  "Number of jobs:",
                  no_jobs, no_jobs,
                  multiple = T),
      selectInput("dt_no_machs",
                  "Number of machines:",
                  no_machs, no_machs,
                  multiple = T),
      selectInput("dt_dists",
                  "Distribution:",
                  dists, dists,
                  multiple = T),
      selectInput("dt_corrs",
                  "Correlation:",
                  corrs, corrs,
                  multiple = T),
      selectInput("dt_types",
                  "FSP types:",
                  types, types,
                  multiple = T),
      selectInput("dt_objectives",
                  "Objectives:",
                  objectives, objectives,
                  multiple = T)
    ),
    mainPanel(
      plotOutput("dtModelPlot", height = "500px")
    )
  )
}

ui <- fluidPage(
  tabsetPanel(
    tabPanel(
      "FLA metrics",
      flaMetricsPanel()
    ),
    tabPanel(
      "LONs",
      lonsPanel()
    )
  )
)

server <- function(input, output) {
  
  filteredDt <- reactive({
    filteredDt <- all_fla_tidy %>%
      filter(
        dist %in% input$dists,
        corr %in% input$corrs,
        no_jobs %in% input$no_jobs,
        no_machines %in% input$no_machs,
        type %in% input$types,
        objective %in% input$objectives,
        sample %in% input$sample
      )
  })
  
  filteredLonMetrics <- reactive({
    filteredLonMetrics <- filteredDt() %>%
      filter(
        metric == input$metrics
      )
  })
   
  plotHistogram <- reactive({
    plt <- filteredLonMetrics() %>%
      mutate(obj_type = paste(objective, type)) %>%
      mutate(all = "all") %>%
      mutate_at(input$fill, as.factor) %>%
      ggplot(aes_string(fill = input$fill)) + 
        facet_wrap(
          ~sample, ncol = 2
        ) +
        geom_histogram(aes(x = value), bins = 30) +
        theme_bw() +
        theme(legend.position = 'bottom',
              axis.text.y = element_blank(),
              axis.ticks.y = element_blank())
    limita <- as.numeric(input$limita)
    limitb <- as.numeric(input$limitb)
    if (input$log) {
      plt <- plt +
        scale_x_log10(limits = c(limita, limitb))
    }
    plt
  })
  
   output$histogram <- renderPlot({
     plotHistogram()
   })
   
   output$histogramPdf <- downloadHandler(
     filename = "test.pdf",
     content = function(file) {
       device <- function(..., filename, width, height) {
         grDevices::pdf(..., file = filename,  width = 9, 
                        height = 2.2, family = "Times")
       }
       ggsave(file, plot = plotHistogram(), device = device)
     })
   
   output$no_missing <- renderText({
     filteredLonMetrics() %>% pull(value) %>% is.na %>% sum
   })
   
   output$summary <- renderPrint({
     print(filteredLonMetrics() %>% pull(value) %>% summary())
     print(input$metrics)
   })
   
   output$table <- renderDataTable({
     filteredLonMetrics() %>%
       mutate(instance = instanceNameForConfig(.)) %>%
       select(instance, objective, type, value)
   })
   
   output$rpart <- renderPlot({
     dt <- filteredLonMetrics() %>% select(-inst_n)
     dt <- dt[complete.cases(dt),]
     model <- train(
       x = dt %>% select(-metric, -value) %>% as.data.frame(),
       y = dt %>% pull(value),
       method = 'rpart'
     )
     rpart.plot(model$finalModel)
   })
   
   output$cor <- renderPlot({
     cormat %>% 
       corrplot::corrplot(
         type = "upper",
         order = 'hclust',
         diag = F,
         tl.cex = 1
       )
   })
   
   output$igPerfCor <- renderDataTable({
     cormat %>% 
       as_cordf() %>%
       focus(ig_perf) %>%
       arrange(ig_perf)
   })
   
   output$pcaEig <- renderPlot({
     fviz_eig(pca_model)
   })
   
   output$pcaVar1 <- renderPlot({
     cols <- rownames(pca_model$var$coord)
     fviz_pca_var(
        title = "",
        pca_model, repel = F, axes = c(1,2),
                  #repel = T,
        col.var = case_when(
          cols %in% solution_statistics_metrics ~ "Solution statistics",
          cols %in% random_walk_metrics ~ "Random walk based",
          str_starts(cols, "fdc_") | cols == 'mean_no_steps' ~ 'Adaptive walk and FDC metrics',
          str_starts(cols, "md_") ~ 'Local optima mean distances',
          cols %in% lon_metrics ~ 'LON metrics',
          str_starts(cols, "neutral_") ~ 'CLON metrics',
          TRUE ~ 'Other'
        ),
        alpha.var = 0.5,
        select.var = list(
           name = qc(ig_perf,
                     no_jobs,
                     no_machines,
                     fdc_shiftDistance,
                     fdc_precedenceDistance,
                     side, slope,
                     neutral_mean_size,
                     no_nodes,
                     neutral_no_groups,
                     # entropy,
                     partial_inf,
                     density_basin,
                     # fitness_fitness_correlation,
                     average_weight_of_self_loops,
                     average_disparity,
                     rw_autocorr_1,
                     md_precedenceDistance,
                     md_shiftDistance,
                     mean_no_steps
            )
        )
      )
   })
   
   output$pcaVar2 <- renderPlot({
     fviz_pca_var(pca_model, repel = T, axes = c(3,4), alpha.var = 'cos2')
   })
   
   output$pcaInd1 <- renderPlot({
     cols <- pca_data %>% pull(input$colInd)
     fviz_pca_ind(pca_model, geom = "point", axes = c(1,2), col.ind = cols, addEllipses = input$addEllipses)
   })
   
   output$pcaInd2 <- renderPlot({
     cols <- pca_data %>% pull(input$colInd)
     fviz_pca_ind(pca_model, geom = "point", axes = c(3,4), col.ind = cols, addEllipses = input$addEllipses)
   })
   
   output$mcaCor1 <- renderPlot({
     fviz_mca_var(mca_model, choice = "mca.cor", axes = c(1,2))
   })
   
   output$mcaVar1 <- renderPlot({
     fviz_mca_var(mca_model,
                  select.var = list(cos2 = 0.10),
                  col.quanti.sup = "blue",
                  repel = T,
                  axes = c(1,2))
   })
   
   output$mcaVar2 <- renderPlot({
     fviz_mca_var(mca_model,
                  select.var = list(cos2 = 0.10),
                  col.quanti.sup = "blue",
                  repel = T,
                  axes = c(3,4))
   })
   
   output$mcaVar2 <- renderPlot({
     fviz_mca_var(mca_model,
                  select.var = list(cos2 = 0.10),
                  col.quanti.sup = "blue",
                  repel = T,
                  axes = c(3,4))
   })
   
   output$mcaEig <- renderPlot({
     fviz_eig(mca_model)
   })
   
   output$lonPlot <-  renderPlot({
     problem <-  paste(c(
       input$lon_problem, 
       input$lon_type, 
       input$lon_objective, 
       input$lon_budget, 
       input$lon_instance, 
       input$lon_stopping_criterion), collapse = ";", sep = "_")
     fn <- sprintf("%s_%s_full.rds", input$lon_sample, problem)
     fn <- here('data', 'lons_cache', fn)
     print(fn)
     if (file.exists(fn)) {
       print("reading...")
       lon <- readRDS(fn)
       print("drawing...")
       # neutral_graph <- tidygraph::tbl_graph(
       #   edges = lon$edges,
       #   nodes = as_tibble(lon$nodes)
       # ) %>%
       #   activate(edges) %>%
       #   filter(from != to, .N()$fitness[from] == .N()$fitness[to])
       # 
       # # tidygraph::tbl_graph(
       # #   edges = lon$edges,
       # #   nodes = as_tibble(lon$nodes) %>%
       # #     mutate(label = row_number()) #map_chr(solutions, ~paste0(.x, collapse = ",")))
       # # ) %>%
       # neutral <- tidygraph::tbl_graph(
       #   edges = lon$edges,
       #   nodes = as_tibble(lon$nodes)
       # ) %>% morph(to_contracted, fitness) %>% crystallize() %>% 
       #   pull(graph)
       
       # neutral[[1]] %>%
       # browser()
       tidygraph::tbl_graph(
           edges = lon$edges,
           nodes = as_tibble(lon$nodes)
         ) %>%
         ggraph(layout = input$lonLayout) +
         geom_edge_link(edge_alpha = .05) +
         geom_node_point(aes(color = 1)) +
         theme_graph()
       # geom_edge_loop(edge_alpha = .05) +
     }
   })
  
  rfSelected <- reactive({
    fn <- here::here(str_interp(
      "data/fla/models/rf_${rf_type}_${rf_corr}_${rf_objective}.Rdata",
      reactiveValuesToList(input)))
    load(fn)
    print(rf_model)
    rf_model
  })
   
  output$rfImportance <- renderPlot({
    model <- rfSelected()$finalModel
    randomForest::varImpPlot(model)
  })
  
  output$rfSummary <- renderTable({
    rfSelected()$resample
  })
  
  getInfoGrain <- function() {
    information.gain(
      ig_perf ~ .,
      data = all_fla
    )
  }

  output$infGainPlot <- renderPlot({
    fn <- here::here(str_interp(
      "data/fla/models/ig_${ig_type}_${ig_corr}_${ig_objective}.Rdata",
      reactiveValuesToList(input)))
    load(fn)
    inf_gain %>%
      rownames_to_column("metric") %>%
      arrange(desc(attr_importance)) %>%
      mutate(metric = fct_reorder(metric, (attr_importance))) %>%
      ggplot() +
        geom_col(aes(x = metric, y = attr_importance)) +
        coord_flip()
  })
  
  output$dtModelPlot <- renderPlot({
    # fn <- here::here(str_interp(
    #   "data/fla/models/dt_${dt_type}_${dt_corr}_${dt_objective}.Rdata",
    #   reactiveValuesToList(input)))
    # load(fn)
    train_dt <- all_fla %>%
      filter(
        dist %in% input$dt_dists,
        corr %in% input$dt_corrs,
        no_jobs %in% input$dt_no_jobs,
        no_machines %in% input$dt_no_machs,
        type %in% input$dt_types,
        objective %in% input$dt_objectives
      )
    inputs <- train_dt %>% select(-inst_n, -ig_perf)
    colnames(inputs) <- make.names(colnames(inputs))
    dt_model <- train(
      x = inputs,
      y = train_dt$ig_perf,
      method = "rpart",
      trControl = trainControl(
        number = 1
      )
    )
    rpart.plot::rpart.plot(dt_model$finalModel)
  })
}

# Run the application 
shinyApp(ui = ui, server = server)

