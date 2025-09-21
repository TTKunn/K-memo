#include "task.h"
#include <QUuid>
#include <QJsonDocument>
#include <QDate>

Task::Task()
    : m_priority(TaskPriority::Normal)
    , m_status(TaskStatus::Pending)
    , m_category("default")
    , m_reminderEnabled(false)
    , m_reminderMinutes(15)
{
    generateId();
    m_createTime = QDateTime::currentDateTime();
}

Task::Task(const QString& title, const QString& description)
    : m_title(title)
    , m_description(description)
    , m_priority(TaskPriority::Normal)
    , m_status(TaskStatus::Pending)
    , m_category("default")
    , m_reminderEnabled(false)
    , m_reminderMinutes(15)
{
    generateId();
    m_createTime = QDateTime::currentDateTime();
}

bool Task::isValid() const
{
    // Check basic required fields
    if (m_id.isEmpty() || m_title.isEmpty()) {
        return false;
    }

    // Check ID format (UUID format: 32 chars + 4 hyphens = 36 total)
    if (m_id.length() != 36) {
        return false;
    }

    // Check title length limit
    if (m_title.length() > 200) {
        return false;
    }

    // Check reminder settings validity
    if (m_reminderEnabled && m_reminderMinutes < 0) {
        return false;
    }

    // Check time logic consistency
    if (m_dueTime.isValid() && m_createTime.isValid()) {
        if (m_dueTime < m_createTime) {
            return false;  // Due time cannot be earlier than creation time
        }
    }

    return true;
}

bool Task::isOverdue() const
{
    if (!m_dueTime.isValid()) {
        return false;
    }
    return m_dueTime < QDateTime::currentDateTime() && m_status != TaskStatus::Completed;
}

bool Task::isDueToday() const
{
    if (!m_dueTime.isValid()) {
        return false;
    }
    return m_dueTime.date() == QDate::currentDate();
}

bool Task::isDueSoon(int hours) const
{
    if (!m_dueTime.isValid()) {
        return false;
    }

    QDateTime currentTime = QDateTime::currentDateTime();
    QDateTime thresholdTime = currentTime.addSecs(hours * 3600); // Convert hours to seconds

    return m_dueTime <= thresholdTime && m_dueTime >= currentTime && m_status != TaskStatus::Completed;
}

bool Task::isValidTransition(TaskStatus newStatus) const
{
    // Allow any transition from Pending
    if (m_status == TaskStatus::Pending) {
        return true;
    }

    // From InProgress, can go to any status
    if (m_status == TaskStatus::InProgress) {
        return true;
    }

    // From Completed, only allow transition to Cancelled or back to InProgress
    if (m_status == TaskStatus::Completed) {
        return newStatus == TaskStatus::Cancelled || newStatus == TaskStatus::InProgress;
    }

    // From Cancelled, only allow transition to Pending or InProgress
    if (m_status == TaskStatus::Cancelled) {
        return newStatus == TaskStatus::Pending || newStatus == TaskStatus::InProgress;
    }

    return false;
}

bool Task::hasValidReminder() const
{
    if (!m_reminderEnabled) {
        return true; // If reminder is disabled, it's considered valid
    }

    // Check if reminder time is reasonable (between 1 minute and 7 days)
    if (m_reminderMinutes < 1 || m_reminderMinutes > 10080) { // 7 days = 7 * 24 * 60 minutes
        return false;
    }

    // If there's a due time, check if reminder is before due time
    if (m_dueTime.isValid()) {
        QDateTime reminderTime = m_dueTime.addSecs(-m_reminderMinutes * 60);
        return reminderTime > QDateTime::currentDateTime();
    }

    return true;
}

QString Task::priorityString() const
{
    return taskPriorityToString(m_priority);
}

QString Task::statusString() const
{
    return taskStatusToString(m_status);
}

QJsonObject Task::toJson() const
{
    QJsonObject obj;
    obj["version"] = "1.0";  // Add version identifier
    obj["id"] = m_id;
    obj["title"] = m_title;
    obj["description"] = m_description;

    // Only serialize valid DateTime
    if (m_createTime.isValid()) {
        obj["createTime"] = m_createTime.toString(Qt::ISODate);
    }
    if (m_dueTime.isValid()) {
        obj["dueTime"] = m_dueTime.toString(Qt::ISODate);
    }

    obj["priority"] = static_cast<int>(m_priority);
    obj["status"] = static_cast<int>(m_status);
    obj["category"] = m_category;
    obj["reminderEnabled"] = m_reminderEnabled;
    obj["reminderMinutes"] = m_reminderMinutes;

    QJsonArray tagsArray;
    for (const QString& tag : m_tags) {
        tagsArray.append(tag);
    }
    obj["tags"] = tagsArray;

    return obj;
}

