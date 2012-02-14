#include "Logger.h"

#include <cstdlib>
#include <iostream>
#include <QFile>
#include <QDataStream>

QFile Logger::sFile;
QString Logger::sFileName;
bool Logger::sShouldUseColor = shouldUseColor();
quint8 Logger::sMaxFiles;
qint64 Logger::sMaxFileSize;
quint8 Logger::sCurrentFile;

void Logger::setLogFileProperties(const QString &inFileName,
                                  const quint8 inMaxFiles,
                                  const qint64 inMaxFileSize)
{
  sMaxFiles = inMaxFiles;
  sMaxFileSize = inMaxFileSize;
  sFileName = inFileName;
  sCurrentFile = 0;
  sFile.setFileName(QString::number(sCurrentFile) + "-" + sFileName);
  sFile.open(QIODevice::WriteOnly);
}

void Logger::messageHandler(QtMsgType inType, const char *inMsg)
{
  QDateTime date_time = QDateTime::currentDateTime();
  QString msg(inMsg);

  switch (inType)
  {
  case QtDebugMsg:
    sFile.write("[DEBUG] ");
    if (sShouldUseColor)
      std::cout << qPrintable(colorize(msg, GREEN)) << std::endl;
    else
      std::cout << "[DEBUG] " << inMsg << std::endl;
    break;
  case QtWarningMsg:
    sFile.write("[WARNING] ");
    if (sShouldUseColor)
      std::cout << qPrintable(colorize(msg, YELLOW)) << std::endl;
    else
      std::cout << "[WARNING] " << inMsg << std::endl;
    break;
  case QtCriticalMsg:
    sFile.write("[CRITICAL] ");
    if (sShouldUseColor)
      std::cerr << qPrintable(colorize(msg, RED)) << std::endl;
    else
      std::cerr << "[CRITICAL] " << inMsg << std::endl;
    break;
  case QtFatalMsg:
    sFile.write("[FATAL] ");
    if (sShouldUseColor)
      std::cerr << qPrintable(colorize(msg, RED)) << std::endl;
    else
      std::cerr << "[FATAL] " << inMsg << std::endl;
    break;
  }

  sFile.write(date_time.toString("(dd/MM/yy hh:mm) ").toAscii());
  sFile.write(inMsg);
  sFile.write("\n");
  sFile.flush();

  if (sFile.size() > sMaxFileSize)
  {
    sFile.close();
    sCurrentFile = (sCurrentFile + 1) % sMaxFiles;
    sFile.setFileName(QString::number(sCurrentFile) + "-" + sFileName);
    sFile.open(QIODevice::WriteOnly);
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
