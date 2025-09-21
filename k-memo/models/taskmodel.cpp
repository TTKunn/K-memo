#include "taskmodel.h"
#include "database/databasemanager.h"
#include <QDebug>
#include <algorithm>

TaskModel::TaskModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_database(DatabaseManager::instance())
    , m_hasFilter(false)
    , m_sortRole(TitleRole)
    , m_sortOrder(Qt::AscendingOrder)
    , m_overdueTimer(new QTimer(this))
{
    // Connect to database signals
    connect(m_database, &DatabaseManager::taskInserted, this, &TaskModel::onTaskInserted);
    connect(m_database, &DatabaseManager::taskUpdated, this, &TaskModel::onTaskUpdated);
    connect(m_database, &DatabaseManager::taskDeleted, this, &TaskModel::onTaskDeleted);
    
    // Setup overdue timer
    m_overdueTimer->setInterval(60000); // Check every minute
    connect(m_overdueTimer, &QTimer::timeout, this, &TaskModel::refreshOverdueStatus);
    m_overdueTimer->start();
    
    // Load initial data
    loadTasks();
}

int TaskModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_tasks.size();
}

QVariant TaskModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_tasks.size()) {
        return QVariant();
    }
    
    const Task& task = m_tasks.at(index.row());
    
    switch (role) {
    case IdRole:
        return task.id();
    case TitleRole:
    case Qt::DisplayRole:
        return task.title();
    case DescriptionRole:
        return task.description();
    case CreateTimeRole:
        return task.createTime();
    case DueTimeRole:
        return task.dueTime();
    case PriorityRole:
        return static_cast<int>(task.priority());
    case StatusRole:
        return static_cast<int>(task.status());
    case CategoryRole:
        return task.category();
    case TagsRole:
        return task.tags();
    case ReminderEnabledRole:
        return task.reminderEnabled();
    case ReminderMinutesRole:
        return task.reminderMinutes();
    case IsOverdueRole:
        return task.isOverdue();
    case IsDueTodayRole:
        return task.isDueToday();
    case PriorityStringRole:
        return task.priorityString();
    case StatusStringRole:
        return task.statusString();
    default:
        return QVariant();
    }
}

bool TaskModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || index.row() >= m_tasks.size()) {
        return false;
    }
    
    Task& task = m_tasks[index.row()];
    bool changed = false;
    
    switch (role) {
    case TitleRole:
        if (task.title() != value.toString()) {
            task.setTitle(value.toString());
            changed = true;
        }
        break;
    case DescriptionRole:
        if (task.description() != value.toString()) {
            task.setDescription(value.toString());
            changed = true;
        }
        break;
    case StatusRole:
        if (static_cast<int>(task.status()) != value.toInt()) {
            task.setStatus(static_cast<TaskStatus>(value.toInt()));
            changed = true;
        }
        break;
    // Add more cases as needed
    default:
        return false;
    }
    
    if (changed) {
        // Update in database first
        if (m_database->updateTask(task)) {
            emit dataChanged(index, index, {role});
        } else {
            qWarning() << "Failed to update task in database";
            return false;
        }
    }
    
    return changed;
}

Qt::ItemFlags TaskModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }
    
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}

QHash<int, QByteArray> TaskModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[IdRole] = "id";
    roles[TitleRole] = "title";
    roles[DescriptionRole] = "description";
    roles[CreateTimeRole] = "createTime";
    roles[DueTimeRole] = "dueTime";
    roles[PriorityRole] = "priority";
    roles[StatusRole] = "status";
    roles[CategoryRole] = "category";
    roles[TagsRole] = "tags";
    roles[ReminderEnabledRole] = "reminderEnabled";
    roles[ReminderMinutesRole] = "reminderMinutes";
    roles[IsOverdueRole] = "isOverdue";
    roles[IsDueTodayRole] = "isDueToday";
    roles[PriorityStringRole] = "priorityString";
    roles[StatusStringRole] = "statusString";
    return roles;
}

bool TaskModel::addTask(const Task& task)
{
    if (!task.isValid()) {
        return false;
    }
    
    // Add to database first
    if (m_database->insertTask(task)) {
        // The model will be updated via the database signal
        return true;
    }
    
    return false;
}

Task TaskModel::getTask(int row) const
{
    if (row >= 0 && row < m_tasks.size()) {
        return m_tasks.at(row);
    }
    return Task();
}

void TaskModel::loadTasks()
{
    beginResetModel();

    // Load tasks based on filter settings
    if (m_hasFilter) {
        QList<Task> allTasks = m_database->getAllTasks();
        m_tasks.clear();

        // Apply filters
        for (const Task& task : allTasks) {
            if (matchesFilter(task)) {
                m_tasks.append(task);
            }
        }
    } else {
        m_tasks = m_database->getAllTasks();
    }

    sortTasks();
    endResetModel();
    emit taskCountChanged();
}