void Task::fromJson(const QJsonObject& json)
{
    // Validate JSON format
    if (!isValidJson(json)) {
        return;
    }

    // Safely parse basic fields
    m_id = json["id"].toString();
    m_title = json["title"].toString();
    m_description = json["description"].toString();

    // Safely parse DateTime fields
    if (json.contains("createTime")) {
        QString createTimeStr = json["createTime"].toString();
        if (!createTimeStr.isEmpty()) {
            QDateTime dt = QDateTime::fromString(createTimeStr, Qt::ISODate);
            if (dt.isValid()) {
                m_createTime = dt;
            }
        }
    }

    if (json.contains("dueTime")) {
        QString dueTimeStr = json["dueTime"].toString();
        if (!dueTimeStr.isEmpty()) {
            QDateTime dt = QDateTime::fromString(dueTimeStr, Qt::ISODate);
            if (dt.isValid()) {
                m_dueTime = dt;
            }
        }
    }

    // Safely parse enum fields
    if (json.contains("priority")) {
        int priorityInt = json["priority"].toInt(static_cast<int>(TaskPriority::Normal));
        if (priorityInt >= 1 && priorityInt <= 4) {
            m_priority = static_cast<TaskPriority>(priorityInt);
        }
    }

    if (json.contains("status")) {
        int statusInt = json["status"].toInt(static_cast<int>(TaskStatus::Pending));
        if (statusInt >= 0 && statusInt <= 3) {
            m_status = static_cast<TaskStatus>(statusInt);
        }
    }

    // Parse other fields
    if (json.contains("category")) {
        m_category = json["category"].toString();
    }
    if (json.contains("reminderEnabled")) {
        m_reminderEnabled = json["reminderEnabled"].toBool();
    }
    if (json.contains("reminderMinutes")) {
        m_reminderMinutes = json["reminderMinutes"].toInt(15);
    }

    // Safely parse tags array
    m_tags.clear();
    if (json.contains("tags") && json["tags"].isArray()) {
        QJsonArray tagsArray = json["tags"].toArray();
        for (const QJsonValue& value : tagsArray) {
            if (value.isString()) {
                m_tags.append(value.toString());
            }
        }
    }
}

bool Task::isValidJson(const QJsonObject& json) const
{
    // Check required fields
    if (!json.contains("id") || !json.contains("title")) {
        return false;
    }

    // Check basic data types
    if (!json["id"].isString() || !json["title"].isString()) {
        return false;
    }

    // Check enum value ranges
    if (json.contains("priority")) {
        if (!json["priority"].isDouble()) {
            return false;
        }
        int priority = json["priority"].toInt();
        if (priority < 1 || priority > 4) {
            return false;
        }
    }

    if (json.contains("status")) {
        if (!json["status"].isDouble()) {
            return false;
        }
        int status = json["status"].toInt();
        if (status < 0 || status > 3) {
            return false;
        }
    }

    // Check tags array format
    if (json.contains("tags") && !json["tags"].isArray()) {
        return false;
    }

    return true;
}

QString Task::toJsonString() const
{
    QJsonDocument doc(toJson());
    return doc.toJson(QJsonDocument::Compact);
}

bool Task::fromJsonString(const QString& jsonStr)
{
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(jsonStr.toUtf8(), &error);

    if (error.error != QJsonParseError::NoError) {
        return false;
    }

    if (!doc.isObject()) {
        return false;
    }

    QJsonObject obj = doc.object();
    if (!isValidJson(obj)) {
        return false;
    }

    fromJson(obj);
    return true;
}

bool Task::operator==(const Task& other) const
{
    return m_id == other.m_id;
}

bool Task::operator!=(const Task& other) const
{
    return !(*this == other);
}

void Task::generateId()
{
    m_id = QUuid::createUuid().toString(QUuid::WithoutBraces);
}

// Helper functions
QString taskPriorityToString(TaskPriority priority)
{
    switch (priority) {
    case TaskPriority::Low: return "Low";
    case TaskPriority::Normal: return "Normal";
    case TaskPriority::High: return "High";
    case TaskPriority::Urgent: return "Urgent";
    default: return "Normal";
    }
}

TaskPriority taskPriorityFromString(const QString& str)
{
    if (str == "Low") return TaskPriority::Low;
    if (str == "High") return TaskPriority::High;
    if (str == "Urgent") return TaskPriority::Urgent;
    return TaskPriority::Normal;
}

QString taskStatusToString(TaskStatus status)
{
    switch (status) {
    case TaskStatus::Pending: return "Pending";
    case TaskStatus::InProgress: return "In Progress";
    case TaskStatus::Completed: return "Completed";
    case TaskStatus::Cancelled: return "Cancelled";
    default: return "Pending";
    }
}

TaskStatus taskStatusFromString(const QString& str)
{
    if (str == "In Progress") return TaskStatus::InProgress;
    if (str == "Completed") return TaskStatus::Completed;
    if (str == "Cancelled") return TaskStatus::Cancelled;
    return TaskStatus::Pending;
}
