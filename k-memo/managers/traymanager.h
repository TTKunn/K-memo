#ifndef TRAYMANAGER_H
#define TRAYMANAGER_H

#include <QObject>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QTimer>

class QMainWindow;

class TrayManager : public QObject
{
    Q_OBJECT

public:
    explicit TrayManager(QMainWindow* mainWindow, QObject *parent = nullptr);
    ~TrayManager();
    
    bool isSystemTrayAvailable() const;
    void show();
    void hide();
    bool isVisible() const;
    
    void updateTaskCount(int pendingCount, int overdueCount);
    void showMessage(const QString& title, const QString& message, 
                    QSystemTrayIcon::MessageIcon icon = QSystemTrayIcon::Information,
                    int msecs = 10000);

public slots:
    void showMainWindow();
    void hideMainWindow();
    void toggleMainWindow();
    void quit();

signals:
    void showRequested();
    void hideRequested();
    void quitRequested();

private slots:
    void onTrayIconActivated(QSystemTrayIcon::ActivationReason reason);
    void onMessageClicked();
    void updateTrayIcon();

private:
    void createTrayIcon();
    void createContextMenu();
    void updateTooltip();
    QIcon createTaskCountIcon(int count);
    
    QMainWindow* m_mainWindow;
    QSystemTrayIcon* m_trayIcon;
    QMenu* m_contextMenu;
    
    // Actions
    QAction* m_showAction;
    QAction* m_hideAction;
    QAction* m_newTaskAction;
    QAction* m_separatorAction;
    QAction* m_quitAction;
    
    // Task counts
    int m_pendingCount;
    int m_overdueCount;
    
    // Icon update timer
    QTimer* m_iconUpdateTimer;
};

#endif // TRAYMANAGER_H
