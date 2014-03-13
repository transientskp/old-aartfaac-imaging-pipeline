Aartfaac Imaging Pipeline
=========================

The Aartfaac Imaging Pipeline is a realtime imaging system for the aartfaac all sky monitor. The ASM uses the main core of the LOFAR radio telescope to look for transients on a 24/7 basis. This document describes a generic overview of the system and howto configure it for usage. The pipeline is build upon the Pelican Framework [#]_. 

The overal system can be split up into three main programs through which data flows. The emulator, the server and the pipeline which we will explain in more detail below.

Emulator
--------

The emulator...

Server
------

The server...

Pipeline
--------

The pipeline...


Configurations
==============

Configuration of the imaging system is defined via xml files. Each component of the pipline has its own configuration, giving us three xml files defined in ``data/xml``

Emulator
--------

The base configuration for the Emulator_ has the following structure

.. code-block:: xml

  <configuration version="1.0">
      <StreamEmulator     name="O1">
          <connection     host="127.0.0.1" port="4100" />
          <measurementset name="/path/to/data.MS" />
          <emulator       packetInterval="0" />
      </StreamEmulator>
  </configuration>

Where the ``packetInterval`` defines the time interval in *microseconds* between packets. Furthermore, one can define multiple StreamEmulator blocks with different names and pick one on the command line e.g. ``aartfaac-emulator O2``.


Definitions
===========

AARTFAAC
  Amsterdam-Astron Radio Transients Facility And Analysis Center

ACM
  Array Correlation Matrix. A 288x288 matrix consisting of the visibilities layed out in the antenna structure.

StreamChunker
  The function of the chunker is to take an incoming data stream and turn it into suitable size chunks that can be fed
  into the data adapter. The chunker is defined in the server.

StreamBlob
  DataBlobs are simply C++ structures that hold data for use by Pelican pipeline modules. They may contain arrays,
  blocks of memory and/or other data, and should provide methods to interact with that data. Their main function is
  to act as an interface between pipeline modules. The streamblob contains an ACM for each polarisation.


.. [#] *Pipeline for Extensible, Lightweight Imaging and CAlibratioN*. See https://github.com/pelican/pelican for more information.
