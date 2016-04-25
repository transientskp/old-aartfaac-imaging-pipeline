#include "Logger.h"

#include <stdlib.h>
#include <unistd.h>

#include <iostream>
#include <QDataStream>
#include <QtCore>

QString Logger::sName;

void Logger::open(const QString &inName)
{
  sName = inName;
}

void Logger::close()
{
}

void Logger::messageHandler(QtMsgType inType, const char *inMsg)
{
  QString msg(inMsg);
  __pid_t pid = getpid();
  msg = QDateTime::currentDateTimeUtc().toString("yyyy-MM-dd hh:mm:ss") + " " + QString::number(pid) + "] " + inMsg;

  switch (inType)
  {
  case QtDebugMsg:
    {
      if (shouldUseColor())
        std::cout << qPrintable(colorize(msg, GREEN)) << std::endl;
      else
        std::cout << "II " << qPrintable(msg) << std::endl;

      break;
    }
  case QtWarningMsg:
    {
      if (shouldUseColor())
        std::cout << qPrintable(colorize(msg, YELLOW)) << std::endl;
      else
        std::cout << "WW " << qPrintable(msg) << std::endl;

      break;
    }
  case QtCriticalMsg:
    {
      if (shouldUseColor())
        std::cerr << qPrintable(colorize(msg, RED)) << std::endl;
      else
        std::cerr << "EE " << qPrintable(msg) << std::endl;

      break;
    }
  case QtFatalMsg:
    {
      if (shouldUseColor())
        std::cerr << qPrintable(colorize(msg, RED)) << std::endl;
      else
        std::cerr << "FF " << qPrintable(msg) << std::endl;
      exit(EXIT_FAILURE);
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
