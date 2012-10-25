#ifndef LOGGER_H
#define LOGGER_H

#include <QtCore>

class Logger
{
public:
  static void messageHandler(QtMsgType inType, const char *inMsg);
  static bool shouldUseColor();
  static void open(const QString &inName);
  static void close();

private:
  enum Color
  {
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
  static bool sShouldUseColor;
};

#endif // LOGGER_H
