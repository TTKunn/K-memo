#include "databasemanager.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>

const int DatabaseManager::DATABASE_VERSION = 1;
const QString DatabaseManager::DATABASE_NAME = "kmemo.db";

DatabaseManager* DatabaseManager::m_instance = nullptr;

DatabaseManager* DatabaseManager::instance()
{
    if (!m_instance) {
        m_instance = new DatabaseManager();
    }
    return m_instance;
}

DatabaseManager::DatabaseManager(QObject *parent)
    : QObject(parent)
    , m_initialized(false)
{
}

DatabaseManager::~DatabaseManager()
{
    if (m_database.isOpen()) {
        m_database.close();
    }
}

bool DatabaseManager::initialize()
{
    if (m_initialized) {
        return true;
    }
    
    // Create data directory if it doesn't exist
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir;
    if (!dir.exists(dataDir)) {
        dir.mkpath(dataDir);
    }
    
    QString dbPath = dataDir + "/" + DATABASE_NAME;
    
    m_database = QSqlDatabase::addDatabase("QSQLITE");
    m_database.setDatabaseName(dbPath);
    
    if (!m_database.open()) {
        qWarning() << "Failed to open database:" << m_database.lastError().text();
        return false;
    }
    
    if (!createTables()) {
        qWarning() << "Failed to create database tables";
        return false;
    }

    if (!createIndexes()) {
        qWarning() << "Failed to create database indexes";
        return false;
    }
    
    // Check database version and migrate if necessary
    int currentVersion = getDatabaseVersion();
    if (currentVersion < DATABASE_VERSION) {
        if (!migrateDatabase(currentVersion, DATABASE_VERSION)) {
            qWarning() << "Failed to migrate database";
            return false;
        }
    }
    
    m_initialized = true;
    return true;
}

bool DatabaseManager::createTables()
{
    QSqlQuery query(m_database);
    
    // Create tasks table
    QString createTasksTable = R"(
        CREATE TABLE IF NOT EXISTS tasks (
            id TEXT PRIMARY KEY,
            title TEXT NOT NULL,
            description TEXT,
            create_time DATETIME DEFAULT CURRENT_TIMESTAMP,
            due_time DATETIME,
            priority INTEGER DEFAULT 2,
            status INTEGER DEFAULT 0,
            category TEXT DEFAULT 'default',
            reminder_enabled BOOLEAN DEFAULT 0,
            reminder_minutes INTEGER DEFAULT 15
        )
    )";
    
    if (!query.exec(createTasksTable)) {
        qWarning() << "Failed to create tasks table:" << query.lastError().text();
        return false;
    }
    
    // Create task_tags table
    QString createTagsTable = R"(
        CREATE TABLE IF NOT EXISTS task_tags (
            task_id TEXT,
            tag TEXT,
            PRIMARY KEY(task_id, tag),
            FOREIGN KEY(task_id) REFERENCES tasks(id) ON DELETE CASCADE
        )
    )";
    
    if (!query.exec(createTagsTable)) {
        qWarning() << "Failed to create task_tags table:" << query.lastError().text();
        return false;
    }
    
    // Create app_config table
    QString createConfigTable = R"(
        CREATE TABLE IF NOT EXISTS app_config (
            key TEXT PRIMARY KEY,
            value TEXT
        )
    )";
    
    if (!query.exec(createConfigTable)) {
        qWarning() << "Failed to create app_config table:" << query.lastError().text();
        return false;
    }
    
    return true;
}

bool DatabaseManager::createIndexes()
{
    QSqlQuery query(m_database);

    // Performance indexes for tasks table
    QStringList indexQueries = {
        "CREATE INDEX IF NOT EXISTS idx_tasks_status ON tasks(status)",
        "CREATE INDEX IF NOT EXISTS idx_tasks_priority ON tasks(priority)",
        "CREATE INDEX IF NOT EXISTS idx_tasks_category ON tasks(category)",
        "CREATE INDEX IF NOT EXISTS idx_tasks_due_time ON tasks(due_time)",
        "CREATE INDEX IF NOT EXISTS idx_tasks_create_time ON tasks(create_time)",
        "CREATE INDEX IF NOT EXISTS idx_tasks_status_due_time ON tasks(status, due_time)",

        // Performance indexes for task_tags table
        "CREATE INDEX IF NOT EXISTS idx_task_tags_task_id ON task_tags(task_id)",
        "CREATE INDEX IF NOT EXISTS idx_task_tags_tag ON task_tags(tag)",

        // Performance indexes for app_config table
        "CREATE INDEX IF NOT EXISTS idx_app_config_key ON app_config(key)"
    };

    for (const QString& indexQuery : indexQueries) {
        if (!query.exec(indexQuery)) {
            qWarning() << "Failed to create index:" << query.lastError().text();
            qWarning() << "Query was:" << indexQuery;
            return false;
        }
    }

    return true;
}

