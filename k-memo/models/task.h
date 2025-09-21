#ifndef TASK_H
#define TASK_H

#include <QString>
#include <QDateTime>
#include <QStringList>
#include <QJsonObject>
#include <QJsonArray>

enum class TaskPriority {
    Low = 1,
    Normal = 2,
    High = 3,
    Urgent = 4
};

enum class TaskStatus {
    Pending = 0,
    InProgress = 1,
    Completed = 2,
    Cancelled = 3
};

class Task
{
public:
    Task();
    Task(const QString& title, const QString& description = QString());
    
    // Getters
    QString id() const { return m_id; }
    QString title() const { return m_title; }
    QString description() const { return m_description; }
    QDateTime createTime() const { return m_createTime; }
    QDateTime dueTime() const { return m_dueTime; }
    TaskPriority priority() const { return m_priority; }
    TaskStatus status() const { return m_status; }
    QString category() const { return m_category; }
    QStringList tags() const { return m_tags; }
    bool reminderEnabled() const { return m_reminderEnabled; }
    int reminderMinutes() const { return m_reminderMinutes; }
    
    // Setters
    void setId(const QString& id) { m_id = id; }
    void setTitle(const QString& title) { m_title = title; }
    void setDescription(const QString& description) { m_description = description; }
    void setCreateTime(const QDateTime& createTime) { m_createTime = createTime; }
    void setDueTime(const QDateTime& dueTime) { m_dueTime = dueTime; }
    void setPriority(TaskPriority priority) { m_priority = priority; }
    void setStatus(TaskStatus status) { m_status = status; }
    void setCategory(const QString& category) { m_category = category; }
    void setTags(const QStringList& tags) { m_tags = tags; }
    void setReminderEnabled(bool enabled) { m_reminderEnabled = enabled; }
    void setReminderMinutes(int minutes) { m_reminderMinutes = minutes; }
    
    // Utility methods
    bool isValid() const;
    bool isOverdue() const;
    bool isDueToday() const;
    bool isDueSoon(int hours = 24) const;
    bool isValidTransition(TaskStatus newStatus) const;
    bool hasValidReminder() const;
    QString priorityString() const;
    QString statusString() const;
    
    // Serialization
    QJsonObject toJson() const;
    void fromJson(const QJsonObject& json);
    bool isValidJson(const QJsonObject& json) const;
    QString toJsonString() const;
    bool fromJsonString(const QString& jsonStr);
    
    // Operators
    bool operator==(const Task& other) const;
    bool operator!=(const Task& other) const;

private:
    QString m_id;
    QString m_title;
    QString m_description;
    QDateTime m_createTime;
    QDateTime m_dueTime;
    TaskPriority m_priority;
    TaskStatus m_status;
    QString m_category;
    QStringList m_tags;
    bool m_reminderEnabled;
    int m_reminderMinutes;
    
    void generateId();
};

// Helper functions
QString taskPriorityToString(TaskPriority priority);
TaskPriority taskPriorityFromString(const QString& str);
QString taskStatusToString(TaskStatus status);
TaskStatus taskStatusFromString(const QString& str);

#endif // TASK_H
