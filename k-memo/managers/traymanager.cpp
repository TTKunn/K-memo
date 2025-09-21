#include "traymanager.h"
#include <QMainWindow>
#include <QApplication>
#include <QIcon>
#include <QPainter>
#include <QFont>

TrayManager::TrayManager(QMainWindow* mainWindow, QObject *parent)
    : QObject(parent)
    , m_mainWindow(mainWindow)
    , m_trayIcon(nullptr)
    , m_contextMenu(nullptr)
    , m_pendingCount(0)
    , m_overdueCount(0)
    , m_iconUpdateTimer(new QTimer(this))
{
    if (isSystemTrayAvailable()) {
        createTrayIcon();
        createContextMenu();
        
        // Setup icon update timer
        m_iconUpdateTimer->setSingleShot(true);
        m_iconUpdateTimer->setInterval(100);
        connect(m_iconUpdateTimer, &QTimer::timeout, this, &TrayManager::updateTrayIcon);
    }
}

TrayManager::~TrayManager()
{
    if (m_trayIcon) {
        m_trayIcon->hide();
    }
}

bool TrayManager::isSystemTrayAvailable() const
{
    return QSystemTrayIcon::isSystemTrayAvailable();
}

void TrayManager::show()
{
    if (m_trayIcon) {
        m_trayIcon->show();
    }
}

void TrayManager::hide()
{
    if (m_trayIcon) {
        m_trayIcon->hide();
    }
}

bool TrayManager::isVisible() const
{
    return m_trayIcon && m_trayIcon->isVisible();
}

void TrayManager::updateTaskCount(int pendingCount, int overdueCount)
{
    if (m_pendingCount != pendingCount || m_overdueCount != overdueCount) {
        m_pendingCount = pendingCount;
        m_overdueCount = overdueCount;
        
        updateTooltip();
        
        // Delay icon update to avoid frequent updates
        m_iconUpdateTimer->start();
    }
}

void TrayManager::showMessage(const QString& title, const QString& message, 
                             QSystemTrayIcon::MessageIcon icon, int msecs)
{
    if (m_trayIcon && m_trayIcon->isVisible()) {
        m_trayIcon->showMessage(title, message, icon, msecs);
    }
}

void TrayManager::createTrayIcon()
{
    m_trayIcon = new QSystemTrayIcon(this);
    
    // Set initial icon
    updateTrayIcon();
    
    // Connect signals
    connect(m_trayIcon, &QSystemTrayIcon::activated, 
            this, &TrayManager::onTrayIconActivated);
    connect(m_trayIcon, &QSystemTrayIcon::messageClicked,
            this, &TrayManager::onMessageClicked);
}

void TrayManager::createContextMenu()
{
    m_contextMenu = new QMenu();
    
    m_showAction = m_contextMenu->addAction("Show", this, &TrayManager::showMainWindow);
    m_hideAction = m_contextMenu->addAction("Hide", this, &TrayManager::hideMainWindow);
    m_contextMenu->addSeparator();
    m_newTaskAction = m_contextMenu->addAction("New Task", [this]() {
        showMainWindow();
        // Emit signal for new task creation
    });
    m_contextMenu->addSeparator();
    m_quitAction = m_contextMenu->addAction("Quit", this, &TrayManager::quit);
    
    if (m_trayIcon) {
        m_trayIcon->setContextMenu(m_contextMenu);
    }
}

void TrayManager::updateTooltip()
{
    if (!m_trayIcon) return;
    
    QString tooltip = "K-memo";
    if (m_pendingCount > 0 || m_overdueCount > 0) {
        tooltip += QString("\nPending: %1").arg(m_pendingCount);
        if (m_overdueCount > 0) {
            tooltip += QString("\nOverdue: %1").arg(m_overdueCount);
        }
    }
    
    m_trayIcon->setToolTip(tooltip);
}

QIcon TrayManager::createTaskCountIcon(int count)
{
    // Create a simple icon with task count
    QPixmap pixmap(16, 16);
    pixmap.fill(Qt::transparent);
    
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Draw background circle
    QColor bgColor = (m_overdueCount > 0) ? Qt::red : Qt::blue;
    painter.setBrush(bgColor);
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(0, 0, 16, 16);
    
    // Draw count text
    if (count > 0) {
        painter.setPen(Qt::white);
        painter.setFont(QFont("Arial", 8, QFont::Bold));
        QString text = (count > 99) ? "99+" : QString::number(count);
        painter.drawText(pixmap.rect(), Qt::AlignCenter, text);
    }
    
    return QIcon(pixmap);
}

void TrayManager::updateTrayIcon()
{
    if (!m_trayIcon) return;
    
    int totalCount = m_pendingCount + m_overdueCount;
    QIcon icon = createTaskCountIcon(totalCount);
    m_trayIcon->setIcon(icon);
}

void TrayManager::onTrayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason) {
    case QSystemTrayIcon::DoubleClick:
        toggleMainWindow();
        break;
    case QSystemTrayIcon::Trigger:
        // Single click - could show a quick preview
        break;
    default:
        break;
    }
}

void TrayManager::onMessageClicked()
{
    showMainWindow();
}

void TrayManager::showMainWindow()
{
    if (m_mainWindow) {
        m_mainWindow->show();
        m_mainWindow->raise();
        m_mainWindow->activateWindow();
    }
    emit showRequested();
}

void TrayManager::hideMainWindow()
{
    if (m_mainWindow) {
        m_mainWindow->hide();
    }
    emit hideRequested();
}

void TrayManager::toggleMainWindow()
{
    if (m_mainWindow) {
        if (m_mainWindow->isVisible() && m_mainWindow->isActiveWindow()) {
            hideMainWindow();
        } else {
            showMainWindow();
        }
    }
}

void TrayManager::quit()
{
    emit quitRequested();
    QApplication::quit();
}
