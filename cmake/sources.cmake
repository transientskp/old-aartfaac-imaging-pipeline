# === All *.{cpp,h} files

# === Emulator sources
set (EMULATOR_SOURCES
  src/Constants.h
  src/emulator/main.cpp
  src/emulator/stream/StreamPacket.h
  src/emulator/stream/StreamEmulator.cpp
  src/utilities/Logger.cpp
  src/utilities/Utils.cpp
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
  src/utilities/NMSMax.h
  src/utilities/Utils.cpp
  src/utilities/AntennaPositions.cpp
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
)

# === Test sources
set (TESTS emulatortest pipelinetest servertest utilitiestest)

set (emulatortest_SOURCES
  src/Constants.h
  src/emulator/test/main.cpp
  src/emulator/test/UniboardEmulatorTest.cpp
  src/emulator/stream/StreamEmulator.cpp
  src/utilities/Utils.cpp
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
)

set (servertest_SOURCES
  src/server/test/main.cpp
)

set (utilitiestest_SOURCES
  src/utilities/test/main.cpp
  src/utilities/Utils.cpp
  src/utilities/test/UtilitiesTest.cpp
)

set (SCRIPTS
  scripts/start_aartfaac.py
  scripts/start_emulator.py
  scripts/start_server.py
  scripts/start_pipeline.py
)

set (DATAFILES
  data/antennasets/lba_outer.dat
  data/antennasets/lba_inner.dat
)
