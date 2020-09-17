FROM ubuntu:20.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && \
    apt-get install -y --no-install-recommends build-essential openjdk-11-jdk git python3-pip python3-dev r-base ruby && \
    apt-get clean && \
    ln -s /usr/bin/python3 /usr/bin/python && \
    git clone --depth 1 --recurse-submodules --shallow-submodules https://github.com/automl/GenericWrapper4AC && \
    cd GenericWrapper4AC/ && \
    python3 setup.py install && \
    cd .. && \
    git clone --depth 1 --recurse-submodules --shallow-submodules https://bitbucket.org/mlindauer/aclib2.git && \
    cd aclib2 && \
    # ConfigSpace dependency issue: https://github.com/automl/ConfigSpace/issues/130
    pip3 install cython && \
    pip3 install -r requirements.txt && \
    cd configurators/ && \
    make install

ENV JAVA_HOME /usr/lib/jvm/java-11-openjdk-amd64/
RUN export JAVA_HOME
