#include "Logger.h"

#include <syslog.h>
#include <stdlib.h>

#include <iostream>
#include <QDataStream>

bool Logger::sShouldUseColor = shouldUseColor();

void Logger::open(const QString &inName)
{
  openlog(qPrintable(inName), LOG_PID | LOG_NDELAY, LOG_LOCAL0);
  syslog(LOG_INFO, "Program started by User %d", getuid ());
}

void Logger::close()
{
  closelog();
}

void Logger::messageHandler(QtMsgType inType, const char *inMsg)
{
  QString msg(inMsg);

  switch (inType)
  {
    case QtDebugMsg:
    {
      syslog(LOG_INFO, "[INFO] %s", inMsg);
      if (sShouldUseColor)
        std::cout << qPrintable(colorize(msg, GREEN)) << std::endl;
      else
        std::cout << "[INFO] " << inMsg << std::endl;
      break;
    }
    case QtWarningMsg:
    {
      syslog(LOG_WARNING, "[WARNING] %s", inMsg);
      if (sShouldUseColor)
        std::cout << qPrintable(colorize(msg, YELLOW)) << std::endl;
      else
        std::cout << "[WARNING] " << inMsg << std::endl;
      break;
    }
    case QtCriticalMsg:
    {
      syslog(LOG_CRIT, "[CRITICAL] %s", inMsg);
      if (sShouldUseColor)
        std::cerr << qPrintable(colorize(msg, RED)) << std::endl;
      else
        std::cerr << "[CRITICAL] " << inMsg << std::endl;
      break;
    }
    case QtFatalMsg:
    {
      syslog(LOG_EMERG, "[FATAL] %s", inMsg);
      if (sShouldUseColor)
        std::cerr << qPrintable(colorize(msg, RED)) << std::endl;
      else
        std::cerr << "[FATAL] " << inMsg << std::endl;
      exit(EXIT_FAILURE);
      break;
    }
  }
}

QString Logger::colorize(const QString &inMsg, Color inColor)
{
  QString output("\033[0;");
  return output + QString::number(inColor) + "m" + inMsg + "\033[0m";
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
