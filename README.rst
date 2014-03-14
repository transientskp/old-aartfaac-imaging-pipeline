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
`Current Limitations`_) groups of channels which will be processed together to
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
element as follows:

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

The following attributes may be specified:

``<buffer maxSize />``, ``<buffer maxChunkSize />``
  The maximum number of chunks in byts and the maximum number of bytes per
  chunk, respectively. After these thresholds are exceed, the server will
  start discarding old data to make space for new.

.. todo:: finish!

Pipeline
--------

The pipeline is the workhorse of the system. When it receives a chunk from the
server, it restructures it into a StreamBlob via the StreamAdapter after which
it can process the data. The base configuration for the pipeline is located at
``data/xml/configPipeline.xml.in`` and has the following structure.

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

The pipeline consists of three major components, an adapter, modules and output
streamers. As stated the adapter structures the data into a blob such that we
can call useful functions on the data. The modules perform flagging,
calibration and imaging. The flagger requires a ``deviation multiplier`` which
determines the max deviation an antenna may have from the variance of all
antennas. Both the calibrator and imager require the itrf antenna positions for
the current configuration, LBA_OUTER in this case. The output streams send the
processed streams to the defined path. In the future they will send the data
over the network to the TRAP. 

One can also define the number of threads used for each pipeline. Each thread
flags and calibrates a channel in parallel. Its recommended to set this to the
minimal number of channels in a subband.  Finally each pipeline also allows for
listening on a monitoring port ``monport`` which, once connected shows realtime
diagnostics of the data being processed in ascii [#]_.

Current Limitations
===================



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
