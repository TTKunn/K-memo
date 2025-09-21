#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QString>
#include <QList>
#include "models/task.h"

class DatabaseManager : public QObject
{
    Q_OBJECT

public:
    static DatabaseManager* instance();
    
    bool initialize();
    bool isInitialized() const { return m_initialized; }
    
    // Task operations
    bool insertTask(const Task& task);
    bool updateTask(const Task& task);
    bool deleteTask(const QString& taskId);
    Task getTask(const QString& taskId);
    QList<Task> getAllTasks();
    QList<Task> getTasksByCategory(const QString& category);
    QList<Task> getTasksByStatus(TaskStatus status);
    QList<Task> getTasksByPriority(TaskPriority priority);
    QList<Task> getOverdueTasks();
    QList<Task> getTodayTasks();
    
    // Tag operations
    bool addTagToTask(const QString& taskId, const QString& tag);
    bool removeTagFromTask(const QString& taskId, const QString& tag);
    QStringList getTaskTags(const QString& taskId);
    QStringList getAllTags();
    
    // Category operations
    QStringList getAllCategories();
    int getTaskCountByCategory(const QString& category);
    
    // Statistics
    int getTotalTaskCount();
    int getCompletedTaskCount();
    int getPendingTaskCount();
    
    // Configuration
    bool setConfig(const QString& key, const QString& value);
    QString getConfig(const QString& key, const QString& defaultValue = QString());
    
    // Database maintenance
    bool backup(const QString& backupPath);
    bool restore(const QString& backupPath);
    bool vacuum();

signals:
    void taskInserted(const Task& task);
    void taskUpdated(const Task& task);
    void taskDeleted(const QString& taskId);
    void databaseError(const QString& error);

private:
    explicit DatabaseManager(QObject *parent = nullptr);
    ~DatabaseManager();
    
    bool createTables();
    bool createIndexes();
    bool migrateDatabase(int fromVersion, int toVersion);
    bool executeMigrationStep(int fromVersion, int toVersion);
    bool validateDatabaseIntegrity();
    bool repairDatabase();
    int getDatabaseVersion();
    void setDatabaseVersion(int version);
    
    bool executeQuery(const QString& query, const QVariantList& params = QVariantList());
    QSqlQuery prepareQuery(const QString& query);
    
    static DatabaseManager* m_instance;
    QSqlDatabase m_database;
    bool m_initialized;
    
    static const int DATABASE_VERSION;
    static const QString DATABASE_NAME;
};

#endif // DATABASEMANAGER_H
