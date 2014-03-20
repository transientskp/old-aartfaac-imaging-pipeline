AARTFAAC Imaging Pipeline
=========================

The AARTFAAC Imaging Pipeline is a realtime imaging system for the AARTFAAC
All Sky Monitor. The ASM uses the main core of the LOFAR radio telescope to
look for transients on a 24/7 basis. This document describes a generic
overview of the system and how to configure it for usage. The pipeline is
built upon the Pelican Framework [#]_.

Pipeline Structure
==================

The AARTFAAC Correlator will produce (up to) several hundred independent
frequency channels. These will be supplied to the AARTFAAC imaging system at
(approximately) one second intervals over multiple TCP connections (or
"streams").

Data arriving from the correlator is ingested by the *server*. The server
groups the data into "subbands", which consist of arbitrary (but see the
Limitations_) groups of channels which will be processed together to
form a single image. Each subband is processed independently of the others.

One or more *pipelines* perform the flagging, calibration and imaging. When a
pipeline is idle, it polls the server for the next "chunk" of data,
corresponding to a single integration of a particular subband. The
pipeline processes this data, ultimately writing out the data product to disk,
then returns to ask the server for more work.

For testing and commissioning purposes, it is convenient to be able to process
pre-correlated visibilities stored on disk. This task is handled by the
*emulator*, which reads a MeasurementSet from disk and streams it to the server
in the same way as if it had just been produced by the correlator.

Each server, pipeline or emulator in the system is a separate process: an
instance of the executables ``aartfaac-server``, ``aartfaac-pipeline`` or
``aartfaac-emulator``, respectively.

Data Formats
============

From Correlator to Server
-------------------------

The server is the redistribution centre of the system. One or more incomming
streams [#]_ connect to the server and send the data over TCP/IP. Next, these
streams are restructured into chunks via the StreamChunkers and send to the
available pipelines. The server expects the following header on the incomming
stream.

+------------+------+------------------+
| **Header**        | **Size (bytes)** |
+------------+------+------------------+
| magic      | pad0 | 8                |
+------------+------+------------------+
| start time        | 8                |
+------------+------+------------------+
| end time          | 8                |
+------------+------+------------------+
| pad1              | 488              |
+------------+------+------------------+

Where the magic is defined as 4 byte integer ``0x3B98F002`` and the start/end
times as doubles.  The final padding ``pad1`` ensures a 512 byte header.

When an incomming stream connects to the server sends 512 bytes
of data, the server does the following:

#. Parse the header.
#. Check for the correct magic.
#. Read a precomputed number of bytes [#]_ from the stream and restructure them
   into chunks according to the subbands defined in the xml. In this case 2
   chunks, one with channels 1-7 and the other with channels 57-63.
#. Send each chunk of to an available pipeline.
#. Wait for the next 512 bytes to be ready and go to step 1.

Visibility Output
-----------------

Pipelines may be configured to store calibrated visibilities in the CASA
MeasurementSet format.

.. todo:: Reference to MeasurementSet definition for LOFAR.

Image Output
------------

Pipelines may be configured to output images in either CASA Table format or as
TIFF files.

.. todo:: Document metadata provided in images.

.. todo:: Reference to LOFAR image format definition.

Pipeline Configuration
======================

All three components are configured by means of an XML file. This file should
be UTF-8 encoded and should comply with the ``pelican`` document type
definition. The XML document consists of a ``<configuration />`` element with
the attribute ``version="1.0"``. The contents of the ``<configuration />`` are
specific to the particular pipeline component.

That is, the configuration file should look broadly like:

.. code-block:: xml

   <?xml version="1.0" encoding="UTF-8"?>
   <!DOCTYPE pelican>

   <configuration version="1.0">
     <!-- Component specific configuration -->
   </configuration>

Each of the executables takes the path to the configuration file as a
(non-optional) command line argument.

Template versions of all of the configuration files are place in
``${PREFIX}/share/aartfaac/xml`` when the imaging system is installed.

Emulator
--------

In the case of ``aartfaac-emulator``, the ``<configuration />`` contains one
or more ``<StreamEmulator />`` elements according to the following pattern:

.. code-block:: xml

   <StreamEmulator     name="O1">
       <connection     host="127.0.0.1" port="4100" />
       <measurementset name="/path/to/data.MS" />
       <emulator       packetInterval="0" />
   </StreamEmulator>

The following attributes may be specified:

``<StreamEmulator name />``
  An arbitrary name by which to refer to this emulator. Note that, unless an
  alternative name is specified on the command line, at least one
  ``<StreamEmualtor />`` with the name ``O1`` must be defined, and will be
  used by default.

``<connection host />``, ``connection port/>``
  Host name and TCP port of the server to which to send data.

``<measurementset name />``
  Path to MeasurementSet from which to read data.

``<emulator packetInterval />``
  The time interval in microseconds between packets sent by the emulator.

``aartfaac-emulator`` takes an optional second command line argument which
specifies the name of the ``<StreamEmulator />`` to use. If this name is not
specified, the value of ``O1`` is assumed. A ``<StreamEmulator />`` with the
given name must be defined in the configuration file.

``aartfaac-emulator`` will exit once all the data in the MeasurementSet has
been transmitted.

Server
------

The ``aartfaac-server`` ``<configuration />`` contains a single ``<server />``
element. The ``<server />`` contains ``<buffers />`` and ``<chunkers />``
elements; ``<buffers />`` contains a single ``<StreamBlob />``, while
``<chunkers />`` contains one or more ``<StreamChunkers />``.

Each ``<StreamChunker />`` corresponds to a separate TCP stream from the
correlator (or, alternatively, to the output of a single emulator). Each
chunker listens on a separate TCP port and receives its own, independent,
selection of channels.

.. todo:: Check the above for correctness!

.. code-block:: xml

  <configuration version="1.0">
    <server>
      <buffers>
        <StreamBlob>
          <buffer maxSize="1024" maxChunkSize="1024"/>
        </StreamBlob>
      </buffers>

      <chunkers>
        <StreamChunker name="Stream1">
          <data type="StreamBlob"/>
          <stream subbands="1-7,57-63" numChannels="64" frequency="59669494.628906" width="3051.757812"/>
          <connection host="127.0.0.1" port="4100"/>
        </StreamChunker>
      </chunkers>
    </server>
  </configuration>

The following attributes may be specified by the end user:

``<buffer maxSize />``, ``<buffer maxChunkSize />``
  The maximum number of chunks in byts and the maximum number of bytes per
  chunk, respectively. After these thresholds are exceed, the server will
  start discarding old data to make space for new.

``<StreamChunker name />``
  An arbitrary string to identify the chunker.

``<stream subbands />``
  A series of comma-separated ``start`` - ``end`` (inclusive) channel ranges.
  Each range of channels is regarded as a "subband", which is sent to a single
  pipeline and results in a single output image. Note that the subbands are
  0-indexed (i.e., the first subband is labelled ``0``).

``<stream numChannels />``
  The total number of channels in the stream.

``<stream frequency />``
  The central frequency of the first channel in the stream in Hz.

``<stream width />``
  The width of the channels in the stream in Hz. Note that channels are
  assumed to all be of equal width.

``<connection host />``, ``<connection port />``
  The TCP host and port to which the server will bind to listen for incoming
  data. Specifying a host of ``0.0.0.0`` will cause the server to bind to all
  available interfaces.

.. todo:: Check: is the subband definition inclusive?

.. todo:: Check: is the frequency the *central* frequency or the *edge*
          frequency?

Pipeline
--------

The pipeline is the workhorse of the system. When it receives a chunk from the
server, it restructures it into a StreamBlob via the StreamAdapter after which
it can process the data through a series of modules, which normally include
flagging, calibration and imaging..

``aartfaac-pipeline`` takes a configuration file which defines a ``<pipeline
/>`` element. The pipeline contains a list of ``<clients />``: normally only
one, which connects to the Server_. It specifies a list of ``<modules />``,
which configure the data processing to be done on the data received. Finally,
it specifies how the resultant data products are stored in the ``<output />``
element.

Note that the ``<module />`` definitions do not define *what* processing is
performed on the data: the pipeline code explicitly flags, then calibrates,
then images. Here we define only the configuration parameters used during
those steps.

.. todo:: Check: is that correct?

An example pipeline configuration is as follows:

.. code-block:: xml

  <configuration version="1.0">
    <pipeline monport="4200" threads="1">
      <clients>
        <PelicanServerClient>
          <server host="127.0.0.1" port="2000"/>
          <data type="StreamBlob" adapter="StreamAdapter"/>
        </PelicanServerClient>
      </clients>

      <adapters>
        <StreamAdapter>
          <!-- No specific settings -->
        </StreamAdapter>
      </adapters>

      <modules>
        <Flagger>
          <deviation multiplier="4.0"/>
        </Flagger>

        <Calibrator>
          <positrf path="@CMAKE_INSTALL_PREFIX@/share/aartfaac/antennasets/lba_outer.dat"/>
        </Calibrator>

        <Imager>
          <positrf path="@CMAKE_INSTALL_PREFIX@/share/aartfaac/antennasets/lba_outer.dat"/>
        </Imager>
      </modules>

      <output>
        <streamers>
          <TiffStorage active="false">
            <output path="/data"/>
          </TiffStorage>
          <CasaImageStorage active="true">
            <output path="/data"/>
          </CasaImageStorage>
        </streamers>
        <dataStreams>
          <stream name="post" listeners="TiffStorage"/>
          <stream name="post" listeners="CasaImageStorage"/>
        </dataStreams>
      </output>
    </pipeline>
  </configuration>

The following parameters may be configured:

``<pipeline monport />``
  The pipeline also allows for listening on a monitoring port, ``monport``,
  which, once connected shows realtime diagnostics of the data being processed
  in ascii [#]_.

``<pipeline threads />``
  Sets the number of threads of execution used by the pipeline. Channels may
  be flagged and calibrated concurrently, so that multi-core architectures can
  be exploited, but note that all data in the subband is combined and imaged
  by a single thread.

``<PelicanServerClient host />``, ``<PelicanServerClient port />``
  The host and port of the server from which to fetch data.

``<deviation multiplier />``
  The maximum deviation an antenna may have from the variance of all antennas.

``<positrf path />``
  The full path to a file providing the IRTF positions of the antennae
  currently being correlated. Note that ordering of this file must correspond
  to the ordering of data being produced by the correlator.

``<TiffStorage active />``
  If true, store output images in TIFF format.

``<CasaImageStorage active />``
  If true, store output images as CASA tables.

``<output path />``
  Directory into which output images will be written. Every image will be
  written into this path under a unique filename.

.. todo:: To which interface does monport bind?

.. todo:: deviation multiplier requires more explanation. We don't have a
          value per antenna; we have a value per baseline!

.. todo:: Specify how ordering of antennae must is determined.

.. todo:: How are image filenames generated?

Note that currently the only output methods supported write files to disk. In
future, we expect to stream output image data directly over the network to the
transients detection pipeline.

.. todo:: Don't we also support output of calibrated visibilities? Where is
          that configured?

Limitations
===========

It is not (currently) possible to specify a subband which contains channels
from different streams. This is issue #30.

Glossary
========

AARTFAAC
  Amsterdam-Astron Radio Transients Facility And Analysis Center.

ACM
  Array Correlation Matrix. A 288x288 matrix consisting of the visibilities
  layed out in the antenna structure.

MeasurementSet
  An AIPS++/CASA/casacore Table containing visibility data.

StreamChunker
  The function of the chunker is to take an incoming data stream and turn it
  into suitable size chunks that can be fed into the data adapter. The chunker
  is defined in the server.

StreamBlob
  DataBlobs are simply C++ structures that hold data for use by Pelican
  pipeline modules. They may contain arrays, blocks of memory and/or other
  data, and should provide methods to interact with that data. Their main
  function is to act as an interface between pipeline modules. The streamblob
  contains an ACM for each polarisation.

StreamAdapter
  Adapters are the final components of the data-import chain, and provide a
  mechanism to convert chunks of raw binary data into the data members of a
  Pelican data-blob (a specialised C++ container for holding data used by the
  Pelican pipeline; see below). The most basic function of an adapter is to
  de-serialise chunks of data, although re-ordering and re-factoring of the
  data to a form that is convenient for subsequent pipeline processing may also
  be carried out. Pelican currently provides support for two categories of
  adapters, distinguished by the type of input data chunks they are expected to
  process: these are stream data adapters and service data adapters, which
  operate on the relevant data types.

Subband
  A sequence of channels between 0 and 63.

.. [#] *Pipeline for Extensible, Lightweight Imaging and CAlibratioN*. See https://github.com/pelican/pelican for more information.
.. [#] This can be multiple emulators or the correlator with multiple connections.
.. [#] See https://github.com/aartfaac/imaging/blob/master/src/server/StreamChunker.cpp#L62 for the full details.
.. [#] A webbased interface called Cherimoya will be connected. See https://github.com/gijzelaerr/cherimoya
