/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:    LZ <zhou.lu@archermind.com>
*
* Maintainer:  LZ <zhou.lu@archermind.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include "logfileparser.h"
#include "journalwork.h"
#include "sharedmemorymanager.h"
#include "utils.h"

#include <DMessageManager>

#include <QDateTime>
#include <QDebug>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QMessageBox>
#include <QProcess>
#include <QtConcurrent>

int journalWork::thread_index = 0;
int JournalBootWork::thread_index = 0;
DWIDGET_USE_NAMESPACE

LogFileParser::LogFileParser(QWidget *parent)
    : QObject(parent)
{
    m_dateDict.clear();
    m_dateDict.insert("Jan", "1月");
    m_dateDict.insert("Feb", "2月");
    m_dateDict.insert("Mar", "3月");
    m_dateDict.insert("Apr", "4月");
    m_dateDict.insert("May", "5月");
    m_dateDict.insert("Jun", "6月");
    m_dateDict.insert("Jul", "7月");
    m_dateDict.insert("Aug", "8月");
    m_dateDict.insert("Sep", "9月");
    m_dateDict.insert("Oct", "10月");
    m_dateDict.insert("Nov", "11月");
    m_dateDict.insert("Dec", "12月");

    // TODO::
    m_levelDict.insert("Warning", WARN);
    m_levelDict.insert("Debug", DEB);
    qRegisterMetaType<QList<LOG_MSG_KWIN> > ("QList<LOG_MSG_KWIN>");
    qRegisterMetaType<QList<LOG_MSG_XORG> > ("QList<LOG_MSG_XORG>");
    qRegisterMetaType<QList<LOG_MSG_DPKG> > ("QList<LOG_MSG_DPKG>");
    qRegisterMetaType<QList<LOG_MSG_BOOT> > ("QList<LOG_MSG_BOOT>");
    qRegisterMetaType<QList<LOG_MSG_NORMAL> > ("QList<LOG_MSG_NORMAL>");
    qRegisterMetaType<QList<LOG_MSG_DNF>>("QList<LOG_MSG_DNF>");
    qRegisterMetaType<QList<LOG_MSG_DMESG>>("QList<LOG_MSG_DMESG>");
    qRegisterMetaType<LOG_FLAG> ("LOG_FLAG");

}

LogFileParser::~LogFileParser()
{
    stopAllLoad();
    //释放共享内存
    SharedMemoryManager::instance()->releaseMemory();
}

int LogFileParser::parseByJournal(QStringList arg)
{
    stopAllLoad();
    m_isJournalLoading = true;

#if 0
    m_currentJournalWork = journalWork::instance();

    m_currentJournalWork->stopWork();
//    journalWork   *work = new journalWork();
//    m_currentJournalWork = work;
    disconnect(m_currentJournalWork, SIGNAL(journalFinished()), this, SLOT(slot_journalFinished()));
    disconnect(m_currentJournalWork, &journalWork::journalData, this, &LogFileParser::journalData);
    m_currentJournalWork->setArg(arg);
    connect(m_currentJournalWork, SIGNAL(journalFinished()), this,  SLOT(slot_journalFinished()),
            Qt::QueuedConnection);
    connect(m_currentJournalWork, &journalWork::journalData, this, &LogFileParser::slot_journalData,
            Qt::QueuedConnection);
    connect(this, &LogFileParser::stopJournal, this, [ = ] {
        if (m_currentJournalWork)
        {
            disconnect(m_currentJournalWork, SIGNAL(journalFinished()), this,  SLOT(slot_journalFinished()));
            disconnect(m_currentJournalWork, &journalWork::journalData, this, &LogFileParser::slot_journalData);
        }
    });
    m_currentJournalWork->start();
    //QtConcurrent::run(work, &journalWork::doWork);
    // QThreadPool::globalInstance()->start(work);

#endif
#if 1
    emit stopJournal();
    journalWork *work = new journalWork(this);

    work->setArg(arg);
    auto a = connect(work, &journalWork::journalFinished, this, &LogFileParser::journalFinished,
                     Qt::QueuedConnection);
    auto b = connect(work, &journalWork::journalData, this, &LogFileParser::journalData,
                     Qt::QueuedConnection);

    connect(this, &LogFileParser::stopJournal, work, &journalWork::stopWork);

    int index = work->getIndex();
    QThreadPool::globalInstance()->start(work);
    return index;
#endif
}

int LogFileParser::parseByJournalBoot(QStringList arg)
{
    stopAllLoad();
    JournalBootWork *work = new JournalBootWork(this);

    work->setArg(arg);
    auto a = connect(work, &JournalBootWork::journalBootFinished, this, &LogFileParser::journalBootFinished,
                     Qt::QueuedConnection);
    auto b = connect(work, &JournalBootWork::journaBootlData, this, &LogFileParser::journaBootlData,
                     Qt::QueuedConnection);

    connect(this, &LogFileParser::stopJournalBoot, work, &JournalBootWork::stopWork);

    int index = work->getIndex();
    QThreadPool::globalInstance()->start(work);
    return index;
}

