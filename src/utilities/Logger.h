#ifndef LOGGER_H
#define LOGGER_H

#include <QtCore>

class Logger {
public:

  static void setLogFileProperties(const QString &fileName,
                                   quint8 maxFiles,
                                   qint64 maxFileSize);

  static void messageHandler(QtMsgType type, const char *msg);
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

  static QString colorize(const QString &msg, Color color);

  static QFile _sFile;
  static bool _sShouldUseColor;

  static quint8 _sMaxFiles;
  static quint8 _sCurFile;
  static qint64 _sMaxFileSize;
  static QString _sFileName;
};

#endif // LOGGER_H
