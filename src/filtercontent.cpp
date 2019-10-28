#include "filtercontent.h"
#include <DApplication>
#include <DApplicationHelper>
#include <DComboBox>
#include <DCommandLinkButton>
#include <DFileDialog>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QFileInfoList>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QProcess>
#include <QVBoxLayout>
#include "logperiodbutton.h"
#include "structdef.h"

DWIDGET_USE_NAMESPACE

FilterContent::FilterContent(QWidget *parent)
    : DFrame(parent)
    , m_curBtnId(ALL)
    , m_curLvCbxId(INF)
{
    initUI();
    initConnections();
}

FilterContent::~FilterContent() {}

void FilterContent::initUI()
{
    QVBoxLayout *vLayout = new QVBoxLayout;
    // set period info
    hLayout_period = new QHBoxLayout;
    periodLabel = new DLabel(DApplication::translate("Label", "Period:"), this);
    m_btnGroup = new QButtonGroup;
    LogPeriodButton *m_allBtn = new LogPeriodButton(DApplication::translate("Button", "All"), this);
    m_allBtn->setObjectName("allBtn");
    m_btnGroup->addButton(m_allBtn, 0);
    LogPeriodButton *m_todayBtn =
        new LogPeriodButton(DApplication::translate("Button", "Today"), this);
    m_btnGroup->addButton(m_todayBtn, 1);
    LogPeriodButton *m_threeDayBtn =
        new LogPeriodButton(DApplication::translate("Button", "3 days"), this);
    m_btnGroup->addButton(m_threeDayBtn, 2);
    LogPeriodButton *m_lastWeekBtn =
        new LogPeriodButton(DApplication::translate("Button", "1 week"), this);
    m_btnGroup->addButton(m_lastWeekBtn, 3);
    LogPeriodButton *m_lastMonthBtn =
        new LogPeriodButton(DApplication::translate("Button", "1 month"), this);
    m_btnGroup->addButton(m_lastMonthBtn, 4);
    LogPeriodButton *m_threeMonthBtn =
        new LogPeriodButton(DApplication::translate("Button", "3 months"), this);
    m_btnGroup->addButton(m_threeMonthBtn, 5);

    setUeButtonSytle();

    hLayout_period->addWidget(periodLabel);
    hLayout_period->addWidget(m_allBtn);
    hLayout_period->addWidget(m_todayBtn);
    hLayout_period->addWidget(m_threeDayBtn);
    hLayout_period->addWidget(m_lastWeekBtn);
    hLayout_period->addWidget(m_lastMonthBtn);
    hLayout_period->addWidget(m_threeMonthBtn);
    hLayout_period->addStretch(1);

    DSuggestButton *cmdLinkBtn =
        new DSuggestButton(DApplication::translate("Button", "Reset"), this);
    cmdLinkBtn->setFlat(true);
    cmdLinkBtn->hide();
    m_btnGroup->addButton(cmdLinkBtn, 6);
    hLayout_period->addWidget(cmdLinkBtn);

    // set level info
    hLayout_all = new QHBoxLayout;

    QHBoxLayout *hLayout_lv = new QHBoxLayout;
    lvTxt = new DLabel(DApplication::translate("Label", "Level:"), this);
    cbx_lv = new DComboBox(this);
    cbx_lv->setMinimumWidth(150);
    cbx_lv->addItems(QStringList() << DApplication::translate("ComboBox", "Emer")
                                   << DApplication::translate("ComboBox", "Alert")
                                   << DApplication::translate("ComboBox", "Critical")
                                   << DApplication::translate("ComboBox", "Error")
                                   << DApplication::translate("ComboBox", "Warning")
                                   << DApplication::translate("ComboBox", "Notice")
                                   << DApplication::translate("ComboBox", "Info")
                                   << DApplication::translate("ComboBox", "Debug"));
    cbx_lv->setCurrentText(DApplication::translate("ComboBox", "Info"));
    hLayout_lv->addWidget(lvTxt);
    hLayout_lv->addWidget(cbx_lv, 1);
    hLayout_all->addLayout(hLayout_lv);

    // set all files under ~/.cache/deepin
    QHBoxLayout *hLayout_app = new QHBoxLayout;
    appTxt = new DLabel(DApplication::translate("Label", "Application list:"), this);
    cbx_app = new DComboBox(this);
    hLayout_app->addWidget(appTxt);
    hLayout_app->addWidget(cbx_app, 1);
    hLayout_all->addLayout(hLayout_app);

    hLayout_all->addStretch();

    DPushButton *exportBtn = new DPushButton(DApplication::translate("Button", "Export"), this);
    exportBtn->setMinimumSize(m_allBtn->size());
    m_btnGroup->addButton(exportBtn, 7);
    hLayout_all->addWidget(exportBtn);

    // set layout
    vLayout->addLayout(hLayout_period);
    vLayout->addLayout(hLayout_all);
    this->setLayout(vLayout);

    // default application list is invisible
    setSelectorVisible(true, false, true, false);
}