bool DatabaseManager::insertTask(const Task& task)
{
    if (!m_initialized || !task.isValid()) {
        return false;
    }
    
    QSqlQuery query(m_database);
    query.prepare(R"(
        INSERT INTO tasks (id, title, description, create_time, due_time, 
                          priority, status, category, reminder_enabled, reminder_minutes)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
    )");
    
    query.addBindValue(task.id());
    query.addBindValue(task.title());
    query.addBindValue(task.description());
    query.addBindValue(task.createTime());
    query.addBindValue(task.dueTime());
    query.addBindValue(static_cast<int>(task.priority()));
    query.addBindValue(static_cast<int>(task.status()));
    query.addBindValue(task.category());
    query.addBindValue(task.reminderEnabled());
    query.addBindValue(task.reminderMinutes());
    
    if (!query.exec()) {
        qWarning() << "Failed to insert task:" << query.lastError().text();
        return false;
    }
    
    // Insert tags
    for (const QString& tag : task.tags()) {
        addTagToTask(task.id(), tag);
    }
    
    emit taskInserted(task);
    return true;
}

QList<Task> DatabaseManager::getAllTasks()
{
    QList<Task> tasks;
    
    if (!m_initialized) {
        return tasks;
    }
    
    QSqlQuery query(m_database);
    query.prepare("SELECT * FROM tasks ORDER BY create_time DESC");
    
    if (!query.exec()) {
        qWarning() << "Failed to get all tasks:" << query.lastError().text();
        return tasks;
    }
    
    while (query.next()) {
        Task task;
        task.setId(query.value("id").toString());
        task.setTitle(query.value("title").toString());
        task.setDescription(query.value("description").toString());
        task.setCreateTime(query.value("create_time").toDateTime());
        task.setDueTime(query.value("due_time").toDateTime());
        task.setPriority(static_cast<TaskPriority>(query.value("priority").toInt()));
        task.setStatus(static_cast<TaskStatus>(query.value("status").toInt()));
        task.setCategory(query.value("category").toString());
        task.setReminderEnabled(query.value("reminder_enabled").toBool());
        task.setReminderMinutes(query.value("reminder_minutes").toInt());
        
        // Load tags
        task.setTags(getTaskTags(task.id()));
        
        tasks.append(task);
    }
    
    return tasks;
}

bool DatabaseManager::addTagToTask(const QString& taskId, const QString& tag)
{
    if (!m_initialized || taskId.isEmpty() || tag.isEmpty()) {
        return false;
    }
    
    QSqlQuery query(m_database);
    query.prepare("INSERT OR IGNORE INTO task_tags (task_id, tag) VALUES (?, ?)");
    query.addBindValue(taskId);
    query.addBindValue(tag);
    
    return query.exec();
}

QStringList DatabaseManager::getTaskTags(const QString& taskId)
{
    QStringList tags;
    
    if (!m_initialized || taskId.isEmpty()) {
        return tags;
    }
    
    QSqlQuery query(m_database);
    query.prepare("SELECT tag FROM task_tags WHERE task_id = ?");
    query.addBindValue(taskId);
    
    if (query.exec()) {
        while (query.next()) {
            tags.append(query.value("tag").toString());
        }
    }
    
    return tags;
}

int DatabaseManager::getDatabaseVersion()
{
    QSqlQuery query(m_database);
    query.prepare("SELECT value FROM app_config WHERE key = 'database_version'");
    
    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }
    
    return 0; // Default version for new database
}

void DatabaseManager::setDatabaseVersion(int version)
{
    QSqlQuery query(m_database);
    query.prepare("INSERT OR REPLACE INTO app_config (key, value) VALUES ('database_version', ?)");
    query.addBindValue(version);
    query.exec();
}

