Aartfaac Imaging Pipeline
=========================

------------------
Build instructions
------------------
Simple version
--------------

Assuming all dependencies are available, simply::

  $ cd build
  $ cmake ..
  $ make -j

DAS-4 cluster
-------------

Specify the location of QMake, Pelican, etc::

  $ cd build
  $ cmake .. -DQT_QMAKE_EXECUTABLE=/var/scratch/fhuizing/usr/local/bin/qmake -DPELICAN_INCLUDE_DIR=/var/scratch/fhuizing/usr/local/include/pelican -DPELICAN_LIBRARY=/var/scratch/fhuizing/usr/local/lib/libpelican.so -DPELICAN_TESTUTILS_LIBRARY=/var/scratch/fhuizing/usr/local/lib/libpelican-testutils.so -DCASACORE_ROOT=/var/scratch/swinbank/sw-20130410/ -DEIGEN3_INCLUDE_DIR=/var/scratch/fhuizing/usr/local/include/eigen3 -DENABLE_TESTS=OFF -DCMAKE_BUILD_TYPE=Release
  $ make -j

------------
Using Docker
------------

Building
--------

`Docker <http://www.docker.io/>`_ provides a quick and easy way to build and
deploy. The included ``Dockerfile`` will do all the work for you after you
have installed and configured Docker as per its documentation. In brief::

  $ sudo docker build .

will do the trick, producing a Docker image which has the AARTFAAC imaging
tools corresponding to your currently checked-out version of the repository
installed in it.

Smarter is to name the image (say, ``aartfaac`` for the sake of argument) and
tag it wht the SHA1 of the version you are building::

  $ sudo docker build -t aartfaac:$(git log -1 --format="%h") .

Simple Usage
------------

After building, the ``aartfaac-emulator``, ``aartfaac-server`` and
``aartfaac-pipeline`` tools are available on your standard path within the
Docker environment. For example, you can run::

   $ sudo docker run -i -t aartfaac aartfaac-server
   Usage: aartfaac-server <config.xml>

Refer to ``docker run --help`` for details of the command line options.

You can also invoke a shell in your new Docker environment and take a look
around::

  $ sudo docker run -i -t aartfaac bash
  root@9d6a0d260047:/# ls
  bin  boot  dev  etc  home  lib  lib64  media  mnt  opt  proc  root  run
  sbin  selinux  src  srv  sys  tmp  usr  var
  root@9d6a0d260047:/# which aartfaac-server
  /usr/local/bin/aartfaac-server
  root@9d6a0d260047:/# exit

Mounting Volumes
----------------

You can mount volumes from the host filesystems in your Docker image. This
would be convenient e.g. to feed a MeasurementSet to the emulator, or to
provide a place for the pipeline to write out images. Use the ``-v`` option to
``docker run``::

  $ sudo docker run -v /path/on/host:/path/in/docker:ro [...]

The ``ro`` argument marks the directory as "read only" in the Docker
environment. ``rw`` does the obvious.

Network Services
----------------

You can map network ports from the Docker container so that they are
accessible on the host using the ``-p`` option. For example::

  $ sudo docker run -t -i -p 4100 aartfaac nc -l 4100

``nc`` is now running in Docker, listening to port 4100. This port is mapped
to an arbitrary port on the host. Find out which with ``docker ps``::

  $ sudo docker.io ps
  CONTAINER ID        IMAGE               COMMAND             CREATED         STATUS              PORTS                     NAMES
  5c91643035bf        aartfaac:latest     nc -l 4100          5 seconds ago   Up 4 seconds        0.0.0.0:49160->4100/tcp   determined_albattani

That's fine, but it's convenient to specify the host port you are mapping to.
Do that with an extra option to ``-p``::

  $ sudo docker run -t -i -p 4100:4100 aartfaac nc -l 4100

Now in another terminal you can simply run::

  $ nc localhost 4100
  <type some characters>

and watch the message appear on your Docker screen.

*Note* your Pelican XML configuration file requires you specify a hostname
telling Pelican where to bind. This is difficult, because you don't know what
the hostname of your container will be until the container has started. A
quick and easy fix is to tell Pelican to bind to ``0.0.0.0``, which will cause
it to attach to all interfaces, including the one that Docker is exposing::

  <connection host="0.0.0.0" port="4100" timeout="30000" />

Linking Containers
------------------

My might want to link containers directly, without exposing ports on the host.
For example, a container running ``aartfaac-emulator`` should directly connect
to ``aartfaac-server`` without worrying about mapping ports on the host. We
can use the ``--expose`` option when running the container to expose a
particular port to other containers. For example::

  $ sudo docker run -t -i --expose 4100 aartfaac nc -l 4100

We can now connect in to this container on port 4100 from another container...
if we happen to know its IP address which, a priori, we don't. The solution is
to *link* the containers. The server is to give the container running the
server a name::

  $ sudo docker run -t -i --expose 4100 --name nc_listener aartfaac nc -l 4100

Another container can then *link* to the exposed name, and receive information
about the exposed ports via environment variables::

  $ sudo docker.io run -t -i --link nc_listener:nc_listener aartfaac bash
  # echo ${NC_LISTENER_PORT_4100_TCP_ADDR}
  172.17.0.2
  # nc ${NC_LISTENER_PORT_4100_TCP_ADDR} 4100 # Connection is made

AARTFAAC Docker Topology
------------------------

All components of the AARTFAAC imaging system (the server, the pipelines,
and, if required, the emulator) run in separate containers. We can therefore
easily move, reconfigure, redploy, etc them seperately.

The server and the emulator (if applicable) can be linked continers, so that
the emulator can easily connect and supply data to the server.

The server and the pipelines run on different hosts, so linked containers do
not apply. However, the server container simply be explosed to the relevant
network port.

Each container has its own logging, etc (see the Docker documentation). For
simplicity, all our tools should run in separate containers, so we can
start/stop/monitor/etc them individually.

It is not possible to add files to a container when it is starting. In other
words, it is not possible to invoke a container running ``aartfaac-pipeline``
and pass it an XML configuration file. Instead, we need to write some simple
wrappers that convert command line arguments to XML and write out to disk
before spawning the relevant tools.
