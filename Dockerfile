FROM ubuntu:18.04

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
        libeigen3-dev

# installing ParadisEO
RUN wget https://gforge.inria.fr/frs/download.php/31732/ParadisEO-2.0.1.tar.gz \
 && tar -xzf ParadisEO-2.0.1.tar.gz \
 && cd ParadisEO-2.0 \
 && mkdir build \
 && cd build \
 && sed -i -e 's/CMAKE_MINIMUM_REQUIRED(VERSION 2.6)/CMAKE_MINIMUM_REQUIRED(VERSION 3.1)/g' ../eo/CMakeLists.txt \
 && cmake .. -DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=true \
 && make \
 && make install

# install google test
RUN cd /usr/src/gtest \
 && cmake CMakeLists.txt \
 && make \
 && cp *.a /usr/lib

RUN Rscript -e "install.packages(c('Rcpp', 'tidyverse', 'irace', 'optparse', 'caret', 'keras', 'xgboost', 'FSelector', 'utiml', 'rpart', 'randomForest', 'e1071', 'infotheo', 'kknn', 'knitr', 'rJava', 'RWeka', 'C50'))"
