#ifndef LOGGER_H
#define LOGGER_H

#include <QtCore>

class Logger {
public:

  static void setFileName(const QString &fileName);
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
  static QString colorize(const QString &msg, Logger::Color color);

  static QFile _sFile;
  static bool _sShouldUseColor;
};

#endif // LOGGER_H
