FROM ubuntu:20.04

ARG PROXY
ENV DEBIAN_FRONTEND noninteractive
ENV http_proxy $PROXY
ENV https_proxy $PROXY

# update and install dependencies
RUN  apt-get update && apt-get -y --no-install-recommends install \
        wget \
        make \
        git \
        curl \
        r-base \
        cmake \
        libgtest-dev \
        libcurl4-openssl-dev \
        libssl-dev \
        libxml2-dev \
        gcc \
        g++ \
        libeigen3-dev \
        libz-dev \
        libopenmpi-dev \
        doxygen \
        graphviz \
        libgnuplot-iostream-dev \
        ninja-build

# installing ParadisEO
RUN git clone --depth 1 -b master https://github.com/lucasmpavelski/paradiseo.git \
        && cd paradiseo \
        && mkdir build \
        && cd build \
        && cmake -D CMAKE_BUILD_TYPE=Release POSITION_INDEPENDENT_CODE=ON -G Ninja .. \
        && ninja \
        && ninja install

RUN Rscript -e "install.packages(c('future', 'here', 'Rcpp', 'tidyverse', 'irace', 'optparse', 'devtools'))"
RUN Rscript -e "devtools::install_github('lucasmpavelski/schedulingInstancesGeneratoR')"
RUN Rscript -e "devtools::install_github('lucasmpavelski/metaOpt')"

RUN addgroup --gid 1024 mygroup
RUN usermod -a -G mygroup root

# CMD [ "./src/install.sh" ]

# cd /src
# rm _install/CMakeCache.txt
# R CMD INSTALL .
