#include "threads/mountthread.h"

MountThread::MountThread(QObject *parent): QThread(parent)
{

}

void MountThread::MountNetWorkDrive(QString ip)
{
    QProcess process_mount;
    QString command;

    rootDir = QDir::currentPath();
    rootDir.cdUp();
    rootDir.cdUp();
    rootDir.cdUp();
    QString roStr = rootDir.absolutePath();
    QString roo = roStr + "/" + "shares";

    QByteArray ba = roo.toLatin1();
    const char *shrs = ba.data();
    if (!QDir(shrs).exists())
    {
        QDir().mkdir(shrs);
    }

    QString rip = roo + "/" + ip;
    QByteArray baip = rip.toLatin1();
    const char *ipfile = baip.data();

    if (!QDir(ipfile).exists())
    {
        QDir().mkdir(ipfile);
    }

    command =   "mount -t smbfs ";

    QString usrshr = "//jose:joselon@" + ip + "/motion ";
    QByteArray bausr = usrshr.toLatin1();
    const char *credshare = bausr.data();

    command += credshare;

    QString mount =  roo + "/" + ip;
    QByteArray bamount = mount.toLatin1();
    const char *mountpoint = bamount.data();

    command += mountpoint;

    std::cout << "mount command ::  " << command.toStdString() <<  std::endl;

    process_mount.start(command, QIODevice::ReadOnly);
    process_mount.waitForFinished();

    QString stdout = process_mount.readAllStandardOutput();
    QString stderr = process_mount.readAllStandardError();

    emit SharedMounted(mountpoint);

}

void MountThread::UmountNetWorkDrive()
{
    /*rootDir = QDir::currentPath();

    process_mount.start("umount " + rootDir + "/"  + "shares" + ip);
    process_mount.waitForFinished(-1); // will wait forever until finished

    QString stdout = process_mount.readAllStandardOutput();
    QString stderr = process_mount.readAllStandardError();

    emit ShareUmounted();*/
}
