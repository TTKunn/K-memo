#ifndef TASKDETAILWIDGET_H
#define TASKDETAILWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QTextEdit>
#include <QComboBox>
#include <QPushButton>
#include <QScrollArea>
#include <QFrame>
#include <QDateTimeEdit>
#include "models/task.h"

class TaskDetailWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TaskDetailWidget(QWidget *parent = nullptr);

public slots:
    void setTask(const Task& task);
    void clearTask();

signals:
    void taskUpdated(const Task& task);
    void editTaskRequested(const Task& task);
    void deleteTaskRequested(const Task& task);

private slots:
    void onEditButtonClicked();
    void onDeleteButtonClicked();
    void onPriorityChanged();
    void onStatusChanged();
    void onDescriptionChanged();

private:
    void setupUI();
    void setupConnections();
    void updateTaskDisplay();
    void setNoSelectionState();

    // UI组件
    QScrollArea *m_scrollArea;
    QWidget *m_contentWidget;
    QVBoxLayout *m_contentLayout;

    // 任务信息显示
    QLabel *m_titleLabel;
    QLabel *m_statusLabel;
    QLabel *m_priorityLabel;
    QLabel *m_categoryLabel;
    QLabel *m_createTimeLabel;
    QLabel *m_dueTimeLabel;
    QTextEdit *m_descriptionEdit;

    // 快速编辑控件
    QComboBox *m_priorityCombo;
    QComboBox *m_statusCombo;
    QDateTimeEdit *m_dueTimeEdit;

    // 操作按钮
    QPushButton *m_editButton;
    QPushButton *m_deleteButton;
    QPushButton *m_completeButton;

    // 无选择状态
    QLabel *m_noSelectionLabel;

    // 当前任务
    Task m_currentTask;
    bool m_hasValidTask;
};

#endif // TASKDETAILWIDGET_H