#include "notificationmanager.h"
#include "traymanager.h"
#include "models/task.h"
#include <QDebug>

NotificationManager::NotificationManager(TrayManager* trayManager, QObject *parent)
    : QObject(parent)
    , m_trayManager(trayManager)
    , m_checkTimer(new QTimer(this))
    , m_defaultReminderMinutes(15)
    , m_notificationsEnabled(true)
    , m_soundEnabled(true)
{
    // Setup check timer
    m_checkTimer->setInterval(CHECK_INTERVAL);
    connect(m_checkTimer, &QTimer::timeout, this, &NotificationManager::processNotifications);
    m_checkTimer->start();
}

NotificationManager::~NotificationManager()
{
    m_checkTimer->stop();
}

void NotificationManager::scheduleTaskReminder(const Task& task)
{
    if (!task.reminderEnabled() || !task.dueTime().isValid()) {
        return;
    }
    
    // Remove existing notification for this task
    removeNotification(task.id());
    
    // Calculate reminder time
    QDateTime reminderTime = task.dueTime().addSecs(-task.reminderMinutes() * 60);
    
    if (reminderTime <= QDateTime::currentDateTime()) {
        // If reminder time has passed, show notification immediately
        showTaskDueNotification(task);
        return;
    }
    
    // Create notification item
    NotificationItem item;
    item.taskId = task.id();
    item.title = "Task Reminder";
    item.message = QString("Task '%1' is due in %2 minutes").arg(task.title()).arg(task.reminderMinutes());
    item.scheduledTime = reminderTime;
    item.isRepeating = false;
    item.isActive = true;
    
    addNotification(item);
}

void NotificationManager::cancelTaskReminder(const QString& taskId)
{
    removeNotification(taskId);
}

void NotificationManager::updateTaskReminder(const Task& task)
{
    // Cancel existing reminder and schedule new one
    cancelTaskReminder(task.id());
    scheduleTaskReminder(task);
}

void NotificationManager::showTaskNotification(const QString& title, const QString& message,
                                              QSystemTrayIcon::MessageIcon icon)
{
    if (!m_notificationsEnabled || !m_trayManager) {
        return;
    }
    
    m_trayManager->showMessage(title, message, icon);
    
    if (m_soundEnabled) {
        playNotificationSound();
    }
}

void NotificationManager::showTaskDueNotification(const Task& task)
{
    QString title = "Task Due";
    QString message = QString("Task '%1' is due now!").arg(task.title());
    showTaskNotification(title, message, QSystemTrayIcon::Warning);
    
    emit reminderTriggered(task.id());
}

void NotificationManager::showTaskOverdueNotification(const Task& task)
{
    QString title = "Task Overdue";
    QString message = QString("Task '%1' is overdue!").arg(task.title());
    showTaskNotification(title, message, QSystemTrayIcon::Critical);
}

void NotificationManager::checkPendingNotifications()
{
    processNotifications();
}

void NotificationManager::onTaskUpdated(const Task& task)
{
    updateTaskReminder(task);
}

void NotificationManager::onTaskDeleted(const QString& taskId)
{
    cancelTaskReminder(taskId);
}

void NotificationManager::processNotifications()
{
    QDateTime currentTime = QDateTime::currentDateTime();
    
    for (auto it = m_notifications.begin(); it != m_notifications.end();) {
        NotificationItem& item = *it;
        
        if (!item.isActive) {
            it = m_notifications.erase(it);
            continue;
        }
        
        if (item.scheduledTime <= currentTime) {
            // Show notification
            showTaskNotification(item.title, item.message, QSystemTrayIcon::Information);
            emit notificationShown(item.taskId, item.message);
            
            if (item.isRepeating && item.repeatIntervalMinutes > 0) {
                // Schedule next repeat
                item.scheduledTime = currentTime.addSecs(item.repeatIntervalMinutes * 60);
                ++it;
            } else {
                // Remove one-time notification
                it = m_notifications.erase(it);
            }
        } else {
            ++it;
        }
    }
}

void NotificationManager::addNotification(const NotificationItem& item)
{
    m_notifications.append(item);
}

void NotificationManager::removeNotification(const QString& taskId)
{
    m_notifications.erase(
        std::remove_if(m_notifications.begin(), m_notifications.end(),
                      [&taskId](const NotificationItem& item) {
                          return item.taskId == taskId;
                      }),
        m_notifications.end());
}

NotificationItem* NotificationManager::findNotification(const QString& taskId)
{
    for (auto& item : m_notifications) {
        if (item.taskId == taskId) {
            return &item;
        }
    }
    return nullptr;
}

void NotificationManager::playNotificationSound()
{
    // Placeholder for sound playing
    // Could use QSound or platform-specific APIs
    qDebug() << "Playing notification sound";
}

int NotificationManager::getActiveNotificationCount() const
{
    int count = 0;
    for (const auto& item : m_notifications) {
        if (item.isActive) {
            count++;
        }
    }
    return count;
}
