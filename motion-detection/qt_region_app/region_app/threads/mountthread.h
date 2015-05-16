#ifndef MOUNTTHREAD_H
#define MOUNTTHREAD_H

#include <QDir>
#include <QProcess>
#include <QStringList>
#include <QString>
#include <QDir>
#include <QThread>
#include <QIODevice>

using namespace std;

class MountThread : public QThread
{
    Q_OBJECT
public:
    explicit MountThread(QObject *parent =0);
    void MountNetWorkDrive(QString ip);
    void UmountNetWorkDrive();

private:
    QDir rootDir;
    QProcess process_mount;
    QProcess process_mkdir;

signals:
   void SharedMounted(QString folder);

};

#endif // MOUNTTHREAD_H
