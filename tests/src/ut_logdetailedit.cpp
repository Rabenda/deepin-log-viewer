/*
* Copyright (C) 2020 ~ 2021 UnionTech Software Technology Co.,Ltd
*
* Author:      linxun<linxun@uniontech.com>
* Maintainer:  linxun<linxun@uniontech.com>
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

#include "logdetailedit.h"
#include <stub.h>

#include <QGestureEvent>
#include <QMouseEvent>

#include <gtest/gtest.h>

Qt::MouseEventSource stud_source()
{
    return Qt::MouseEventSynthesizedByQt;
}

class ut_logDetailEdit_Test : public ::testing::Test
{
    // Test interface
protected:
    void SetUp()
    {
        m_infoTextEdit = new logDetailEdit;
    }
    void TearDown()
    {
        delete m_infoTextEdit;
    }

    logDetailEdit *m_infoTextEdit = nullptr;
};

TEST_F(ut_logDetailEdit_Test, ShowInstallInfoTextEdit_UT_slideGesture)
{
    m_infoTextEdit->slideGesture(5);
}

TEST_F(ut_logDetailEdit_Test, ShowInstallInfoTextEdit_UT_onSelectionArea)
{
    m_infoTextEdit->m_gestureAction = logDetailEdit::GA_tap;
    QTextCursor cursor = m_infoTextEdit->textCursor();
    cursor.selectedText() = "//";
    m_infoTextEdit->onSelectionArea();
}

TEST(FlashTween_TEST, FlashTween_UT_start)
{
    FlashTween flash;
    EXPECT_EQ(flash.sinusoidalEaseOut(1.0, 1.0, 1.0, 1.0), 1.0 * sin(1.0 / 1.0 * (3.14 / 2)) + 1.0);
    flash.start(0, 0, 1, 0, 0);
    flash.start(0, 0, 1, 1, 0);
    flash.m_timer->start(5);
    usleep(5000);
    flash.m_timer->stop();
}

int states = 1;

int state()
{
    return states;
}

TEST_F(ut_logDetailEdit_Test, ShowInstallInfoTextEdit_UT_tapGestureTriggered)
{
    Stub stub;
    stub.set(ADDR(QGesture, state), state);

    while (states <= 4) {
        QTapGesture tap;
        tap.setPosition(QPointF(10, 10));
        m_infoTextEdit->tapGestureTriggered(&tap);
        if (states != 2 && states != 4) {
            QTapAndHoldGesture tapAndHold;
            tapAndHold.setPosition(QPointF(10, 10));
            m_infoTextEdit->tapAndHoldGestureTriggered(&tapAndHold);
        }

        states++;
    }
}

TEST_F(ut_logDetailEdit_Test, ShowInstallInfoTextEdit_UT_gestureEvent)
{
    QGesture *gesture = new QGesture();

    QGestureEvent gestureEvent(QList<QGesture *> {gesture, gesture});
    m_infoTextEdit->gestureEvent(&gestureEvent);
    delete gesture;
}

TEST_F(ut_logDetailEdit_Test, ShowInstallInfoTextEdit_UT_mouseReleaseEvent)
{
    QMouseEvent releaseEvent(QEvent::MouseButtonRelease, QPoint(0, 0), QPoint(0, 0), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    m_infoTextEdit->mouseReleaseEvent(&releaseEvent);
}

TEST_F(ut_logDetailEdit_Test, ShowInstallInfoTextEdit_UT_mouseMoveEvent)
{
    QMouseEvent moveEvent(QEvent::MouseMove, QPoint(0, 0), QPoint(10, 0), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    m_infoTextEdit->mouseMoveEvent(&moveEvent);
}

TEST_F(ut_logDetailEdit_Test, ShowInstallInfoTextEdit_UT_mouseMoveEvent_source)
{
    Stub stub;
    stub.set(ADDR(QMouseEvent, source), stud_source);

    m_infoTextEdit->m_gestureAction = logDetailEdit::GA_slide;
    QMouseEvent moveEvent(QEvent::MouseMove, QPoint(0, 0), QPoint(10, 0), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    m_infoTextEdit->mouseMoveEvent(&moveEvent);
}
