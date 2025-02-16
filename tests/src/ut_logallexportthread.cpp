/*
* Copyright (C) 2022 UnionTech Software Technology Co.,Ltd
*
* Author:     liuxinping <liuxinping@uniontech.com>
* Maintainer:  liuxinping <liuxinping@uniontech.com>
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <gtest/gtest.h>
#include "dbusproxy/dldbushandler.h"
#include "../application/logallexportthread.h"
#include <stub.h>

QStringList LogAllExportThread_stub_toString()
{
    QStringList list;
    list << "test" << "test2";
    return list;
}

bool LogAllExportThread_stub_bool(const QString &outDir, const QString &in, bool isFile)
{
    Q_UNUSED(outDir);
    Q_UNUSED(in);
    Q_UNUSED(isFile);
    return false;
}


TEST(LogAllExportThread_LogAllExportThread_UT, LogAllExportThread_LogAllExportThread_UT_001)
{
    QStringList thread;
    thread << "test" << "test2";
    LogAllExportThread *p = new LogAllExportThread(thread, "path");
    ASSERT_TRUE(p);
    EXPECT_EQ(p->m_outfile, "path");
    delete p;
}


TEST(LogAllExportThread_run_UT, LogAllExportThread_run_UT_001)
{
    QStringList thread;
    thread << "test" << "test2";
    LogAllExportThread *p = new LogAllExportThread(thread, "path");
    ASSERT_TRUE(p);
    Stub stub;
    stub.set(ADDR(DLDBusHandler, getFileInfo), LogAllExportThread_stub_toString);
    stub.set(ADDR(DLDBusHandler, exportLog), LogAllExportThread_stub_bool);
    p->m_types << JOUR_TREE_DATA << BOOT_KLU_TREE_DATA << DMESG_TREE_DATA << LAST_TREE_DATA <<
               DPKG_TREE_DATA << KERN_TREE_DATA << XORG_TREE_DATA << DNF_TREE_DATA << BOOT_TREE_DATA <<
               KWIN_TREE_DATA << APP_TREE_DATA;
    p->run();
    delete p;
}

TEST(LogAllExportThread_run_UT, LogAllExportThread_run_UT_002)
{
    QStringList thread;
    thread << "test" << "test2";
    Stub stub;
    stub.set(ADDR(DLDBusHandler, getFileInfo), LogAllExportThread_stub_toString);
    stub.set(ADDR(DLDBusHandler, exportLog), LogAllExportThread_stub_bool);
    LogAllExportThread *p = new LogAllExportThread(thread, "path");
    ASSERT_TRUE(p);
    p->run();
    delete p;
}

TEST(LogAllExportThread_slot_cancelExport_UT, LogAllExportThread_slot_cancelExport_UT_001)
{
    QStringList thread;
    thread << "test" << "test2";
    LogAllExportThread *p = new LogAllExportThread(thread, "path");
    ASSERT_TRUE(p);
    p->slot_cancelExport();
    delete p;
}
