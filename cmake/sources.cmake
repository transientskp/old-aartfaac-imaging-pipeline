# === All *.{cpp,h} files

# === Metadata sources
set(METADATA_SOURCES
  src/metadata/main.cpp
  src/metadata/MetaData.cpp
  src/utilities/Logger.cpp
)

# === Emulator sources
set(EMULATOR_SOURCES
  src/emulator/main.cpp
  src/emulator/stream/StreamUdpPacket.h
  src/emulator/stream/StreamEmulator.cpp
  src/emulator/service/ServiceUdpPacket.h
  src/emulator/service/ServiceEmulator.cpp
  src/utilities/halfprecision.c
  src/utilities/Logger.cpp
)

# === Pipeline sources
set(PIPELINE_SOURCES
  src/pipeline/main.cpp
  src/pipeline/StreamBlob.cpp
  src/pipeline/StreamAdapter.cpp
  src/pipeline/ServiceAdapter.cpp
	src/pipeline/ServiceBlob.cpp
  src/pipeline/UniboardPipeline.cpp
  src/pipeline/modules/output/TiffStorage.cpp
  src/pipeline/modules/output/CasaImageStorage.cpp
  src/pipeline/modules/output/Visibilities.cpp
  src/pipeline/modules/calibrator/Calibrator.cpp
  src/pipeline/modules/imager/Imager.cpp
  src/utilities/halfprecision.c
  src/utilities/Utils.cpp
  src/utilities/Logger.cpp
)

# === Server sources
set(SERVER_SOURCES
  src/server/main.cpp
  src/server/StreamChunker.cpp
  src/server/ServiceChunker.cpp
  src/emulator/service/ServiceUdpPacket.h
  src/emulator/stream/StreamUdpPacket.h
  src/utilities/Utils.cpp
  src/utilities/Logger.cpp
)

# === Test sources
set (TESTS emulatortest pipelinetest servertest)

set (emulatortest_SOURCES
  src/emulator/test/main.cpp
  src/emulator/test/UniboardEmulatorTest.cpp
  src/emulator/stream/StreamEmulator.cpp
  src/emulator/service/ServiceEmulator.cpp
  src/utilities/halfprecision.c
)

set (pipelinetest_SOURCES
  src/pipeline/test/main.cpp
)

set (servertest_SOURCES
  src/server/test/main.cpp
)
