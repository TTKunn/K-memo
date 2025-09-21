#ifndef TASKMODEL_H
#define TASKMODEL_H

#include <QAbstractListModel>
#include <QList>
#include <QTimer>
#include "task.h"

class DatabaseManager;

class TaskModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum TaskRoles {
        IdRole = Qt::UserRole + 1,
        TitleRole,
        DescriptionRole,
        CreateTimeRole,
        DueTimeRole,
        PriorityRole,
        StatusRole,
        CategoryRole,
        TagsRole,
        ReminderEnabledRole,
        ReminderMinutesRole,
        IsOverdueRole,
        IsDueTodayRole,
        PriorityStringRole,
        StatusStringRole
    };

    explicit TaskModel(QObject *parent = nullptr);
    
    // QAbstractListModel interface
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QHash<int, QByteArray> roleNames() const override;
    
    // Task management
    bool addTask(const Task& task);
    bool updateTask(const Task& task);
    bool removeTask(const QString& taskId);
    bool removeTask(int row);
    Task getTask(int row) const;
    Task getTask(const QString& taskId) const;
    int findTaskRow(const QString& taskId) const;
    
    // Filtering and sorting
    void setFilter(const QString& category = QString(), TaskStatus status = TaskStatus::Pending);
    void clearFilter();
    void setSortOrder(Qt::SortOrder order);
    void setSortRole(TaskRoles role);
    
    // Data access
    QList<Task> getAllTasks() const { return m_tasks; }
    int getTaskCount() const { return m_tasks.size(); }
    int getCompletedCount() const;
    int getPendingCount() const;
    
    // Refresh data
    void refresh();

public slots:
    void onTaskInserted(const Task& task);
    void onTaskUpdated(const Task& task);
    void onTaskDeleted(const QString& taskId);

signals:
    void taskCountChanged();
    void filterChanged();

private slots:
    void refreshOverdueStatus();

private:
    void loadTasks();
    void sortTasks();
    bool matchesFilter(const Task& task) const;
    
    QList<Task> m_tasks;
    DatabaseManager* m_database;
    
    // Filtering
    QString m_filterCategory;
    TaskStatus m_filterStatus;
    bool m_hasFilter;
    
    // Sorting
    TaskRoles m_sortRole;
    Qt::SortOrder m_sortOrder;
    
    // Timer for updating overdue status
    QTimer* m_overdueTimer;
};

#endif // TASKMODEL_H
