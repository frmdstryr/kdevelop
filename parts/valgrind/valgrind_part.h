/*
 *  Copyright (C) 2002 Harald Fernengel <harry@kdevelop.org>   
 */

#ifndef __KDEVPART_VALGRIND_H__
#define __KDEVPART_VALGRIND_H__

#include <qguardedptr.h>
#include <qstring.h>
#include <qstringlist.h>
#include <kdevplugin.h>

class ValgrindWidget;
class KProcess;

class ValgrindPart : public KDevPlugin
{
  Q_OBJECT

public:   
  ValgrindPart( QObject *parent, const char *name, const QStringList & );
  ~ValgrindPart();
  
  void runValgrind( const QString& exec, const QString& parameters, const QString& valExec, const QString& valParameters );

private slots:
  void slotExecValgrind();
  void receivedStdout( KProcess*, char*, int );
  void receivedStderr( KProcess*, char*, int );
  void processExited( KProcess* );
  
private:
  void getActiveFiles();
  void appendMessage( const QString& message );
  KProcess* proc;
  QString currentMessage;
  QString lastPiece;
  QStringList activeFiles;
  int currentPid;
  QGuardedPtr<ValgrindWidget> m_widget;
};


#endif