bool DatabaseManager::migrateDatabase(int fromVersion, int toVersion)
{
    if (fromVersion == toVersion) {
        return true;
    }

    qDebug() << "Migrating database from version" << fromVersion << "to version" << toVersion;

    // Execute migrations step by step
    for (int version = fromVersion; version < toVersion; version++) {
        if (!executeMigrationStep(version, version + 1)) {
            qWarning() << "Failed to migrate from version" << version << "to" << (version + 1);
            return false;
        }
        qDebug() << "Successfully migrated to version" << (version + 1);
    }

    setDatabaseVersion(toVersion);
    qDebug() << "Database migration completed successfully";
    return true;
}

bool DatabaseManager::executeMigrationStep(int fromVersion, int toVersion)
{
    // Define migration steps for each version transition
    switch (fromVersion) {
    case 0:
        // Migration from version 0 to 1 (initial schema creation)
        if (toVersion == 1) {
            // Indexes are already created in createIndexes()
            // Any additional version 1 specific changes would go here
            return true;
        }
        break;

    case 1:
        // Migration from version 1 to 2 (future schema changes)
        if (toVersion == 2) {
            // Example: Add new column to tasks table
            // QSqlQuery query(m_database);
            // query.exec("ALTER TABLE tasks ADD COLUMN new_field TEXT DEFAULT ''");
            return true;
        }
        break;

    // Add more migration cases as needed
    default:
        qWarning() << "No migration path defined from version" << fromVersion << "to" << toVersion;
        return false;
    }

    return false;
}

bool DatabaseManager::validateDatabaseIntegrity()
{
    if (!m_initialized) {
        return false;
    }

    QSqlQuery query(m_database);

    // Check that all required tables exist
    QStringList requiredTables = {"tasks", "task_tags", "app_config"};

    for (const QString& tableName : requiredTables) {
        query.prepare("SELECT name FROM sqlite_master WHERE type='table' AND name=?");
        query.addBindValue(tableName);

        if (!query.exec() || !query.next()) {
            qWarning() << "Required table" << tableName << "is missing";
            return false;
        }
    }

    // Check foreign key constraints
    query.exec("PRAGMA foreign_key_check");
    if (query.next()) {
        qWarning() << "Foreign key constraint violations detected";
        return false;
    }

    // Check for orphaned records in task_tags
    query.exec(R"(
        SELECT COUNT(*) FROM task_tags tt
        LEFT JOIN tasks t ON tt.task_id = t.id
        WHERE t.id IS NULL
    )");

    if (query.next() && query.value(0).toInt() > 0) {
        qWarning() << "Orphaned task_tags records detected";
        return false;
    }

    return true;
}

bool DatabaseManager::repairDatabase()
{
    if (!m_initialized) {
        return false;
    }

    QSqlQuery query(m_database);

    // Remove orphaned task_tags records
    query.exec(R"(
        DELETE FROM task_tags
        WHERE task_id NOT IN (SELECT id FROM tasks)
    )");

    if (query.numRowsAffected() > 0) {
        qDebug() << "Removed" << query.numRowsAffected() << "orphaned task_tags records";
    }

    // Recreate indexes if missing
    if (!createIndexes()) {
        qWarning() << "Failed to recreate indexes during repair";
        return false;
    }

    // Run database integrity check
    query.exec("PRAGMA integrity_check");
    if (query.next()) {
        QString result = query.value(0).toString();
        if (result != "ok") {
            qWarning() << "Database integrity check failed:" << result;
            return false;
        }
    }

    return true;
}

// CRUD method implementations
bool DatabaseManager::updateTask(const Task& task)
{
    if (!m_initialized || !task.isValid()) {
        return false;
    }

    QSqlQuery query(m_database);
    query.prepare(R"(
        UPDATE tasks SET
            title = ?, description = ?, due_time = ?,
            priority = ?, status = ?, category = ?,
            reminder_enabled = ?, reminder_minutes = ?
        WHERE id = ?
    )");

    query.addBindValue(task.title());
    query.addBindValue(task.description());
    query.addBindValue(task.dueTime());
    query.addBindValue(static_cast<int>(task.priority()));
    query.addBindValue(static_cast<int>(task.status()));
    query.addBindValue(task.category());
    query.addBindValue(task.reminderEnabled());
    query.addBindValue(task.reminderMinutes());
    query.addBindValue(task.id());

    if (!query.exec()) {
        qWarning() << "Failed to update task:" << query.lastError().text();
        return false;
    }

    // Update tags - remove old ones and add new ones
    QSqlQuery deleteTagsQuery(m_database);
    deleteTagsQuery.prepare("DELETE FROM task_tags WHERE task_id = ?");
    deleteTagsQuery.addBindValue(task.id());
    deleteTagsQuery.exec();

    // Add new tags
    for (const QString& tag : task.tags()) {
        addTagToTask(task.id(), tag);
    }

    emit taskUpdated(task);
    return true;
}

