FROM ubuntu:precise
MAINTAINER John Swinbank <j.swinbank@uva.nl>

# Do not prompt for debconf
ENV DEBIAN_FRONTEND noninteractive

# LOFAR libraries will be installed in /usr/local/lib
ENV LD_LIBRARY_PATH /usr/local/lib

# Ensure the list of packages on the system is up to date.
RUN apt-get update

# We need the multiverse repository for pgplot5, which is required by libwcs4.
RUN apt-get install -q -y python-software-properties lsb-release
RUN apt-add-repository "deb http://nl.archive.ubuntu.com/ubuntu/ $(lsb_release -sc) multiverse"
RUN apt-add-repository "deb http://nl.archive.ubuntu.com/ubuntu/ $(lsb_release -sc)-updates multiverse"

# Add the SKA-SA packages.
RUN apt-add-repository ppa:ska-sa/main
RUN apt-get update
RUN apt-get install -q -y casacore libcasacore-dev casacore-data

# Depend on the following to build Pelican/AARTFAAC.
# NB libicu is loaded by Qt with dlopen() at runtime; not clear if it's
# necessary, but it complains if it's not available.
RUN apt-get install -q -y build-essential git cmake libqt4-dev libcppunit-dev \
                          libboost-program-options-dev libfftw3-dev           \
                          wcslib-dev libicu-dev gfortran

# The build environment contains both the AARTFAAC source and the SSH key
# needed to access the LOFAR repository.
ADD . /src/aartfaac

# Clone and install Pelican; will end up in /usr/local.
# NB we are using the HEAD of master here -- should we use a tagged release?
# We are also applying Folkert's workaround for pure virtual method errors on
# Pelican server startup.
RUN mkdir -p /src && cd /src &&                             \
    git clone https://github.com/pelican/pelican.git &&     \
    cd /src/pelican &&                                      \
    cp /src/aartfaac/data/virtfix.patch . &&                \
    git am virtfix.patch &&                                 \
    mkdir -p /src/pelican/build && cd /src/pelican/build && \
    cmake -DCMAKE_BUILD_TYPE=release ../pelican &&          \
    make -j && make install && ldconfig

# Need a more modern version of eigen3 than supplied by Ubuntu 12.04.
# We are hard-coded to use the 3.2.0 release.
ADD http://bitbucket.org/eigen/eigen/get/3.2.0.tar.bz2 /src/eigen.tar.bz2
RUN mkdir -p /src/eigen &&                                            \
    tar jxvf /src/eigen.tar.bz2 --strip-components=1 -C /src/eigen && \
    mkdir -p /src/eigen/build && cd /src/eigen/build &&               \
    cmake .. && make install

# We will also need the LOFAR Storage Manager if we want to read correlated
# data from disk with aartfaac-emulator.
RUN mkdir /root/.ssh &&                                                      \
    ln -s /src/aartfaac/data/lofar-release_2_1_1.deploy /root/.ssh/id_rsa && \
    echo "StrictHostKeyChecking no" > /root/.ssh/config &&                   \
    cd /src &&                                                               \
    git clone git@github.com:transientskp/lofar-release-2_1_1.git ./lofar && \
    mkdir -p lofar/build/gnu_opt &&                                          \
    cd lofar/build/gnu_opt &&                                                \
    cmake -DCMAKE_INSTALL_PREFIX=/usr/local -DBUILD_PACKAGES=LofarStMan      \
          -DBUILD_SHARED_LIBS=ON  -DUSE_LOG4CPLUS=OFF -DENABLE_LIB64=OFF     \
          ../.. &&                                                           \
    make -j install

# Copy over this repository and build.
# Note that we remove any pre-existing build directory.
RUN rm -rf /src/aartfaac/build &&                                            \
    mkdir -p /src/aartfaac/build &&                                          \
    cd /src/aartfaac/build &&                                                \
    cmake -DENABLE_OPENMP=ON -DENABLE_LOFARSTMAN=ON -DENABLE_TESTS=OFF .. && \
    make -j install
