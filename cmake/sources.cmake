# === All *.{cpp,h} files

# === Emulator sources
set(EMULATOR_SOURCES
  src/emulator/main.cpp
  src/emulator/UniboardEmulator.cpp
  src/utilities/Logger.cpp
)

# === Server sources
set(SERVER_SOURCES
  src/server/main.cpp
  src/server/DataChunker.cpp
  src/utilities/Logger.cpp
)

# === Pipeline sources
set(PIPELINE_SOURCES
  src/pipeline/main.cpp
  src/utilities/Logger.cpp
)
