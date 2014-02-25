FROM ubuntu:precise
MAINTAINER John Swinbank <j.swinbank@uva.nl>

# Do not prompt for debconf
ENV DEBIAN_FRONTEND noninteractive

# Ensure the list of packages on the system is up to date.
RUN apt-get update

# Add the SKA-SA packages.
RUN apt-get install -q -y python-software-properties
RUN apt-add-repository ppa:ska-sa/main
RUN apt-get update
RUN apt-get install -q -y casacore libcasacore-dev casacore-data

# Depend on the following to build Pelican/AARTFAAC.
# NB libicu is loaded by Qt with dlopen() at runtime; not clear if it's
# necessary, but it complains if it's not available.
RUN apt-get install -q -y build-essential git cmake libqt4-dev libcppunit-dev \
                          libboost-program-options-dev libfftw3-dev           \
                          wcslib-dev libicu-dev

# Need a more modern version of eigen3 than supplied by Ubuntu 12.04.
# We are hard-coded to use the 3.2.0 release.
ADD http://bitbucket.org/eigen/eigen/get/3.2.0.tar.bz2 /src/eigen
RUN mkdir -p /src/eigen/build && cd /src/eigen/build && \
    cmake ../eigen-eigen-ffa86ffb5570 && make install

# Clone and install Pelican; will end up in /usr/local.
# NB we are using the HEAD of master here -- should we use a tagged release?
RUN cd /src && git clone https://github.com/pelican/pelican.git
RUN mkdir -p /src/pelican/build && cd /src/pelican/build &&  \
    cmake -DCMAKE_BUILD_TYPE=release ../pelican && \
    make -j && make install && ldconfig

# Copy over this repository and build.
# Note that we remove any pre-existing build directory.
ADD . /src/aartfaac
RUN rm -rf /src/aartfaac/build && mkdir -p /src/aartfaac/build && \
    cd /src/aartfaac/build && cmake -DENABLE_OPENMP=ON .. && make install
