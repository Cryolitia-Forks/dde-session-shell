// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "multiuserswarningview.h"
#include "inhibitbutton.h"

#include <DFontSizeManager>

#include <QLabel>
#include <QListWidget>
#include <QPainter>
#include <QPainterPath>
#include <QVBoxLayout>
#include <QPushButton>
#include <QButtonGroup>

const static QSize UserAvatarSize = QSize(64, 64);
const static QSize UserListItemSize = QSize(180, 80);
const int ButtonWidth = 200;
const int ButtonHeight = 64;
const QSize iconSize = QSize(24, 24);

MultiUsersWarningView::MultiUsersWarningView(SessionBaseModel::PowerAction inhibitType, QWidget *parent)
    : WarningView(parent)
    , m_vLayout(new QVBoxLayout(this))
    , m_userList(new QListWidget)
    , m_warningTip(new QLabel)
    , m_cancelBtn(new InhibitButton(this))
    , m_actionBtn(new InhibitButton(this))
    , m_inhibitType(inhibitType)
{
    QIcon acceptIcon = QIcon::fromTheme(":/img/inhibitview/shutdown.svg");

    m_userList->setAttribute(Qt::WA_TranslucentBackground);
    // m_userList->setSelectionRectVisible(false);
    m_userList->setSelectionMode(QListView::NoSelection);
    m_userList->setEditTriggers(QListView::NoEditTriggers);
    m_userList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_userList->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    // m_userList->viewport()->setAttribute(Qt::WA_TranslucentBackground);
    m_userList->setFrameStyle(QFrame::NoFrame);
    m_userList->setGridSize(UserListItemSize);
    m_userList->setFocusPolicy(Qt::NoFocus);
    m_userList->setStyleSheet("background-color:transparent;");
    DFontSizeManager::instance()->bind(m_warningTip, DFontSizeManager::T5);

    m_warningTip->setFixedWidth(300);
    m_warningTip->setStyleSheet("color: white;");
    m_warningTip->setWordWrap(true);
    m_warningTip->setAlignment(Qt::AlignCenter);
    m_warningTip->setFocusPolicy(Qt::NoFocus);
    m_warningTip->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    m_actionBtn->setFixedSize(ButtonWidth, ButtonHeight);
    m_actionBtn->setFocusPolicy(Qt::StrongFocus);
    m_actionBtn->setNormalPixmap(acceptIcon.pixmap(iconSize * devicePixelRatioF()));
    m_actionBtn->setHoverPixmap(acceptIcon.pixmap(iconSize * devicePixelRatioF()));
    QIcon iconCancelNormal = QIcon::fromTheme(":/img/inhibitview/cancel_normal.svg");
    QIcon iconCancelHover = QIcon::fromTheme(":/img/inhibitview/cancel_hover.svg");

    m_cancelBtn->setFixedSize(ButtonWidth, ButtonHeight);
    m_cancelBtn->setFocusPolicy(Qt::StrongFocus);
    m_cancelBtn->setNormalPixmap(iconCancelNormal.pixmap(iconSize * devicePixelRatioF()));
    m_cancelBtn->setHoverPixmap(iconCancelHover.pixmap(iconSize * devicePixelRatioF()));
    m_cancelBtn->setText(tr("Cancel"));

    QVBoxLayout *btnLayout = new QVBoxLayout;
    btnLayout->addStretch();
    btnLayout->addWidget(m_cancelBtn, 0, Qt::AlignHCenter);
    btnLayout->addSpacing(15);
    btnLayout->addWidget(m_actionBtn, 0, Qt::AlignHCenter);
    btnLayout->addStretch();

    m_vLayout->addStretch();
    m_vLayout->addWidget(m_userList, 0, Qt::AlignHCenter);
    m_vLayout->addSpacing(40);
    m_vLayout->addWidget(m_warningTip, 1, Qt::AlignHCenter);
    m_vLayout->addLayout(btnLayout);
    m_vLayout->addStretch();

    updateWarningTip();

    connect(m_cancelBtn, &InhibitButton::clicked, this, &MultiUsersWarningView::cancelled);
    connect(m_actionBtn, &InhibitButton::clicked, this, &MultiUsersWarningView::actionInvoked);

    this->setTabOrder(m_cancelBtn, m_actionBtn);
    this->setTabOrder(m_actionBtn, m_cancelBtn);
}

MultiUsersWarningView::~MultiUsersWarningView()
{
}

void MultiUsersWarningView::setUsers(QList<std::shared_ptr<User>> users)
{
    m_userList->clear();

    m_userList->setFixedSize(UserListItemSize.width(),
                             UserListItemSize.height() * qMin(users.length(), 4));

    for (std::shared_ptr<User> user : users) {
        QListWidgetItem * item = new QListWidgetItem;
        m_userList->addItem(item);

        QString icon = getUserIcon(user->avatar());
        m_userList->setItemWidget(item, new UserListItem(icon, user->name()));
    }
}

SessionBaseModel::PowerAction MultiUsersWarningView::action() const
{
    return m_action;
}

void MultiUsersWarningView::updateWarningTip()
{
    switch (m_inhibitType) {
    case SessionBaseModel::PowerAction::RequireShutdown:
        m_warningTip->setText(tr("The above users are still logged in and data will be lost due to shutdown, are you sure you want to shut down?"));
        break;
    default:
        m_warningTip->setText(tr("The above users are still logged in and data will be lost due to reboot, are you sure you want to reboot?"));
        break;
    }
}

void MultiUsersWarningView::setAcceptReason(const QString &reason)
{
    m_actionBtn->setText(reason);
}

bool MultiUsersWarningView::focusNextPrevChild(bool next)
{
    if (!next) {
        qWarning() << "focus handling error, nextPrevChild is False";
        return WarningView::focusNextPrevChild(next);
    }
    return WarningView::focusNextPrevChild(next);
}

QString MultiUsersWarningView::getUserIcon(const QString &path)
{
    const QUrl url(path);
    if (url.isLocalFile())
        return url.path();

    return path;
}

UserListItem::UserListItem(const QString &icon, const QString &name) :
    QFrame(),
    m_icon(new QLabel(this)),
    m_name(new QLabel(name, this))
{
    setFixedSize(UserListItemSize);

    m_icon->setFixedSize(UserAvatarSize);
    m_icon->setScaledContents(true);
    m_icon->setPixmap(getRoundPixmap(icon));

    m_name->setStyleSheet("color: white;");
    DFontSizeManager::instance()->bind(m_name, DFontSizeManager::T5);
    m_name->move(80, 20);
}

QPixmap UserListItem::getRoundPixmap(const QString &path)
{
    QPixmap source(path);
    QPixmap result(source.size());
    result.fill(Qt::transparent);

    QPainter p(&result);
    p.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    QPainterPath pp;
    pp.addEllipse(result.rect());
    p.setClipPath(pp);
    p.drawPixmap(result.rect(), source);
    p.end();

    return result;
}
