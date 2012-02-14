#ifndef LOGGER_H
#define LOGGER_H

#include <QtCore>

class Logger {
public:

  static void setLogFileProperties(const QString &inFileName,
                                   quint8 inMaxFiles,
                                   qint64 inMaxFileSize);

  static void messageHandler(QtMsgType inType, const char *inMsg);
  static bool shouldUseColor();

private:
  enum Color {
    BLACK   = 30,
    RED     = 31,
    GREEN   = 32,
    YELLOW  = 33,
    BLUE    = 34,
    MAGENTA = 35,
    CYAN    = 36,
    WHITE   = 37
  };

  static QString colorize(const QString &inMsg, Color inColor);

  static QFile sFile;
  static bool sShouldUseColor;

  static quint8 sMaxFiles;
  static quint8 sCurrentFile;
  static qint64 sMaxFileSize;
  static QString sFileName;
};

#endif // LOGGER_H
