#ifndef TASKSTATSWIDGET_H
#define TASKSTATSWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>
#include <QFrame>
#include <QTimer>
#include "models/task.h"

class TaskModel;

class TaskStatsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TaskStatsWidget(QWidget *parent = nullptr);

    void setTaskModel(TaskModel* model);
    void refreshStats();

public slots:
    void onTaskModelChanged();

signals:
    void filterRequested(const QString& category, TaskStatus status);
    void showAllTasksRequested();

private slots:
    void onShowAllClicked();
    void onShowPendingClicked();
    void onShowInProgressClicked();
    void onShowCompletedClicked();
    void onShowOverdueClicked();
    void updateProgress();

private:
    void setupUI();
    void setupConnections();
    void updateStatistics();
    void updateProgressBars();

    // UI Components
    QVBoxLayout* m_mainLayout;

    // Stats display
    QFrame* m_statsFrame;
    QLabel* m_totalTasksLabel;
    QLabel* m_completedTasksLabel;
    QLabel* m_pendingTasksLabel;
    QLabel* m_overdueTasksLabel;

    // Progress indicators
    QFrame* m_progressFrame;
    QProgressBar* m_completionProgress;
    QLabel* m_completionLabel;
    QProgressBar* m_overdueProgress;
    QLabel* m_overdueLabel;

    // Quick filter buttons
    QFrame* m_filterFrame;
    QPushButton* m_showAllBtn;
    QPushButton* m_showPendingBtn;
    QPushButton* m_showInProgressBtn;
    QPushButton* m_showCompletedBtn;
    QPushButton* m_showOverdueBtn;

    // Categories section
    QFrame* m_categoriesFrame;
    QVBoxLayout* m_categoriesLayout;

    // Data
    TaskModel* m_taskModel;
    QTimer* m_refreshTimer;

    // Statistics
    int m_totalTasks;
    int m_completedTasks;
    int m_pendingTasks;
    int m_inProgressTasks;
    int m_overdueTasks;

    // Helper methods
    QFrame* createStatsCard(const QString& title, const QString& value, const QString& color);
    QPushButton* createFilterButton(const QString& text, const QString& color);
    void updateCategoriesDisplay();
};

#endif // TASKSTATSWIDGET_H