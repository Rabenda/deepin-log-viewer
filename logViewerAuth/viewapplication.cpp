/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     zyc <zyc@uniontech.com>
*
* Maintainer:  zyc <zyc@uniontech.com>
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

#include "viewapplication.h"

#include <QDebug>
#include <QFile>
#include <QProcess>
#include <QString>
#include <QStringList>
#include <QThread>
#include <QCommandLineParser>
#include <QSharedMemory>
#include <QBuffer>
#include <QDataStream>

#include <iostream>
#include<signal.h>

ViewApplication::ViewApplication(int &argc, char **argv): QCoreApplication(argc, argv),m_commondM(new QSharedMemory())
{
    QCommandLineParser parser;
    parser.process(*this);
    const QStringList fileList = parser.positionalArguments();
    if (fileList.count() < 2) {
        qDebug() << "less than 2";
        return ;
    }
    QStringList arg;
    if (fileList[0] == "dmesg") {
        arg << "-c"
            << "dmesg -r";
    } else {
        arg << "-c" << QString("cat %1").arg(fileList[0]);
    }
    m_proc = new QProcess(this);

    m_commondM->setKey(fileList[1]);
    m_commondM->create(sizeof(ShareMemoryInfo));
    if (m_commondM->isAttached())      //检测程序当前是否关联共享内存
        m_commondM->detach();          //解除关联
    m_commondM->attach(QSharedMemory::ReadOnly);

    connect(m_proc, &QProcess::readyReadStandardOutput, this, [ = ] {
        if (!getControlInfo().isStart)
        {
            m_proc->kill();
            releaseMemery();
            exit(0);
        }
        QByteArray byte =   m_proc->readAll();
        std::cout << byte.replace('\u0000', "").data();
    });
    m_proc->start("/bin/bash", arg);

    m_proc->waitForFinished(-1);
    m_proc->close();
}

ViewApplication::~ViewApplication()
{
    releaseMemery();
}

void ViewApplication::releaseMemery()
{
    if (m_commondM) {
        //  m_commondM->unlock();
        if (m_commondM->isAttached())      //检测程序当前是否关联共享内存
            m_commondM->detach();          //解除关联
        m_commondM->deleteLater();
    }
}

ViewApplication::ShareMemoryInfo ViewApplication::getControlInfo()
{
    ShareMemoryInfo defaultInfo;
    defaultInfo.isStart = false;
    if (m_commondM && m_commondM->isAttached()) {
        m_commondM->lock();
        ShareMemoryInfo   *m_pShareMemoryInfo = static_cast<ShareMemoryInfo *>(m_commondM->data());
        defaultInfo = m_pShareMemoryInfo ? *m_pShareMemoryInfo : defaultInfo;
        m_commondM->unlock();
        return defaultInfo;

    }
    return defaultInfo;
}