bool DatabaseManager::deleteTask(const QString& taskId)
{
    if (!m_initialized || taskId.isEmpty()) {
        return false;
    }

    QSqlQuery query(m_database);
    query.prepare("DELETE FROM tasks WHERE id = ?");
    query.addBindValue(taskId);

    if (!query.exec()) {
        qWarning() << "Failed to delete task:" << query.lastError().text();
        return false;
    }

    // Tags will be automatically deleted due to CASCADE foreign key
    emit taskDeleted(taskId);
    return true;
}

Task DatabaseManager::getTask(const QString& taskId)
{
    Task task;

    if (!m_initialized || taskId.isEmpty()) {
        return task;
    }

    QSqlQuery query(m_database);
    query.prepare("SELECT * FROM tasks WHERE id = ?");
    query.addBindValue(taskId);

    if (!query.exec() || !query.next()) {
        return task;
    }

    task.setId(query.value("id").toString());
    task.setTitle(query.value("title").toString());
    task.setDescription(query.value("description").toString());
    task.setCreateTime(query.value("create_time").toDateTime());
    task.setDueTime(query.value("due_time").toDateTime());
    task.setPriority(static_cast<TaskPriority>(query.value("priority").toInt()));
    task.setStatus(static_cast<TaskStatus>(query.value("status").toInt()));
    task.setCategory(query.value("category").toString());
    task.setReminderEnabled(query.value("reminder_enabled").toBool());
    task.setReminderMinutes(query.value("reminder_minutes").toInt());

    // Load tags
    task.setTags(getTaskTags(task.id()));

    return task;
}
QList<Task> DatabaseManager::getTasksByCategory(const QString& category)
{
    QList<Task> tasks;

    if (!m_initialized || category.isEmpty()) {
        return tasks;
    }

    QSqlQuery query(m_database);
    query.prepare("SELECT * FROM tasks WHERE category = ? ORDER BY create_time DESC");
    query.addBindValue(category);

    if (!query.exec()) {
        qWarning() << "Failed to get tasks by category:" << query.lastError().text();
        return tasks;
    }

    while (query.next()) {
        Task task;
        task.setId(query.value("id").toString());
        task.setTitle(query.value("title").toString());
        task.setDescription(query.value("description").toString());
        task.setCreateTime(query.value("create_time").toDateTime());
        task.setDueTime(query.value("due_time").toDateTime());
        task.setPriority(static_cast<TaskPriority>(query.value("priority").toInt()));
        task.setStatus(static_cast<TaskStatus>(query.value("status").toInt()));
        task.setCategory(query.value("category").toString());
        task.setReminderEnabled(query.value("reminder_enabled").toBool());
        task.setReminderMinutes(query.value("reminder_minutes").toInt());
        task.setTags(getTaskTags(task.id()));

        tasks.append(task);
    }

    return tasks;
}

QList<Task> DatabaseManager::getTasksByStatus(TaskStatus status)
{
    QList<Task> tasks;

    if (!m_initialized) {
        return tasks;
    }

    QSqlQuery query(m_database);
    query.prepare("SELECT * FROM tasks WHERE status = ? ORDER BY create_time DESC");
    query.addBindValue(static_cast<int>(status));

    if (!query.exec()) {
        qWarning() << "Failed to get tasks by status:" << query.lastError().text();
        return tasks;
    }

    while (query.next()) {
        Task task;
        task.setId(query.value("id").toString());
        task.setTitle(query.value("title").toString());
        task.setDescription(query.value("description").toString());
        task.setCreateTime(query.value("create_time").toDateTime());
        task.setDueTime(query.value("due_time").toDateTime());
        task.setPriority(static_cast<TaskPriority>(query.value("priority").toInt()));
        task.setStatus(static_cast<TaskStatus>(query.value("status").toInt()));
        task.setCategory(query.value("category").toString());
        task.setReminderEnabled(query.value("reminder_enabled").toBool());
        task.setReminderMinutes(query.value("reminder_minutes").toInt());
        task.setTags(getTaskTags(task.id()));

        tasks.append(task);
    }

    return tasks;
}

