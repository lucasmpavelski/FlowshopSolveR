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
        libgnuplot-iostream-dev

# installing ParadisEO
RUN git clone --depth 1 -b master https://github.com/nojhan/paradiseo.git \
 && cd paradiseo \
 && mkdir build \
 && cd build \
 && sed -i -e 's/CMAKE_MINIMUM_REQUIRED(VERSION 2.6)/CMAKE_MINIMUM_REQUIRED(VERSION 3.1)/g' ../eo/CMakeLists.txt \
 && cmake -D CMAKE_BUILD_TYPE=Release -DEDO=ON .. \
 && make -j \
 && make install

 # invoke-rc.d arno-iptables-firewall start

# install google test
#RUN cd /usr/src/gtest \
# && cmake CMakeLists.txt \
# && make \
# && cp *.a /usr/lib

RUN Rscript -e "install.packages(c('future', 'here', 'Rcpp', 'tidyverse', 'irace', 'optparse', 'caret', 'keras', 'xgboost', 'FSelector', 'utiml', 'rpart', 'randomForest', 'e1071', 'infotheo', 'kknn', 'knitr', 'rJava', 'RWeka', 'C50'))"
