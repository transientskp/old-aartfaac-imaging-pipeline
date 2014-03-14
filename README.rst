Aartfaac Imaging Pipeline
=========================

The Aartfaac Imaging Pipeline is a realtime imaging system for the aartfaac all
sky monitor. The ASM uses the main core of the LOFAR radio telescope to look
for transients on a 24/7 basis. This document describes a generic overview of
the system and howto configure it for usage. The pipeline is build upon the
Pelican Framework [#]_. 

The overal system can be split up into three main programs through which data
flows. The emulator, the server and the pipeline which we will explain in more
detail below.


Emulator
--------

The StreamEmulator reads a measurement set of disk and sends it to the Server_.
Its main goal however, is to emulate the behavior of the correlator developed
at ASTRON.

The base configuration for the emulator is located at
``data/xml/configEmulator.xml.in`` and has the following structure.

.. code-block:: xml

  <configuration version="1.0">
      <StreamEmulator     name="O1">
          <connection     host="127.0.0.1" port="4100" />
          <measurementset name="/path/to/data.MS" />
          <emulator       packetInterval="0" />
      </StreamEmulator>
  </configuration>

Where the ``packetInterval`` defines the time interval in *microseconds*
between packets. Furthermore, one can define multiple StreamEmulator blocks
with different names and pick one on the command line e.g. ``aartfaac-emulator
O2``.


Server
------

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
times as doubles.  The final padding ``pad1`` ensures a 512 byte header. The
base configuration for the server is located at
``data/xml/configServer.xml.in`` and has the following structure.

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

When an incomming stream connects on ``host:port`` to the server and 512 bytes
are available the server does the following.

1. Parse the header
2. Check for the correct magic
3. Read a precomputed number of bytes [#]_ from the stream and restructure them
   into chunks according to the subbands defined in the xml. In this case 2
   chunks, one with channels 1-7 and the other with channels 57-63.
4. Send each chunk of to an available pipeline
5. Wait for the next 512 bytes to be ready and goto step 1

As previously stated, one can connect multiple streams to the server. These are
represented in the xml as StreamChunkers with a unique ``name``. Each stream
gets its own StreamChunker which applies the above. Furthermore, one defines
``maxSize`` and ``maxChunkSize`` in the buffers section to determine the
maximum numer of chunks in bytes and max number of bytes in a chunk
respectively.


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

Definitions
===========

AARTFAAC
  Amsterdam-Astron Radio Transients Facility And Analysis Center.

ACM
  Array Correlation Matrix. A 288x288 matrix consisting of the visibilities
  layed out in the antenna structure.

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
