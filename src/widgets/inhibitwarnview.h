// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef INHIBITWARNVIEW_H
#define INHIBITWARNVIEW_H

#include "warningview.h"
#include "rounditembutton.h"
#include "sessionbasemodel.h"
#include "inhibitbutton.h"

#include <QWidget>

class InhibitorRow : public QWidget
{
    Q_OBJECT
public:
    InhibitorRow(const QString &who, const QString &why, const QIcon &icon = QIcon(), QWidget *parent = nullptr);
    ~InhibitorRow() override;

protected:
    void paintEvent(QPaintEvent* event) override;
};

class InhibitWarnView : public WarningView
{
    Q_OBJECT

public:
    explicit InhibitWarnView(SessionBaseModel::PowerAction inhibitType, QWidget *parent = nullptr);
    ~InhibitWarnView() override;

    struct InhibitorData {
        QString who;
        QString why;
        QString mode;
        quint32 pid;
        QString icon;
    };

    void setInhibitorList(const QList<InhibitorData> & list);
    void setInhibitConfirmMessage(const QString &text);
    void setAcceptReason(const QString &reason) override;
    void setAcceptVisible(const bool acceptable);


protected:
    QString iconString();
    bool focusNextPrevChild(bool next) Q_DECL_OVERRIDE;
    void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;

signals:
    void cancelled() const;
    void actionInvoked() const;

private:
    SessionBaseModel::PowerAction m_inhibitType;
    QList<QWidget*> m_inhibitorPtrList;
    QVBoxLayout *m_inhibitorListLayout = nullptr;
    QLabel *m_confirmTextLabel = nullptr;
    InhibitButton *m_acceptBtn;
    InhibitButton *m_cancelBtn;
    int m_dataBindIndex;
};

#endif // INHIBITWARNVIEW_H