void FilterContent::initConnections()
{
    connect(m_btnGroup, SIGNAL(buttonClicked(int)), this, SLOT(slot_buttonClicked(int)));
    connect(cbx_lv, SIGNAL(currentIndexChanged(int)), this, SLOT(slot_cbxLvIdxChanged(int)));
    connect(cbx_app, SIGNAL(currentIndexChanged(int)), this, SLOT(slot_cbxAppIdxChanged(int)));
}

void FilterContent::setAppComboBoxItem()
{
    QString homePath = QDir::homePath();
    if (homePath.isEmpty()) {
        return;
    }
    QString path = homePath + "/.cache/deepin/";
    QDir dir(path);
    if (!dir.exists()) {
        return;
    }
    cbx_app->clear();

    QStringList fileInfoList = dir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot);

    QMap<QString, QString> name_path_map;
    name_path_map.clear();

    for (int i = 0; i < fileInfoList.size(); ++i) {
        QString appName = fileInfoList.at(i);
        QString fullPath = path + appName;
        QDir subdir(fullPath);
        if (!subdir.exists()) {
            continue;
        }
        QStringList logFiles = subdir.entryList(QDir::NoDotAndDotDot | QDir::Files);

        for (int j = 0; j < logFiles.count(); j++) {
            QString fileName = logFiles.at(j);
            if (!fileName.contains(".log"))
                continue;
            QString absPath = QString("%1/%2").arg(fullPath).arg(fileName);
            name_path_map.insert(fileName, absPath);
        }
    }

    QMap<QString, QString>::const_iterator iter = name_path_map.constBegin();
    while (iter != name_path_map.constEnd()) {
        cbx_app->addItem(iter.key());
        cbx_app->setItemData(cbx_app->count() - 1, iter.value(), Qt::UserRole + 1);
        ++iter;
    }
}

void FilterContent::setSelectorVisible(bool lvCbx, bool appListCbx, bool period, bool needMove)
{
    lvTxt->setVisible(lvCbx);
    cbx_lv->setVisible(lvCbx);

    appTxt->setVisible(appListCbx);
    cbx_app->setVisible(appListCbx);

    periodLabel->setVisible(period);
    for (int i = 0; i < 6; i++) {
        LogPeriodButton *pushBtn = static_cast<LogPeriodButton *>(m_btnGroup->button(i));
        pushBtn->setVisible(period);
    }

    if (needMove) {
        hLayout_period->addWidget(m_btnGroup->button(EXPORT));
        hLayout_all->removeWidget(m_btnGroup->button(EXPORT));
    } else {
        hLayout_period->removeWidget(m_btnGroup->button(EXPORT));
        hLayout_all->addWidget(m_btnGroup->button(EXPORT));
    }
}

void FilterContent::setUeButtonSytle()
{
    for (QAbstractButton *abtn : m_btnGroup->buttons()) {
        LogPeriodButton *btn = static_cast<LogPeriodButton *>(abtn);
        btn->setFlat(true);
        btn->setCheckable(true);
        if (btn->objectName() == "allBtn")
            btn->setChecked(true);
    }
}