QList<Task> DatabaseManager::getTasksByPriority(TaskPriority priority)
{
    QList<Task> tasks;

    if (!m_initialized) {
        return tasks;
    }

    QSqlQuery query(m_database);
    query.prepare("SELECT * FROM tasks WHERE priority = ? ORDER BY create_time DESC");
    query.addBindValue(static_cast<int>(priority));

    if (!query.exec()) {
        qWarning() << "Failed to get tasks by priority:" << query.lastError().text();
        return tasks;
    }

    while (query.next()) {
        Task task;
        task.setId(query.value("id").toString());
        task.setTitle(query.value("title").toString());
        task.setDescription(query.value("description").toString());
        task.setCreateTime(query.value("create_time").toDateTime());
        task.setDueTime(query.value("due_time").toDateTime());
        task.setPriority(static_cast<TaskPriority>(query.value("priority").toInt()));
        task.setStatus(static_cast<TaskStatus>(query.value("status").toInt()));
        task.setCategory(query.value("category").toString());
        task.setReminderEnabled(query.value("reminder_enabled").toBool());
        task.setReminderMinutes(query.value("reminder_minutes").toInt());
        task.setTags(getTaskTags(task.id()));

        tasks.append(task);
    }

    return tasks;
}

QList<Task> DatabaseManager::getOverdueTasks()
{
    QList<Task> tasks;

    if (!m_initialized) {
        return tasks;
    }

    QSqlQuery query(m_database);
    query.prepare(R"(
        SELECT * FROM tasks
        WHERE due_time IS NOT NULL
        AND due_time < datetime('now')
        AND status != ?
        ORDER BY due_time ASC
    )");
    query.addBindValue(static_cast<int>(TaskStatus::Completed));

    if (!query.exec()) {
        qWarning() << "Failed to get overdue tasks:" << query.lastError().text();
        return tasks;
    }

    while (query.next()) {
        Task task;
        task.setId(query.value("id").toString());
        task.setTitle(query.value("title").toString());
        task.setDescription(query.value("description").toString());
        task.setCreateTime(query.value("create_time").toDateTime());
        task.setDueTime(query.value("due_time").toDateTime());
        task.setPriority(static_cast<TaskPriority>(query.value("priority").toInt()));
        task.setStatus(static_cast<TaskStatus>(query.value("status").toInt()));
        task.setCategory(query.value("category").toString());
        task.setReminderEnabled(query.value("reminder_enabled").toBool());
        task.setReminderMinutes(query.value("reminder_minutes").toInt());
        task.setTags(getTaskTags(task.id()));

        tasks.append(task);
    }

    return tasks;
}

QList<Task> DatabaseManager::getTodayTasks()
{
    QList<Task> tasks;

    if (!m_initialized) {
        return tasks;
    }

    QSqlQuery query(m_database);
    query.prepare(R"(
        SELECT * FROM tasks
        WHERE due_time IS NOT NULL
        AND date(due_time) = date('now')
        ORDER BY due_time ASC
    )");

    if (!query.exec()) {
        qWarning() << "Failed to get today tasks:" << query.lastError().text();
        return tasks;
    }

    while (query.next()) {
        Task task;
        task.setId(query.value("id").toString());
        task.setTitle(query.value("title").toString());
        task.setDescription(query.value("description").toString());
        task.setCreateTime(query.value("create_time").toDateTime());
        task.setDueTime(query.value("due_time").toDateTime());
        task.setPriority(static_cast<TaskPriority>(query.value("priority").toInt()));
        task.setStatus(static_cast<TaskStatus>(query.value("status").toInt()));
        task.setCategory(query.value("category").toString());
        task.setReminderEnabled(query.value("reminder_enabled").toBool());
        task.setReminderMinutes(query.value("reminder_minutes").toInt());
        task.setTags(getTaskTags(task.id()));

        tasks.append(task);
    }

    return tasks;
}
bool DatabaseManager::removeTagFromTask(const QString& taskId, const QString& tag)
{
    if (!m_initialized || taskId.isEmpty() || tag.isEmpty()) {
        return false;
    }

    QSqlQuery query(m_database);
    query.prepare("DELETE FROM task_tags WHERE task_id = ? AND tag = ?");
    query.addBindValue(taskId);
    query.addBindValue(tag);

    return query.exec();
}

