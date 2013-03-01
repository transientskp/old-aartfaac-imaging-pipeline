# === All *.{cpp,h} files

# === Emulator sources
set (EMULATOR_SOURCES
  src/Constants.h
  src/emulator/main.cpp
  src/emulator/stream/StreamUdpPacket.h
  src/emulator/stream/StreamEmulator.cpp
  src/emulator/service/ServiceUdpPacket.h
  src/emulator/service/ServiceEmulator.cpp
  src/utilities/Logger.cpp
)

# === Pipeline sources
set (PIPELINE_SOURCES
  src/Constants.h
  src/pipeline/main.cpp
  src/pipeline/StreamBlob.cpp
  src/pipeline/StreamAdapter.cpp
  src/pipeline/ServiceAdapter.cpp
  src/pipeline/ServiceBlob.cpp
  src/pipeline/UniboardPipeline.cpp
  src/pipeline/modules/flagger/Flagger.cpp
  src/pipeline/modules/calibrator/Calibrator.cpp
  src/pipeline/modules/imager/Imager.cpp
  src/pipeline/modules/output/TiffStorage.cpp
  src/pipeline/modules/output/CasaImageStorage.cpp
  src/pipeline/modules/output/Visibilities.cpp
  src/utilities/Utils.cpp
  src/utilities/UVWParser.cpp
  src/utilities/Logger.cpp
)

# === Server sources
set (SERVER_SOURCES
  src/Constants.h
  src/server/main.cpp
  src/server/StreamChunker.cpp
  src/server/ServiceChunker.cpp
  src/emulator/service/ServiceUdpPacket.h
  src/emulator/stream/StreamUdpPacket.h
  src/utilities/Utils.cpp
  src/utilities/Logger.cpp
)

# === Test sources
set (TESTS emulatortest pipelinetest servertest utilitiestest)

set (emulatortest_SOURCES
  src/Constants.h
  src/emulator/test/main.cpp
  src/emulator/test/UniboardEmulatorTest.cpp
  src/emulator/stream/StreamEmulator.cpp
  src/emulator/service/ServiceEmulator.cpp
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
  src/utilities/UVWParser.cpp
)

set (servertest_SOURCES
  src/server/test/main.cpp
)

set (utilitiestest_SOURCES
  src/utilities/test/main.cpp
  src/utilities/test/UtilitiesTest.cpp
)