void FilterContent::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    // Save pen
    QPen oldPen = painter.pen();

    painter.setRenderHint(QPainter::Antialiasing);
    DPalette pa = DApplicationHelper::instance()->palette(this);
    painter.setBrush(QBrush(pa.color(DPalette::Base)));
    QColor penColor = pa.color(DPalette::FrameBorder);
    penColor.setAlphaF(0.05);
    painter.setPen(QPen(penColor));

    QRectF rect = this->rect();
    rect.setX(0.5);
    rect.setY(0.5);
    rect.setWidth(rect.width() - 0.5);
    rect.setHeight(rect.height() - 0.5);

    QPainterPath painterPath;
    painterPath.addRoundedRect(rect, 8, 8);
    painter.drawPath(painterPath);

    // Restore the pen
    painter.setPen(oldPen);

    DFrame::paintEvent(event);
}

void FilterContent::slot_treeClicked(const QModelIndex &index)
{
    if (!index.isValid())
        return;

    QString itemData = index.data(Qt::UserRole + 1).toString();
    if (itemData.isEmpty()) {
        return;
    }

    m_curTreeIndex = index;

    if (itemData.contains(APP_TREE_DATA, Qt::CaseInsensitive)) {
        this->setAppComboBoxItem();
        this->setSelectorVisible(true, true, true, false);
        cbx_app->setCurrentIndex(0);
        emit sigCbxAppIdxChanged(cbx_app->itemData(0, Qt::UserRole + 1).toString());
    } else if (itemData.contains(JOUR_TREE_DATA, Qt::CaseInsensitive)) {
        this->setSelectorVisible(true, false, true, false);
        cbx_lv->setCurrentIndex(INF);
        //    } else if (itemData.contains(".cache")) {
        //        this->setSelectorVisible(true, true);
        //        cbx_lv->setCurrentIndex(INF);
    } else if (itemData.contains(BOOT_TREE_DATA) || itemData.contains(XORG_TREE_DATA)) {
        this->setSelectorVisible(false, false, false, true);
    } else if (itemData.contains(KERN_TREE_DATA) || itemData.contains(DPKG_TREE_DATA)) {
        this->setSelectorVisible(false, false, true, true);
    }
}

void FilterContent::slot_buttonClicked(int idx)
{
    /** note: In order to adapt to the new scene, select time-period first,
     *        then select any log item, should display current log info.
     *        so comment this judge.
     */
    //    if (!m_curTreeIndex.isValid())
    //        return;

    QString itemData = m_curTreeIndex.data(Qt::UserRole + 1).toString();

    switch (idx) {
        case ALL:
        case ONE_DAY:
        case THREE_DAYS:
        case ONE_WEEK:
        case ONE_MONTH:
        case THREE_MONTHS: {
            m_curBtnId = idx;
            emit sigButtonClicked(idx, m_curLvCbxId, m_curTreeIndex);
        } break;
        case RESET: {
            m_curBtnId = ALL;
            //            if (itemData.contains(JOUR_TREE_DATA, Qt::CaseInsensitive) ||
            //                itemData.contains(".cache"))
            if (itemData.contains(JOUR_TREE_DATA, Qt::CaseInsensitive) ||
                itemData.contains(APP_TREE_DATA, Qt::CaseInsensitive)) {
                cbx_lv->setCurrentIndex(INF);
            } else {
                emit sigButtonClicked(m_curBtnId, INVALID, m_curTreeIndex);
            }
        } break;
        case EXPORT:
            if (!itemData.isEmpty())
                emit sigExportInfo();
            break;
        default:
            break;
    }
}

void FilterContent::slot_cbxLvIdxChanged(int idx)
{
    m_curLvCbxId = idx;
    emit sigButtonClicked(m_curBtnId, idx, m_curTreeIndex);
}

void FilterContent::slot_cbxAppIdxChanged(int idx)
{
    QString path = cbx_app->itemData(idx, Qt::UserRole + 1).toString();

    emit sigCbxAppIdxChanged(path);
}
