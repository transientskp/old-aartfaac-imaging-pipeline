#include "Logger.h"

#ifdef ENABLE_SYSLOG
#include <syslog.h>
#else
#define syslog(a,b,c)
#define openlog(a,b,c)
#define closelog()
#endif

#include <stdlib.h>
#include <unistd.h>

#include <iostream>
#include <QDataStream>
#include <QtCore>

QString Logger::sName;

void Logger::open(const QString &inName)
{
  sName = inName;
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
  __pid_t pid = getpid();
  msg = "[" + sName + "-" + QString::number(pid) + "] " + msg;

  switch (inType)
  {
  case QtDebugMsg:
    {
      syslog(LOG_INFO, "[INFO] %s", inMsg);

      if (shouldUseColor())
        std::cout << qPrintable(colorize(msg, GREEN)) << std::endl;
      else
        std::cout << "[INFO] " << qPrintable(msg) << std::endl;

      break;
    }
  case QtWarningMsg:
    {
      syslog(LOG_WARNING, "[WARNING] %s", inMsg);

      if (shouldUseColor())
        std::cout << qPrintable(colorize(msg, YELLOW)) << std::endl;
      else
        std::cout << "[WARNING] " << qPrintable(msg) << std::endl;

      break;
    }
  case QtCriticalMsg:
    {
      syslog(LOG_CRIT, "[CRITICAL] %s", inMsg);

      if (shouldUseColor())
        std::cerr << qPrintable(colorize(msg, RED)) << std::endl;
      else
        std::cerr << "[CRITICAL] " << qPrintable(msg) << std::endl;

      break;
    }
  case QtFatalMsg:
    {
      syslog(LOG_EMERG, "[FATAL] %s", inMsg);

      if (shouldUseColor())
        std::cerr << qPrintable(colorize(msg, RED)) << std::endl;
      else
        std::cerr << "[FATAL] " << qPrintable(msg) << std::endl;

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
  const char *term = getenv("TERM");

  if (term == NULL || !isatty(1))
    return false;

  const bool term_supports_color =
    strcmp(term, "xterm") >= 0 ||
    strcmp(term, "xterm-color") >= 0 ||
    strcmp(term, "xterm-256color") >= 0 ||
    strcmp(term, "screen") >= 0 ||
    strcmp(term, "linux") >= 0 ||
    strcmp(term, "cygwin") >= 0;

  return term_supports_color;
}