QStringList DatabaseManager::getAllTags()
{
    QStringList tags;

    if (!m_initialized) {
        return tags;
    }

    QSqlQuery query(m_database);
    query.prepare("SELECT DISTINCT tag FROM task_tags ORDER BY tag");

    if (query.exec()) {
        while (query.next()) {
            tags.append(query.value("tag").toString());
        }
    }

    return tags;
}

QStringList DatabaseManager::getAllCategories()
{
    QStringList categories;

    if (!m_initialized) {
        return categories;
    }

    QSqlQuery query(m_database);
    query.prepare("SELECT DISTINCT category FROM tasks ORDER BY category");

    if (query.exec()) {
        while (query.next()) {
            categories.append(query.value("category").toString());
        }
    }

    return categories;
}

int DatabaseManager::getTaskCountByCategory(const QString& category)
{
    if (!m_initialized || category.isEmpty()) {
        return 0;
    }

    QSqlQuery query(m_database);
    query.prepare("SELECT COUNT(*) FROM tasks WHERE category = ?");
    query.addBindValue(category);

    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }

    return 0;
}

int DatabaseManager::getTotalTaskCount()
{
    if (!m_initialized) {
        return 0;
    }

    QSqlQuery query(m_database);
    query.prepare("SELECT COUNT(*) FROM tasks");

    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }

    return 0;
}

int DatabaseManager::getCompletedTaskCount()
{
    if (!m_initialized) {
        return 0;
    }

    QSqlQuery query(m_database);
    query.prepare("SELECT COUNT(*) FROM tasks WHERE status = ?");
    query.addBindValue(static_cast<int>(TaskStatus::Completed));

    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }

    return 0;
}

int DatabaseManager::getPendingTaskCount()
{
    if (!m_initialized) {
        return 0;
    }

    QSqlQuery query(m_database);
    query.prepare("SELECT COUNT(*) FROM tasks WHERE status = ?");
    query.addBindValue(static_cast<int>(TaskStatus::Pending));

    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }

    return 0;
}
bool DatabaseManager::setConfig(const QString& key, const QString& value)
{
    if (!m_initialized || key.isEmpty()) {
        return false;
    }

    QSqlQuery query(m_database);
    query.prepare("INSERT OR REPLACE INTO app_config (key, value) VALUES (?, ?)");
    query.addBindValue(key);
    query.addBindValue(value);

    return query.exec();
}

QString DatabaseManager::getConfig(const QString& key, const QString& defaultValue)
{
    if (!m_initialized || key.isEmpty()) {
        return defaultValue;
    }

    QSqlQuery query(m_database);
    query.prepare("SELECT value FROM app_config WHERE key = ?");
    query.addBindValue(key);

    if (query.exec() && query.next()) {
        return query.value("value").toString();
    }

    return defaultValue;
}

bool DatabaseManager::backup(const QString& backupPath)
{
    if (!m_initialized || backupPath.isEmpty()) {
        return false;
    }

    // SQLite database file backup
    QSqlQuery query(m_database);
    QString backupQuery = QString("BACKUP TO '%1'").arg(backupPath);

    // For SQLite, we'll use a more standard approach
    query.prepare("VACUUM INTO ?");
    query.addBindValue(backupPath);

    return query.exec();
}

bool DatabaseManager::restore(const QString& backupPath)
{
    if (!m_initialized || backupPath.isEmpty()) {
        return false;
    }

    // Close current database
    if (m_database.isOpen()) {
        m_database.close();
    }

    // Copy backup file to current database location
    QString currentDbPath = m_database.databaseName();

    // This is a simplified restore - in production, you'd want more robust file operations
    QSqlQuery query(m_database);
    query.prepare("RESTORE FROM ?");
    query.addBindValue(backupPath);

    return query.exec();
}

bool DatabaseManager::vacuum()
{
    if (!m_initialized) {
        return false;
    }

    QSqlQuery query(m_database);
    return query.exec("VACUUM");
}
