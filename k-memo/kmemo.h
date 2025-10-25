#ifndef KMEMO_H
#define KMEMO_H

#include <QMainWindow>
#include <QListView>
#include <QComboBox>
#include <QLabel>
#include <QToolBar>
#include <QAction>
#include <QMenu>
#include <QModelIndex>
#include <QStackedWidget>
#include <QPushButton>
#include "models/taskmodel.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class kmemo;
}
QT_END_NAMESPACE

class kmemo : public QMainWindow
{
    Q_OBJECT

public:
    kmemo(QWidget *parent = nullptr);
    ~kmemo();

private slots:
    // 工具栏动作槽函数
    void onAddTaskClicked();
    void onDeleteTaskClicked();
    void onCompleteTaskClicked();

    // 筛选器槽函数
    void onCategoryFilterChanged(int index);
    void onStatusFilterChanged(int index);

    // 任务列表槽函数
    void onTaskSelectionChanged();
    void onTaskDoubleClicked(const QModelIndex &index);
    void onTaskContextMenuRequested(const QPoint &pos);

    // 数据更新槽函数
    void updateStatusBar();
    void refreshTaskList();

    // 新增槽函数
    void onThemeToggleClicked();
    void onToggleDetailPanel();
    void onSearchTextChanged(const QString &text);

    // 自定义组件槽函数（暂时移除）
    // void onTaskCreated(const Task &task);
    // void onTaskDetailRequested(const Task &task);
    // void onTaskDeleteRequested(const Task &task);
    // void onTaskFilterRequested(const QString &category, TaskStatus status);
    // void onShowAllTasksRequested();

    // 新UI槽函数
    void onAdvancedFeaturesClicked();
    void onMenuClicked();
    void onConfirmAddClicked();
    void onCancelAddClicked();
    void onSortTasksClicked();

private:
    void setupUI();
    void setupTaskModel();
    void setupConnections();
    void setupContextMenu();
    void populateFilters();
    void integrateCustomComponents();
    void setupCustomComponentConnections();

    // 新UI相关方法
    void setupSimpleUI();
    void setupSimpleConnections();
    void updateTaskCount();
    void addNewTask(const QString &title);
    void setupFramelessWindow();

    // 内联编辑支持
    void enterInlineEditMode(const QModelIndex &index);
    void exitInlineEditMode();

    Ui::kmemo *ui;

    // 数据模型
    TaskModel *m_taskModel;

    // UI组件引用（来自ui文件）
    QStackedWidget *m_contentStack;
    QListView *m_taskListView;
    QComboBox *m_categoryComboBox;
    QComboBox *m_statusComboBox;
    QPushButton *m_emptyStateButton;

    // 新增UI组件
    QLineEdit *m_searchLineEdit;
    QPushButton *m_themeToggleBtn;
    QPushButton *m_quickAddTaskBtn;
    QFrame *m_leftSidebarPanel;
    QFrame *m_rightDetailPanel;
    QPushButton *m_collapseDetailBtn;
    QLabel *m_taskCountLabel;
    QLabel *m_completedCountLabel;
    QLabel *m_pendingCountLabel;
    QPushButton *m_deleteTaskBtn;
    QPushButton *m_completeTaskBtn;

    // Actions
    QAction *m_actionAddTask;
    QAction *m_actionDeleteTask;
    QAction *m_actionCompleteTask;

    // 右键菜单
    QMenu *m_contextMenu;
    QAction *m_editAction;
    QAction *m_deleteAction;
    QAction *m_completeAction;

    // 编辑状态
    bool m_inlineEditMode;
    QModelIndex m_editingIndex;

    // 面板状态管理
    bool m_rightPanelCollapsed;

    // 自定义组件（暂时移除）
    // TaskDetailWidget *m_taskDetailWidget;
    // TaskStatsWidget *m_taskStatsWidget;
    // QuickAddWidget *m_quickAddWidget;
};
#endif // KMEMO_H
