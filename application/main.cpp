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

#include "logapplicationhelper.h"
#include "logcollectormain.h"
#include "logapplication.h"
#include "environments.h"
#include "dbusmanager.h"
#include "utils.h"
#include "DebugTimeManager.h"

#include <DApplication>
#include <DApplicationSettings>
#include <DMainWindow>
#include <DWidgetUtil>
#include <DLog>

#include <QDateTime>
#include <QSurfaceFormat>
#include <QDebug>
DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE

int main(int argc, char *argv[])
{
    //在root下或者非deepin/uos环境下运行不会发生异常，需要加上XDG_CURRENT_DESKTOP=Deepin环境变量；
    if (!QString(qgetenv("XDG_CURRENT_DESKTOP")).toLower().startsWith("deepin")){
       setenv("XDG_CURRENT_DESKTOP", "Deepin", 1);
    }

    PERF_PRINT_BEGIN("POINT-01", "");

    //klu下不使用opengl 使用OpenGLES,因为opengl基于x11 现在全面换wayland了
    QCoreApplication::setAttribute(Qt::AA_UseOpenGLES);
    //klu下不使用opengl 使用OpenGLES,因为opengl基于x11 现在全面换wayland了,这个真正有效
    qputenv("QT_WAYLAND_SHELL_INTEGRATION", "kwayland-shell");
    QString  systemName =   DBusManager::getSystemInfo();
    qDebug() << "systemName" << systemName;
    if (Utils::isWayland()) {
        qputenv("QT_WAYLAND_SHELL_INTEGRATION", "kwayland-shell");
    }
    setenv("PULSE_PROP_media.role", "video", 1);
    QSurfaceFormat format;
    format.setRenderableType(QSurfaceFormat::OpenGLES);
    format.setDefaultFormat(format);
    LogApplication a(argc, argv);

    //wayland环境判断
    auto systemEnv = QProcessEnvironment::systemEnvironment();
    QString XDG_SESSION_TYPE = systemEnv.value(QStringLiteral("XDG_SESSION_TYPE"));
    QString WAYLAND_DISPLAY = systemEnv.value(QStringLiteral("WAYLAND_DISPLAY"));
    qDebug() << "XDG_SESSION_TYPE:" << XDG_SESSION_TYPE << "---WAYLAND_DISPLAY:" << WAYLAND_DISPLAY;
    qputenv("DTK_USE_SEMAPHORE_SINGLEINSTANCE", "1");

    if (!DGuiApplicationHelper::instance()->setSingleInstance(a.applicationName(),
                                                              DGuiApplicationHelper::UserScope)) {
        qInfo() << "DGuiApplicationHelper::instance()->setSingleInstance";
        a.activeWindow();
        return 0;
    }

    //高分屏支持
    a.setAttribute(Qt::AA_UseHighDpiPixmaps);
    a.setAutoActivateWindows(true);
    a.loadTranslator();
    a.setOrganizationName("deepin");
    a.setApplicationName("deepin-log-viewer");
    a.setApplicationVersion(VERSION);
    a.setProductIcon(QIcon::fromTheme("deepin-log-viewer"));
    a.setWindowIcon(QIcon::fromTheme("deepin-log-viewer"));
    a.setProductName(DApplication::translate("Main", "Log Viewer"));
    a.setApplicationDisplayName(DApplication::translate("Main", "Log Viewer"));
    a.setApplicationDescription(
        DApplication::translate("Main", "Log Viewer is a useful tool for viewing system logs."));
    DApplicationSettings settings;
    DLogManager::registerConsoleAppender();
    DLogManager::registerFileAppender();
    LogApplicationHelper::instance();

    //命令行参数的解析
    QCommandLineParser cmdParser;
    cmdParser.setApplicationDescription("deepin-log-viewer");
    cmdParser.addHelpOption();
    cmdParser.addVersionOption();
    cmdParser.process(a);

    LogCollectorMain w;
    a.setMainWindow(&w);


    w.show();
    Dtk::Widget::moveToCenter(&w);
    bool result = a.exec();
    PERF_PRINT_END("POINT-02", "");
    return  result;
}
