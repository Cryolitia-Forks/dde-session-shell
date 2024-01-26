// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dbusshutdownagent.h"
#include "sessionbasemodel.h"

DBusShutdownAgent::DBusShutdownAgent(QObject *parent)
    : QObject(parent)
    , m_model(nullptr)
{

}

void DBusShutdownAgent::setModel(SessionBaseModel * const model)
{
    m_model = model;
}

void DBusShutdownAgent::show()
{
    if (isUpdating() || !canShowShutDown())
        return;

    m_model->setCurrentModeState(SessionBaseModel::ModeStatus::ShutDownMode);
    m_model->setVisible(true);
}

void DBusShutdownAgent::Shutdown()
{
    qCInfo(DDE_SHELL) << "Shutdown";
    if (isUpdating())
        return;

    if (!canShowShutDown()) {
         //锁屏时允许关机
        emit m_model->onRequirePowerAction(SessionBaseModel::PowerAction::RequireShutdown, false);
    } else {
        m_model->setCurrentModeState(SessionBaseModel::ModeStatus::ShutDownMode);
        m_model->setVisible(true);
        emit m_model->onRequirePowerAction(SessionBaseModel::PowerAction::RequireShutdown, true);
    }
}

void DBusShutdownAgent::UpdateAndShutdown()
{
    qCInfo(DDE_SHELL) << "Update and shutdown";
    if (isUpdating())
        return;

    m_model->setUpdatePowerMode(SessionBaseModel::UPM_UpdateAndShutdown);
    m_model->setCurrentModeState(SessionBaseModel::ModeStatus::ShutDownMode);
    m_model->setVisible(true);
}

void DBusShutdownAgent::Restart()
{
    qCInfo(DDE_SHELL) << "Restart";
    if (isUpdating())
        return;

    if (!canShowShutDown()) {
        //锁屏时允许重启
        emit m_model->onRequirePowerAction(SessionBaseModel::PowerAction::RequireRestart, false);
    } else {
        m_model->setCurrentModeState(SessionBaseModel::ModeStatus::ShutDownMode);
        m_model->setVisible(true);
        emit m_model->onRequirePowerAction(SessionBaseModel::PowerAction::RequireRestart, true);
    }
}

void DBusShutdownAgent::UpdateAndReboot()
{
    qCInfo(DDE_SHELL) << "Update and reboot";
    if (isUpdating())
        return;

    m_model->setUpdatePowerMode(SessionBaseModel::UPM_UpdateAndReboot);
    m_model->setCurrentModeState(SessionBaseModel::ModeStatus::ShutDownMode);
    m_model->setVisible(true);
}

void DBusShutdownAgent::Logout()
{
    qCInfo(DDE_SHELL) << "Logout";
    if (isUpdating() || !canShowShutDown())
        return;

    m_model->setCurrentModeState(SessionBaseModel::ModeStatus::ShutDownMode);
    m_model->setVisible(true);
    emit m_model->onRequirePowerAction(SessionBaseModel::PowerAction::RequireLogout, true);
}

void DBusShutdownAgent::Suspend()
{
    qCInfo(DDE_SHELL) << "Suspend";
    if (isUpdating())
        return;

    if (!canShowShutDown()) {
        // 锁屏时允许待机
        m_model->setPowerAction(SessionBaseModel::RequireSuspend);
    } else {
        m_model->setCurrentModeState(SessionBaseModel::ModeStatus::ShutDownMode);
        m_model->setVisible(true);
        emit m_model->onRequirePowerAction(SessionBaseModel::RequireSuspend, true);
    }
}

void DBusShutdownAgent::Hibernate()
{
    qCInfo(DDE_SHELL) << "Hibernate";
    if (isUpdating())
        return;

    if (!canShowShutDown()) {
        // 锁屏时允许休眠
        m_model->setPowerAction(SessionBaseModel::RequireHibernate);
    } else {
        m_model->setCurrentModeState(SessionBaseModel::ModeStatus::ShutDownMode);
        m_model->setVisible(true);
        emit m_model->onRequirePowerAction(SessionBaseModel::RequireHibernate, true);
    }
}

void DBusShutdownAgent::SwitchUser()
{
    qCInfo(DDE_SHELL) << "Switch user";
    if (isUpdating() || !canShowShutDown())
        return;

    emit m_model->showUserList();
}

void DBusShutdownAgent::Lock()
{
    qCInfo(DDE_SHELL) << "Lock";
    if (isUpdating() || !canShowShutDown())
        return;

    m_model->setCurrentModeState(SessionBaseModel::ModeStatus::ShutDownMode);
    m_model->setVisible(true);
    m_model->setPowerAction(SessionBaseModel::RequireLock);
}

bool DBusShutdownAgent::canShowShutDown() const
{
    // 如果当前界面已显示，而且不是关机模式，则当前已锁屏，因此不允许调用,以免在锁屏时被远程调用而进入桌面
    qCInfo(DDE_SHELL) << "Can show shutdown, visible: " << m_model->visible() << ", mode: " << m_model->currentModeState();

    return !(m_model->visible() && m_model->currentModeState() != SessionBaseModel::ModeStatus::ShutDownMode);
}

/**
 * @brief 如果处于更新状态,那么不响应shutdown接口
 */
bool DBusShutdownAgent::isUpdating() const
{
    qCInfo(DDE_SHELL) << "DBus shutdown agent is updating, current content type: " << m_model->currentContentType();
    return m_model->currentContentType() == SessionBaseModel::UpdateContent;
}

bool DBusShutdownAgent::Visible() const
{
    return m_model->visible() && (m_model->currentModeState() == SessionBaseModel::ModeStatus::PowerMode || m_model->currentModeState() == SessionBaseModel::ModeStatus::ShutDownMode);
}
