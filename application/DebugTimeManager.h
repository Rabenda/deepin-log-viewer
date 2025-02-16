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

#ifndef DEBUGTIMEMANAGER_H
#define DEBUGTIMEMANAGER_H

#include <QObject>
#include <QMap>
#include <QString>
#include "config.h"
#define PERF_ON
#ifdef PERF_ON
#define PERF_PRINT_BEGIN(point, dsec) DebugTimeManager::getInstance()->beginPointLinux(point,dsec)
#define PERF_PRINT_END(point, dsec) DebugTimeManager::getInstance()->endPointLinux(point, dsec)
#else
#define PERF_PRINT_BEGIN(point, dsec)
#define PERF_PRINT_END(point, dsec)
#endif
/**
 * @brief The PointInfo struct
 */
struct PointInfo {
    QString desc;
    qint64  time;
};
struct PointInfoLinux {
    QString desc;
    timespec  time;
};

class DebugTimeManager
{
public:
    /**
     * @brief getInstance : get signal instance
     * @return : the signal instance
     */
    static DebugTimeManager *getInstance()
    {
        static DebugTimeManager m_manager;
        return  &m_manager;
    }

    /**
     * @brief clear : clear data
     */
    void clear();

    /**
     * @brief beginPointQt : 打点开始,Qt
     * @param point : 所打的点的名称，固定格式，在打点文档中查看 -- POINT-XX POINT-01
     * @param status : 性能测试的状态，比如测试时文件的大小
     */
    //    void beginPointQt(const QString &point, const QString &status = "");

    /**
     * @brief endPointQt : 结束打点,Qt
     * @param point : 需要结束的点
     */
    //    void endPointQt(const QString &point);

    /**
     * @brief beginPointLinux : 打点开始,Linux
     * @param point : 所打的点的名称，固定格式，在打点文档中查看 -- POINT-XX POINT-01
     * @param status : 性能测试的状态，比如测试时文件的大小
     */
    void beginPointLinux(const QString &point, const QString &status = "");

    /**
     * @brief endPointLinux : 结束打点,Linux
     * @param point : 需要结束的点
     */
    void endPointLinux(const QString &point, const QString &status = "");
    /**
     * @brief diff 计算两个timespec结构体时间之差
     * @param start 开始时间
     * @param end 结束时间
     * @return  时间差
     */
    timespec diff(timespec start, timespec end);
protected:
    DebugTimeManager();


private:
    QMap<QString, PointInfo>    m_MapPoint;      //<! 保存所打的点
    QMap<QString, PointInfoLinux>    m_MapLinuxPoint;      //<! 保存所打的点

};

#endif // DEBUGTIMEMANAGER_H
