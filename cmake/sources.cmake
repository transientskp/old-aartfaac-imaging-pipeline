# === All *.{cpp,h} files

# === Emulator sources
set(STREAM_EMULATOR_SOURCES
  src/emulators/stream/main.cpp
  src/emulators/stream/StreamUdpPacket.h
  src/emulators/stream/StreamEmulator.cpp
  src/utilities/halfprecision.c
  src/utilities/Logger.cpp
)

set(SERVICE_EMULATOR_SOURCES
  src/emulators/service/main.cpp
  src/emulators/service/ServiceUdpPacket.h
  src/emulators/service/ServiceEmulator.cpp
  src/utilities/Logger.cpp
)

# === Pipeline sources
set(PIPELINE_SOURCES
  src/pipeline/main.cpp
  src/pipeline/UniboardDataBlob.cpp
  src/pipeline/UniboardAdapter.cpp
  src/pipeline/UniboardPipeline.cpp
  src/pipeline/modules/output/TiffStorage.cpp
  src/pipeline/modules/output/CasaImageStorage.cpp
  src/pipeline/modules/calibrator/Calibrator.cpp
  src/pipeline/modules/imager/Imager.cpp
  src/utilities/halfprecision.c
  src/utilities/Utils.cpp
  src/utilities/Logger.cpp
)

# === Server sources
set(SERVER_SOURCES
  src/server/main.cpp
  src/server/StreamUniboardChunker.cpp
  src/emulators/service/ServiceUdpPacket.h
  src/emulators/stream/StreamUdpPacket.h
  src/utilities/Utils.cpp
  src/utilities/Logger.cpp
)

# === Test sources
set (TESTS emulatortest pipelinetest servertest)

set (emulatortest_SOURCES
  src/emulators/test/main.cpp
  src/emulators/test/UniboardEmulatorTest.cpp
  #src/emulators/service/ServiceEmulator.cpp
  src/emulators/stream/StreamEmulator.cpp
  src/utilities/halfprecision.c
)

set (pipelinetest_SOURCES
  src/pipeline/test/main.cpp
)

set (servertest_SOURCES
  src/server/test/main.cpp
)
