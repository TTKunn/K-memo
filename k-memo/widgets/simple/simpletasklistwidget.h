#ifndef SIMPLETASKLISTWIDGET_H
#define SIMPLETASKLISTWIDGET_H

#include <QWidget>
#include <QListWidget>
#include <QListWidgetItem>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QFrame>
#include <QLabel>
#include "../../models/task.h"

QT_BEGIN_NAMESPACE
namespace Ui { class SimpleTaskListWidget; }
QT_END_NAMESPACE

/**
 * @brief 简化的任务列表组件
 *
 * 提供简洁的任务列表显示和基本操作功能。
 * 专注于任务的查看和状态切换，不包含复杂的编辑功能。
 */
class SimpleTaskListWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SimpleTaskListWidget(QWidget *parent = nullptr);
    ~SimpleTaskListWidget();

    /**
     * @brief 设置任务列表
     * @param tasks 任务列表
     */
    void setTasks(const QList<Task> &tasks);

    /**
     * @brief 添加单个任务
     * @param task 要添加的任务
     */
    void addTask(const Task &task);

    /**
     * @brief 移除任务
     * @param taskId 任务ID
     */
    void removeTask(const QString &taskId);

    /**
     * @brief 更新任务
     * @param task 更新后的任务
     */
    void updateTask(const Task &task);

    /**
     * @brief 清空所有任务
     */
    void clearTasks();

    /**
     * @brief 获取任务数量
     * @return 任务总数
     */
    int getTaskCount() const;

    /**
     * @brief 获取已完成任务数量
     * @return 已完成任务数量
     */
    int getCompletedCount() const;

    /**
     * @brief 刷新列表显示
     */
    void refreshList();

public slots:
    /**
     * @brief 根据过滤条件显示任务
     * @param status 任务状态过滤（-1表示显示所有）
     */
    void filterByStatus(int status);

    /**
     * @brief 根据优先级过滤任务
     * @param priority 优先级过滤（-1表示显示所有）
     */
    void filterByPriority(int priority);

    /**
     * @brief 清除所有过滤条件
     */
    void clearFilters();

signals:
    /**
     * @brief 任务状态改变信号
     * @param taskId 任务ID
     * @param completed 是否完成
     */
    void taskStatusChanged(const QString &taskId, bool completed);

    /**
     * @brief 任务被点击信号
     * @param taskId 任务ID
     */
    void taskClicked(const QString &taskId);

    /**
     * @brief 任务被双击信号（进入编辑模式）
     * @param taskId 任务ID
     */
    void taskDoubleClicked(const QString &taskId);

    /**
     * @brief 任务数量变化信号
     * @param total 总任务数
     * @param completed 已完成数
     */
    void taskCountChanged(int total, int completed);

private slots:
    /**
     * @brief 处理任务复选框状态变化
     */
    void onTaskCheckStateChanged();

    /**
     * @brief 处理任务项双击
     * @param item 被双击的项
     */
    void onTaskItemDoubleClicked(QListWidgetItem *item);

private:
    /**
     * @brief 创建任务项Widget
     * @param task 任务对象
     * @return 任务项Widget
     */
    QWidget* createTaskItemWidget(const Task &task);

    /**
     * @brief 获取优先级颜色
     * @param priority 优先级
     * @return 颜色字符串
     */
    QString getPriorityColor(TaskPriority priority) const;

    /**
     * @brief 更新计数显示
     */
    void updateCountDisplay();

    /**
     * @brief 更新空状态显示
     */
    void updateEmptyState();

    /**
     * @brief 应用过滤条件
     */
    void applyFilters();

    /**
     * @brief 任务是否符合过滤条件
     * @param task 任务对象
     * @return 是否符合条件
     */
    bool isTaskVisible(const Task &task) const;

private:
    Ui::SimpleTaskListWidget *ui;
    QList<Task> m_tasks;              // 所有任务列表
    QMap<QString, QListWidgetItem*> m_itemMap;  // 任务ID到列表项的映射

    // 过滤条件
    int m_statusFilter;    // 状态过滤（-1表示不过滤）
    int m_priorityFilter;  // 优先级过滤（-1表示不过滤）
};

#endif // SIMPLETASKLISTWIDGET_H