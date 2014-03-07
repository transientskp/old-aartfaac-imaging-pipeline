# === All *.{cpp,h} files

# === Emulator sources
set (EMULATOR_SOURCES
  src/Constants.h
  src/emulator/main.cpp
  src/emulator/stream/StreamPacket.h
  src/emulator/stream/StreamEmulator.cpp
  src/utilities/Logger.cpp
)

# === Pipeline sources
set (PIPELINE_SOURCES
  src/Constants.h
  src/Macros.h
  src/pipeline/main.cpp
  src/pipeline/StreamBlob.cpp
  src/pipeline/StreamAdapter.cpp
  src/pipeline/UniboardPipeline.cpp
  src/pipeline/modules/flagger/Flagger.cpp
  src/pipeline/modules/calibrator/Calibrator.cpp
  src/pipeline/modules/imager/Imager.cpp
  src/pipeline/modules/output/TiffStorage.cpp
  src/pipeline/modules/output/CasaImageStorage.cpp
  src/pipeline/modules/output/Visibilities.cpp
  src/utilities/NMSMax.h
  src/utilities/Utils.cpp
  src/utilities/AntennaPositions.cpp
  src/utilities/monitoring/Thread.cpp
  src/utilities/monitoring/TcpStream.cpp
  src/utilities/monitoring/TcpAcceptor.cpp
  src/utilities/monitoring/Server.cpp
  src/utilities/Logger.cpp
)

# === Server sources
set (SERVER_SOURCES
  src/Constants.h
  src/server/main.cpp
  src/server/StreamChunker.cpp
  src/emulator/stream/StreamPacket.h
  src/utilities/Utils.cpp
  src/utilities/Logger.cpp
  src/utilities/monitoring/Thread.cpp
  src/utilities/monitoring/TcpStream.cpp
  src/utilities/monitoring/TcpAcceptor.cpp
  src/utilities/monitoring/Server.cpp
)

# === Regridder sources
set (REGRIDDER_SOURCES
  src/regridder/regrid.cpp
)

# === Test sources
set (TESTS emulatortest pipelinetest servertest utilitiestest)

set (emulatortest_SOURCES
  src/Constants.h
  src/emulator/test/main.cpp
  src/emulator/test/UniboardEmulatorTest.cpp
  src/emulator/stream/StreamEmulator.cpp
)

set (pipelinetest_SOURCES
  src/pipeline/test/main.cpp
  src/pipeline/test/FlaggerTest.cpp
  src/pipeline/test/CalibratorTest.cpp
  src/pipeline/test/ImagerTest.cpp
  src/pipeline/StreamBlob.cpp
  src/pipeline/modules/imager/Imager.cpp
  src/pipeline/modules/calibrator/Calibrator.cpp
  src/pipeline/modules/flagger/Flagger.cpp
  src/utilities/Utils.cpp
  src/utilities/AntennaPositions.cpp
  src/utilities/NMSMax.h
  src/utilities/monitoring/Thread.cpp
  src/utilities/monitoring/TcpStream.cpp
  src/utilities/monitoring/TcpAcceptor.cpp
  src/utilities/monitoring/Server.cpp
)

set (servertest_SOURCES
  src/server/test/main.cpp
  src/utilities/monitoring/Thread.cpp
  src/utilities/monitoring/TcpStream.cpp
  src/utilities/monitoring/TcpAcceptor.cpp
  src/utilities/monitoring/Server.cpp
)

set (utilitiestest_SOURCES
  src/utilities/test/main.cpp
  src/utilities/test/UtilitiesTest.cpp
  src/utilities/monitoring/Thread.cpp
  src/utilities/monitoring/TcpStream.cpp
  src/utilities/monitoring/TcpAcceptor.cpp
  src/utilities/monitoring/Server.cpp
)

set (SCRIPTS
  scripts/aartfaac-system-start.py
  scripts/start-emulator.py
  scripts/start-server.py
  scripts/start-pipeline.py
)

set (DATAFILES
  data/xml/emulatorConfig.xml.in
  data/xml/serverConfig.xml.in
  data/xml/pipelineConfig.xml.in
  data/antennasets/lba_outer.data.in
)
