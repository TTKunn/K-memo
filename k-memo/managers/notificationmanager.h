#ifndef NOTIFICATIONMANAGER_H
#define NOTIFICATIONMANAGER_H

#include <QObject>
#include <QTimer>
#include <QDateTime>
#include <QList>
#include <QSystemTrayIcon>

class Task;
class TrayManager;

struct NotificationItem {
    QString taskId;
    QString title;
    QString message;
    QDateTime scheduledTime;
    bool isRepeating;
    int repeatIntervalMinutes;
    bool isActive;
    
    NotificationItem() : isRepeating(false), repeatIntervalMinutes(0), isActive(true) {}
};

class NotificationManager : public QObject
{
    Q_OBJECT

public:
    explicit NotificationManager(TrayManager* trayManager, QObject *parent = nullptr);
    ~NotificationManager();
    
    // Notification scheduling
    void scheduleTaskReminder(const Task& task);
    void cancelTaskReminder(const QString& taskId);
    void updateTaskReminder(const Task& task);
    
    // Immediate notifications
    void showTaskNotification(const QString& title, const QString& message,
                             QSystemTrayIcon::MessageIcon icon = QSystemTrayIcon::Information);
    void showTaskDueNotification(const Task& task);
    void showTaskOverdueNotification(const Task& task);
    
    // Configuration
    void setDefaultReminderMinutes(int minutes) { m_defaultReminderMinutes = minutes; }
    int getDefaultReminderMinutes() const { return m_defaultReminderMinutes; }
    
    void setNotificationsEnabled(bool enabled) { m_notificationsEnabled = enabled; }
    bool isNotificationsEnabled() const { return m_notificationsEnabled; }
    
    void setSoundEnabled(bool enabled) { m_soundEnabled = enabled; }
    bool isSoundEnabled() const { return m_soundEnabled; }
    
    // Statistics
    int getActiveNotificationCount() const;
    QList<NotificationItem> getScheduledNotifications() const { return m_notifications; }

public slots:
    void checkPendingNotifications();
    void onTaskUpdated(const Task& task);
    void onTaskDeleted(const QString& taskId);

signals:
    void notificationShown(const QString& taskId, const QString& message);
    void reminderTriggered(const QString& taskId);

private slots:
    void processNotifications();

private:
    void addNotification(const NotificationItem& item);
    void removeNotification(const QString& taskId);
    NotificationItem* findNotification(const QString& taskId);
    void playNotificationSound();
    
    TrayManager* m_trayManager;
    QList<NotificationItem> m_notifications;
    QTimer* m_checkTimer;
    
    // Configuration
    int m_defaultReminderMinutes;
    bool m_notificationsEnabled;
    bool m_soundEnabled;
    
    // Check interval (in milliseconds)
    static const int CHECK_INTERVAL = 60000; // 1 minute
};

#endif // NOTIFICATIONMANAGER_H