int LogFileParser::parseByDpkg(DKPG_FILTERS &iDpkgFilter)
{

    stopAllLoad();
    LogAuthThread   *authThread = new LogAuthThread(this);
    authThread->setType(DPKG);
    QStringList filePath = DLDBusHandler::instance(this)->getFileInfo("dpkg");
    //    const QString&str="/var/log/kern";
    authThread->setFilePath(filePath);
    authThread->setFileterParam(iDpkgFilter);
    connect(authThread, &LogAuthThread::proccessError, this,
            &LogFileParser::slog_proccessError, Qt::UniqueConnection);
    connect(authThread, &LogAuthThread::dpkgFinished, this,
            &LogFileParser::dpkgFinished, Qt::UniqueConnection);
    connect(authThread, &LogAuthThread::dpkgData, this,
            &LogFileParser::dpkgData, Qt::UniqueConnection);
    connect(this, &LogFileParser::stopDpkg, authThread, &LogAuthThread::stopProccess);
    int index = authThread->getIndex();
    QThreadPool::globalInstance()->start(authThread);
    return index;
}

int LogFileParser::parseByXlog(XORG_FILTERS &iXorgFilter)    // modifed by Airy
{
    stopAllLoad();
    LogAuthThread   *authThread = new LogAuthThread(this);
    authThread->setType(XORG);
    QStringList filePath = DLDBusHandler::instance(this)->getFileInfo("Xorg");
    authThread->setFilePath(filePath);
    authThread->setFileterParam(iXorgFilter);
    connect(authThread, &LogAuthThread::proccessError, this,
            &LogFileParser::slog_proccessError, Qt::UniqueConnection);
    connect(authThread, &LogAuthThread::xorgFinished, this,
            &LogFileParser::xlogFinished, Qt::UniqueConnection);
    connect(authThread, &LogAuthThread::xorgData, this,
            &LogFileParser::xlogData, Qt::UniqueConnection);
    connect(this, &LogFileParser::stopXlog, authThread, &LogAuthThread::stopProccess);
    int index = authThread->getIndex();
    QThreadPool::globalInstance()->tryStart(authThread);
    return index;
}

// add by Airy
#include <time.h>
#include <utmp.h>
#include <utmpx.h>
#include <wtmpparse.h>

int LogFileParser::parseByNormal(NORMAL_FILTERS &iNormalFiler)
{
    stopAllLoad();
    LogAuthThread   *authThread = new LogAuthThread(this);
    authThread->setType(Normal);
    authThread->setFileterParam(iNormalFiler);
    connect(authThread, &LogAuthThread::proccessError, this,
            &LogFileParser::slog_proccessError, Qt::UniqueConnection);
    connect(authThread, &LogAuthThread::normalFinished, this,
            &LogFileParser::normalFinished, Qt::UniqueConnection);
    connect(authThread, &LogAuthThread::normalData, this,
            &LogFileParser::normalData, Qt::UniqueConnection);
    connect(this, &LogFileParser::stopNormal, authThread, &LogAuthThread::stopProccess);
    int index = authThread->getIndex();
    QThreadPool::globalInstance()->tryStart(authThread);
    return index;
}

int LogFileParser::parseByKwin(KWIN_FILTERS iKwinfilter)
{
    stopAllLoad();
    LogAuthThread   *authThread = new LogAuthThread(this);
    authThread->setType(Kwin);
    authThread->setFileterParam(iKwinfilter);
    connect(authThread, &LogAuthThread::kwinFinished, this,
            &LogFileParser::kwinFinished);
    connect(authThread, &LogAuthThread::kwinData, this,
            &LogFileParser::kwinData);
    connect(this, &LogFileParser::stopKwin, authThread, &LogAuthThread::stopProccess);

    int index = authThread->getIndex();
    QThreadPool::globalInstance()->start(authThread);
    return index;
}
#if 0
void LogFileParser::parseByXlog(QStringList &xList)
{
    QProcess proc;
    proc.start("cat /var/log/Xorg.0.log");  // file path is fixed. so write cmd direct
    proc.waitForFinished(-1);

    if (isErroCommand(QString(proc.readAllStandardError())))
        return;

    QString output = proc.readAllStandardOutput();
    proc.close();

    for (QString str : output.split('\n')) {
        if (str.startsWith("[")) {
            //            xList.append(str);
            xList.insert(0, str);
        } else {
            str += " ";
            //            xList[xList.size() - 1] += str;
            xList[0] += str;
        }
    }
    createFile(output, xList.count());

    emit xlogFinished();
}
#endif

int LogFileParser::parseByBoot()
{
    stopAllLoad();
    m_isBootLoading = true;
    LogAuthThread   *authThread = new LogAuthThread(this);
    authThread->setType(BOOT);

    QStringList filePath = DLDBusHandler::instance(this)->getFileInfo("boot");
    authThread->setFilePath(filePath);
    connect(authThread, &LogAuthThread::bootFinished, this,
            &LogFileParser::bootFinished);
    connect(authThread, &LogAuthThread::bootData, this,
            &LogFileParser::bootData);
    connect(this, &LogFileParser::stopBoot, authThread,
            &LogAuthThread::stopProccess);
    int index = authThread->getIndex();
    QThreadPool::globalInstance()->start(authThread);
    return index;
}

