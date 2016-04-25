# === All *.{cpp,h} files

# === Pipeline sources
set (PIPELINE_SOURCES
  src/Constants.h
  src/Macros.h
  src/pipeline/main.cpp
  src/pipeline/StreamBlob.cpp
  src/pipeline/StreamAdapter.cpp
  src/pipeline/UniboardPipeline.cpp
  src/pipeline/DirectDataClient.cpp
  src/pipeline/StreamChunker.cpp
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

# === Test sources
set (TESTS pipelinetest utilitiestest)

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

set (utilitiestest_SOURCES
  src/utilities/test/main.cpp
  src/utilities/Utils.cpp
  src/utilities/test/UtilitiesTest.cpp
)

set (SCRIPTS
  scripts/start_aartfaac.py
  scripts/start_pipeline.py
)

set (DATAFILES
  data/antennasets/lba_outer.dat
  data/antennasets/lba_inner.dat
)