void TaskModel::sortTasks()
{
    std::sort(m_tasks.begin(), m_tasks.end(), [this](const Task& a, const Task& b) {
        bool result = false;

        switch (m_sortRole) {
        case TitleRole:
            result = a.title() < b.title();
            break;
        case CreateTimeRole:
            result = a.createTime() < b.createTime();
            break;
        case DueTimeRole:
            if (a.dueTime().isValid() && b.dueTime().isValid()) {
                result = a.dueTime() < b.dueTime();
            } else if (a.dueTime().isValid()) {
                result = true; // Valid date comes before invalid
            } else {
                result = false; // Invalid date comes after valid
            }
            break;
        case PriorityRole:
            result = static_cast<int>(a.priority()) > static_cast<int>(b.priority()); // Higher priority first
            break;
        case StatusRole:
            result = static_cast<int>(a.status()) < static_cast<int>(b.status());
            break;
        case CategoryRole:
            result = a.category() < b.category();
            break;
        default:
            result = a.title() < b.title(); // Fallback to title sorting
            break;
        }

        return m_sortOrder == Qt::AscendingOrder ? result : !result;
    });
}

void TaskModel::onTaskInserted(const Task& task)
{
    if (matchesFilter(task)) {
        beginInsertRows(QModelIndex(), m_tasks.size(), m_tasks.size());
        m_tasks.append(task);
        endInsertRows();
        emit taskCountChanged();
    }
}

void TaskModel::onTaskUpdated(const Task& task)
{
    int row = findTaskRow(task.id());
    if (row >= 0) {
        m_tasks[row] = task;
        QModelIndex index = this->index(row);
        emit dataChanged(index, index);
    }
}

void TaskModel::onTaskDeleted(const QString& taskId)
{
    int row = findTaskRow(taskId);
    if (row >= 0) {
        beginRemoveRows(QModelIndex(), row, row);
        m_tasks.removeAt(row);
        endRemoveRows();
        emit taskCountChanged();
    }
}

int TaskModel::findTaskRow(const QString& taskId) const
{
    for (int i = 0; i < m_tasks.size(); ++i) {
        if (m_tasks.at(i).id() == taskId) {
            return i;
        }
    }
    return -1;
}

bool TaskModel::matchesFilter(const Task& task) const
{
    if (!m_hasFilter) {
        return true;
    }

    // Apply category filter
    if (!m_filterCategory.isEmpty() && task.category() != m_filterCategory) {
        return false;
    }

    // Apply status filter (if not default pending)
    if (m_hasFilter && task.status() != m_filterStatus && m_filterStatus != TaskStatus::Pending) {
        return false;
    }

    return true;
}

void TaskModel::refreshOverdueStatus()
{
    // Refresh overdue status for all tasks
    for (int i = 0; i < m_tasks.size(); ++i) {
        QModelIndex index = this->index(i);
        emit dataChanged(index, index, {IsOverdueRole, IsDueTodayRole});
    }
}

void TaskModel::refresh()
{
    loadTasks();
}

// Task management implementations
bool TaskModel::updateTask(const Task& task)
{
    if (!task.isValid()) {
        return false;
    }

    // Update in database first
    if (m_database->updateTask(task)) {
        // The model will be updated via the database signal
        return true;
    }

    return false;
}

bool TaskModel::removeTask(const QString& taskId)
{
    if (taskId.isEmpty()) {
        return false;
    }

    // Remove from database first
    if (m_database->deleteTask(taskId)) {
        // The model will be updated via the database signal
        return true;
    }

    return false;
}

bool TaskModel::removeTask(int row)
{
    if (row < 0 || row >= m_tasks.size()) {
        return false;
    }

    QString taskId = m_tasks.at(row).id();
    return removeTask(taskId);
}

Task TaskModel::getTask(const QString& taskId) const
{
    for (const Task& task : m_tasks) {
        if (task.id() == taskId) {
            return task;
        }
    }
    return Task();
}
void TaskModel::setFilter(const QString& category, TaskStatus status)
{
    m_filterCategory = category;
    m_filterStatus = status;
    m_hasFilter = !category.isEmpty() || status != TaskStatus::Pending;

    // Reload and refilter data
    loadTasks();
    emit filterChanged();
}

void TaskModel::clearFilter()
{
    m_filterCategory.clear();
    m_filterStatus = TaskStatus::Pending;
    m_hasFilter = false;

    // Reload all data
    loadTasks();
    emit filterChanged();
}

void TaskModel::setSortOrder(Qt::SortOrder order)
{
    if (m_sortOrder != order) {
        m_sortOrder = order;
        sortTasks();
        emit dataChanged(index(0, 0), index(rowCount() - 1, 0));
    }
}

void TaskModel::setSortRole(TaskRoles role)
{
    if (m_sortRole != role) {
        m_sortRole = role;
        sortTasks();
        emit dataChanged(index(0, 0), index(rowCount() - 1, 0));
    }
}
int TaskModel::getCompletedCount() const
{
    int count = 0;
    for (const Task& task : m_tasks) {
        if (task.status() == TaskStatus::Completed) {
            count++;
        }
    }
    return count;
}

int TaskModel::getPendingCount() const
{
    int count = 0;
    for (const Task& task : m_tasks) {
        if (task.status() == TaskStatus::Pending) {
            count++;
        }
    }
    return count;
}
