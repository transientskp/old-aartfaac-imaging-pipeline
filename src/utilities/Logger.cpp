#include "Logger.h"

#include <cstdlib>
#include <iostream>
#include <QFile>
#include <QDataStream>

QFile Logger::_sFile;
bool Logger::_sShouldUseColor = shouldUseColor();

void Logger::setFileName(const QString &fileName)
{
  _sFile.setFileName(fileName);
  _sFile.open(QIODevice::WriteOnly);
}

void Logger::messageHandler(QtMsgType type, const char *msg)
{
  QDateTime dateTime = QDateTime::currentDateTime();
  QString message(msg);

  switch (type)
  {
  case QtDebugMsg:
    _sFile.write("[DEBUG] ");
    if (_sShouldUseColor)
      std::cout << qPrintable(colorize(message, GREEN)) << std::endl;
    else
      std::cout << "[DEBUG] " << msg << std::endl;
    break;
  case QtWarningMsg:
    _sFile.write("[WARNING] ");
    if (_sShouldUseColor)
      std::cout << qPrintable(colorize(message, YELLOW)) << std::endl;
    else
      std::cout << "[WARNING] " << msg << std::endl;
    break;
  case QtCriticalMsg:
    _sFile.write("[CRITICAL] ");
    if (_sShouldUseColor)
      std::cerr << qPrintable(colorize(message, RED)) << std::endl;
    else
      std::cerr << "[CRITICAL] " << msg << std::endl;
    break;
  case QtFatalMsg:
    _sFile.write("[FATAL] ");
    if (_sShouldUseColor)
      std::cerr << qPrintable(colorize(message, RED)) << std::endl;
    else
      std::cerr << "[FATAL] " << msg << std::endl;
    break;
  }

  _sFile.write(dateTime.toString("(dd/MM/yy hh:mm) ").toAscii());
  _sFile.write(msg);
  _sFile.write("\n");
  _sFile.flush();
}

QString Logger::colorize(const QString &msg, Color color)
{
  QString output("\033[0;");
  return output + QString::number(color) + "m" + msg + "\033[0m";
}

bool Logger::shouldUseColor()
{
#if WIN32
  // On Windows the TERM variable is usually not set, but the
  // console there does support colors.
  return true;
#else
  // On non-Windows platforms, we rely on the TERM variable.
  const char* term = getenv("TERM");

  if (term == NULL)
    return false;

  const bool term_supports_color =
    strcmp(term, "xterm") >= 0 ||
    strcmp(term, "xterm-color") >= 0 ||
    strcmp(term, "xterm-256color") >= 0 ||
    strcmp(term, "screen") >= 0 ||
    strcmp(term, "linux") >= 0 ||
    strcmp(term, "cygwin") >= 0;

  return term_supports_color;
#endif  // WIN32
}