int LogFileParser::parseByKern(KERN_FILTERS &iKernFilter)
{
    stopAllLoad();
    m_isKernLoading = true;
    LogAuthThread   *authThread = new LogAuthThread(this);
    authThread->setType(KERN);
    QStringList filePath = DLDBusHandler::instance(this)->getFileInfo("kern");
    authThread->setFileterParam(iKernFilter);
    authThread->setFilePath(filePath);
    connect(authThread, &LogAuthThread::kernFinished, this,
            &LogFileParser::kernFinished);
    connect(authThread, &LogAuthThread::kernData, this,
            &LogFileParser::kernData);
    connect(this, &LogFileParser::stopKern, authThread,
            &LogAuthThread::stopProccess);
    int index = authThread->getIndex();
    QThreadPool::globalInstance()->start(authThread);
    return index;
}

int LogFileParser::parseByApp(APP_FILTERS &iAPPFilter)
{
    stopAllLoad();
    m_isAppLoading = true;

    m_appThread = new LogApplicationParseThread(this);
    quitLogAuththread(m_appThread);

    disconnect(m_appThread, &LogApplicationParseThread::appFinished, this,
               &LogFileParser::appFinished);
    disconnect(m_appThread, &LogApplicationParseThread::appData, this,
               &LogFileParser::appData);
    disconnect(this, &LogFileParser::stopApp, m_appThread,
               &LogApplicationParseThread::stopProccess);
    m_appThread->setParam(iAPPFilter);
    connect(m_appThread, &LogApplicationParseThread::appFinished, this,
            &LogFileParser::appFinished);
    connect(m_appThread, &LogApplicationParseThread::appData, this,
            &LogFileParser::appData);
    connect(this, &LogFileParser::stopApp, m_appThread,
            &LogApplicationParseThread::stopProccess);
    connect(m_appThread, &LogApplicationParseThread::finished, m_appThread,
            &QObject::deleteLater);
    int index = m_appThread->getIndex();
    m_appThread->start();
    return index;
}

void LogFileParser::parseByDnf(DNF_FILTERS iDnfFilter)
{
    stopAllLoad();
    LogAuthThread *authThread = new LogAuthThread(this);
    authThread->setType(Dnf);
    QStringList filePath = DLDBusHandler::instance(this)->getFileInfo("dnf");
    authThread->setFilePath(filePath);
    authThread->setFileterParam(iDnfFilter);
    connect(authThread, &LogAuthThread::proccessError, this,
            &LogFileParser::slog_proccessError, Qt::UniqueConnection);
    connect(authThread, &LogAuthThread::dnfFinished, this,
            &LogFileParser::dnfFinished, Qt::UniqueConnection);
    connect(this, &LogFileParser::stopDnf, authThread,
            &LogAuthThread::stopProccess);
    QThreadPool::globalInstance()->start(authThread);
}

void LogFileParser::parseByDmesg(DMESG_FILTERS iDmesgFilter)
{
    stopAllLoad();
    LogAuthThread *authThread = new LogAuthThread(this);
    authThread->setType(Dmesg);
    QStringList filePath = DLDBusHandler::instance(this)->getFileInfo("dmesg");
    authThread->setFilePath(filePath);
    authThread->setFileterParam(iDmesgFilter);
    connect(authThread, &LogAuthThread::proccessError, this,
            &LogFileParser::slog_proccessError, Qt::UniqueConnection);
    connect(authThread, &LogAuthThread::dmesgFinished, this,
            &LogFileParser::dmesgFinished, Qt::UniqueConnection);
    connect(this, &LogFileParser::stopDmesg, authThread,
            &LogAuthThread::stopProccess);
    QThreadPool::globalInstance()->start(authThread);
}

void LogFileParser::createFile(QString output, int count)
{
#if 1
    Q_UNUSED(output)
    Q_UNUSED(count)
#else
    // this is for test parser.
    QFile fi("tempFile");
    if (!fi.open(QIODevice::ReadWrite | QIODevice::Truncate))
        return;
    fi.write(output.toLatin1());
    fi.write(QString::number(count).toLatin1());
    fi.close();
#endif
}

void LogFileParser::stopAllLoad()
{
    emit stopKern();
    emit stopBoot();
    emit stopDpkg();
    emit stopXlog();
    emit stopKwin();
    emit stopApp();
    emit stopJournal();
    emit stopJournalBoot();
    emit stopNormal();
    emit stopDnf();
    emit stopDmesg();
    return;
}



void LogFileParser::quitLogAuththread(QThread *iThread)
{
    if (iThread && iThread->isRunning()) {
        qDebug() << __FUNCTION__;
        iThread->quit();
        iThread->wait();
    }
}






#include <unistd.h>
#include <QApplication>

/**
 * @brief LogFileParser::slog_proccessError 处理转发日志获取线程的错误信息信号
 * @param iError 错误信息
 */
void LogFileParser::slog_proccessError(const QString &iError)
{
    emit proccessError(iError);
}




