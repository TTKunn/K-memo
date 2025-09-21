#include "kmemo.h"
#include "./ui_kmemo.h"
#include "IconManager.h"
#include <QMessageBox>
#include <QInputDialog>
#include <QHeaderView>
#include <QApplication>
#include <algorithm>

kmemo::kmemo(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::kmemo)
    , m_taskModel(nullptr)
    , m_inlineEditMode(false)
{
    ui->setupUi(this);
    setupUI();
    setupTaskModel();
    setupConnections();
    setupContextMenu();
    populateFilters();

    // Initialize status bar
    updateStatusBar();
}

kmemo::~kmemo()
{
    delete ui;
}

void kmemo::setupUI()
{
    // Get UI component references
    m_contentStack = ui->contentStack;
    m_taskListView = ui->taskListView;
    m_categoryComboBox = ui->categoryComboBox;
    m_statusComboBox = ui->statusComboBox;
    m_emptyStateButton = ui->emptyStateButton;

    // 新增组件引用
    m_searchLineEdit = ui->searchLineEdit;
    m_themeToggleBtn = ui->themeToggleBtn;
    m_quickAddTaskBtn = ui->quickAddTaskBtn;
    m_leftSidebarPanel = ui->leftSidebarPanel;
    m_rightDetailPanel = ui->rightDetailPanel;
    m_collapseDetailBtn = ui->collapseDetailBtn;
    m_taskCountLabel = ui->taskCountLabel;
    m_completedCountLabel = ui->completedCountLabel;
    m_pendingCountLabel = ui->pendingCountLabel;
    m_deleteTaskBtn = ui->deleteTaskBtn;
    m_completeTaskBtn = ui->completeTaskBtn;

    // Get Actions
    m_actionAddTask = ui->actionAddTask;
    m_actionDeleteTask = ui->actionDeleteTask;
    m_actionCompleteTask = ui->actionCompleteTask;

    // 初始化面板状态
    m_rightPanelCollapsed = false;

    // 初始化自定义组件
    m_taskDetailWidget = new TaskDetailWidget(this);
    m_taskStatsWidget = new TaskStatsWidget(this);
    m_quickAddWidget = new QuickAddWidget(this);

    // Configure task list view
    m_taskListView->setAlternatingRowColors(false);
    m_taskListView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_taskListView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    m_taskListView->setUniformItemSizes(false);
    m_taskListView->setContextMenuPolicy(Qt::CustomContextMenu);

    // 将自定义组件集成到相应的面板中
    integrateCustomComponents();
}

void kmemo::setupTaskModel()
{
    m_taskModel = new TaskModel(this);
    m_taskListView->setModel(m_taskModel);

    // 设置TaskModel到自定义组件
    if (m_taskStatsWidget) {
        m_taskStatsWidget->setTaskModel(m_taskModel);
    }
}

void kmemo::setupConnections()
{
    // Toolbar action connections
    connect(m_actionAddTask, &QAction::triggered, this, &kmemo::onAddTaskClicked);
    connect(m_actionDeleteTask, &QAction::triggered, this, &kmemo::onDeleteTaskClicked);
    connect(m_actionCompleteTask, &QAction::triggered, this, &kmemo::onCompleteTaskClicked);

    // Filter connections
    connect(m_categoryComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &kmemo::onCategoryFilterChanged);
    connect(m_statusComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &kmemo::onStatusFilterChanged);

    // Task list connections
    connect(m_taskListView, &QListView::doubleClicked, this, &kmemo::onTaskDoubleClicked);
    connect(m_taskListView, &QListView::customContextMenuRequested,
            this, &kmemo::onTaskContextMenuRequested);
    connect(m_taskListView->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &kmemo::onTaskSelectionChanged);

    // Model data connections
    connect(m_taskModel, &TaskModel::taskCountChanged, this, &kmemo::updateStatusBar);

    // Empty state button connection
    connect(m_emptyStateButton, &QPushButton::clicked, this, &kmemo::onAddTaskClicked);

    // 新增连接
    connect(m_quickAddTaskBtn, &QPushButton::clicked, this, &kmemo::onAddTaskClicked);
    connect(m_themeToggleBtn, &QPushButton::clicked, this, &kmemo::onThemeToggleClicked);
    connect(m_collapseDetailBtn, &QPushButton::clicked, this, &kmemo::onToggleDetailPanel);
    connect(m_searchLineEdit, &QLineEdit::textChanged, this, &kmemo::onSearchTextChanged);

    // 连接新工具栏按钮到原有Actions
    connect(m_deleteTaskBtn, &QPushButton::clicked, this, &kmemo::onDeleteTaskClicked);
    connect(m_completeTaskBtn, &QPushButton::clicked, this, &kmemo::onCompleteTaskClicked);

    // 连接自定义组件信号
    setupCustomComponentConnections();
}

void kmemo::setupContextMenu()
{
    IconManager* iconMgr = IconManager::instance();
    m_contextMenu = new QMenu(this);

    m_editAction = m_contextMenu->addAction(iconMgr->getActionIcon("edit"), "Edit");
    connect(m_editAction, &QAction::triggered, [this]() {
        QModelIndexList selected = m_taskListView->selectionModel()->selectedIndexes();
        if (!selected.isEmpty()) {
            onTaskDoubleClicked(selected.first());
        }
    });

    m_deleteAction = m_contextMenu->addAction(iconMgr->getActionIcon("delete"), "Delete");
    connect(m_deleteAction, &QAction::triggered, this, &kmemo::onDeleteTaskClicked);

    m_completeAction = m_contextMenu->addAction(iconMgr->getActionIcon("complete"), "Complete");
    connect(m_completeAction, &QAction::triggered, this, &kmemo::onCompleteTaskClicked);

    m_contextMenu->addSeparator();

    QAction *newTaskAction = m_contextMenu->addAction(iconMgr->getActionIcon("add"), "New Task");
    connect(newTaskAction, &QAction::triggered, this, &kmemo::onAddTaskClicked);
}

void kmemo::populateFilters()
{
    IconManager* iconMgr = IconManager::instance();

    // Category filter
    m_categoryComboBox->clear();
    m_categoryComboBox->addItem(iconMgr->getCategoryIcon("all"), "All Categories", "");
    // TODO: Load actual categories from database
    m_categoryComboBox->addItem(iconMgr->getCategoryIcon("work"), "Work", "work");
    m_categoryComboBox->addItem(iconMgr->getCategoryIcon("personal"), "Personal", "personal");
    m_categoryComboBox->addItem(iconMgr->getCategoryIcon("study"), "Study", "study");

    // Status filter
    m_statusComboBox->clear();
    m_statusComboBox->addItem("All Status", -1);
    m_statusComboBox->addItem(iconMgr->getStatusIcon(TaskStatus::Pending), "Pending",
                             static_cast<int>(TaskStatus::Pending));
    m_statusComboBox->addItem(iconMgr->getStatusIcon(TaskStatus::InProgress), "In Progress",
                             static_cast<int>(TaskStatus::InProgress));
    m_statusComboBox->addItem(iconMgr->getStatusIcon(TaskStatus::Completed), "Completed",
                             static_cast<int>(TaskStatus::Completed));
    m_statusComboBox->addItem(iconMgr->getStatusIcon(TaskStatus::Cancelled), "Cancelled",
                             static_cast<int>(TaskStatus::Cancelled));
}

void kmemo::onAddTaskClicked()
{
    bool ok;
    QString title = QInputDialog::getText(this, "新建任务", "任务标题:", QLineEdit::Normal, "", &ok);

    if (ok && !title.isEmpty()) {
        Task newTask(title, "");
        if (m_taskModel->addTask(newTask)) {
            updateStatusBar();
        } else {
            QMessageBox::warning(this, "错误", "添加任务失败");
        }
    }
}

void kmemo::onDeleteTaskClicked()
{
    QModelIndexList selected = m_taskListView->selectionModel()->selectedIndexes();
    if (selected.isEmpty()) {
        QMessageBox::information(this, "提示", "请先选择要删除的任务");
        return;
    }

    int result = QMessageBox::question(this, "确认删除",
                                       QString("确定要删除 %1 个任务吗?").arg(selected.size()),
                                       QMessageBox::Yes | QMessageBox::No);

    if (result == QMessageBox::Yes) {
        // Delete from back to front to avoid index change issues
        QList<int> rows;
        for (const QModelIndex &index : selected) {
            rows.append(index.row());
        }
        std::sort(rows.begin(), rows.end(), std::greater<int>());

        for (int row : rows) {
            m_taskModel->removeTask(row);
        }
        updateStatusBar();
    }
}

void kmemo::onCompleteTaskClicked()
{
    QModelIndexList selected = m_taskListView->selectionModel()->selectedIndexes();
    if (selected.isEmpty()) {
        QMessageBox::information(this, "提示", "请先选择要完成的任务");
        return;
    }

    for (const QModelIndex &index : selected) {
        Task task = m_taskModel->getTask(index.row());
        if (task.status() != TaskStatus::Completed) {
            task.setStatus(TaskStatus::Completed);
            m_taskModel->updateTask(task);
        }
    }
    updateStatusBar();
}

void kmemo::onCategoryFilterChanged()
{
    QString category = m_categoryComboBox->currentData().toString();
    // TODO: Implement category filtering
    refreshTaskList();
}

void kmemo::onStatusFilterChanged()
{
    int statusInt = m_statusComboBox->currentData().toInt();
    if (statusInt == -1) {
        m_taskModel->clearFilter();
    } else {
        TaskStatus status = static_cast<TaskStatus>(statusInt);
        m_taskModel->setFilter("", status);
    }
    updateStatusBar();
}

void kmemo::onTaskSelectionChanged()
{
    QModelIndexList selected = m_taskListView->selectionModel()->selectedIndexes();
    bool hasSelection = !selected.isEmpty();

    m_actionDeleteTask->setEnabled(hasSelection);
    m_actionCompleteTask->setEnabled(hasSelection);

    // 更新新工具栏按钮状态
    m_deleteTaskBtn->setEnabled(hasSelection);
    m_completeTaskBtn->setEnabled(hasSelection);
}

void kmemo::onTaskDoubleClicked(const QModelIndex &index)
{
    if (!index.isValid()) return;

    Task task = m_taskModel->getTask(index.row());
    bool ok;
    QString newTitle = QInputDialog::getText(this, "编辑任务", "任务标题:",
                                             QLineEdit::Normal, task.title(), &ok);

    if (ok && !newTitle.isEmpty() && newTitle != task.title()) {
        task.setTitle(newTitle);
        m_taskModel->updateTask(task);
    }
}

void kmemo::onTaskContextMenuRequested(const QPoint &pos)
{
    QModelIndex index = m_taskListView->indexAt(pos);
    if (index.isValid()) {
        m_contextMenu->exec(m_taskListView->mapToGlobal(pos));
    }
}

void kmemo::updateStatusBar()
{
    int total = m_taskModel->getTaskCount();
    int completed = m_taskModel->getCompletedCount();
    int pending = m_taskModel->getPendingCount();

    // 更新左侧边栏的统计信息
    m_taskCountLabel->setText(QString("总计: %1").arg(total));
    m_completedCountLabel->setText(QString("已完成: %1").arg(completed));
    m_pendingCountLabel->setText(QString("待办: %1").arg(pending));

    // Switch between empty state and task list based on task count
    if (total == 0) {
        m_contentStack->setCurrentIndex(1); // Show empty state page
    } else {
        m_contentStack->setCurrentIndex(0); // Show task list page
    }
}

void kmemo::refreshTaskList()
{
    m_taskModel->refresh();
    updateStatusBar();
}

void kmemo::enterInlineEditMode(const QModelIndex &index)
{
    // TODO: Implement inline edit mode
    m_inlineEditMode = true;
    m_editingIndex = index;
}

void kmemo::exitInlineEditMode()
{
    m_inlineEditMode = false;
    m_editingIndex = QModelIndex();
}

void kmemo::onThemeToggleClicked()
{
    // TODO: 实现主题切换逻辑（阶段7完成）
    // 暂时切换按钮文本
    static bool isDark = false;
    isDark = !isDark;
    m_themeToggleBtn->setText(isDark ? "Sun" : "Moon");
}

void kmemo::onToggleDetailPanel()
{
    m_rightPanelCollapsed = !m_rightPanelCollapsed;
    m_rightDetailPanel->setVisible(!m_rightPanelCollapsed);
    m_collapseDetailBtn->setText(m_rightPanelCollapsed ? "<" : ">");
}

void kmemo::onSearchTextChanged(const QString &text)
{
    // TODO: 实现搜索功能
    Q_UNUSED(text)
    // 暂时留空，后续阶段实现
}

void kmemo::integrateCustomComponents()
{
    // 将TaskStatsWidget集成到左侧边栏
    if (m_leftSidebarPanel) {
        // 获取左侧边栏的现有布局或创建新布局
        QLayout* existingLayout = m_leftSidebarPanel->layout();
        QVBoxLayout* sidebarLayout = nullptr;

        if (existingLayout) {
            sidebarLayout = qobject_cast<QVBoxLayout*>(existingLayout);
        }

        if (!sidebarLayout) {
            sidebarLayout = new QVBoxLayout(m_leftSidebarPanel);
            sidebarLayout->setContentsMargins(0, 0, 0, 0);
            sidebarLayout->setSpacing(0);
        }

        // 在布局底部添加统计组件
        sidebarLayout->addWidget(m_taskStatsWidget);
    }

    // 将TaskDetailWidget集成到右侧详情面板
    if (m_rightDetailPanel) {
        // 获取右侧面板的现有布局或创建新布局
        QLayout* existingLayout = m_rightDetailPanel->layout();
        QVBoxLayout* detailLayout = nullptr;

        if (existingLayout) {
            detailLayout = qobject_cast<QVBoxLayout*>(existingLayout);
        }

        if (!detailLayout) {
            detailLayout = new QVBoxLayout(m_rightDetailPanel);
            detailLayout->setContentsMargins(0, 0, 0, 0);
            detailLayout->setSpacing(0);
        }

        // 添加任务详情组件
        detailLayout->addWidget(m_taskDetailWidget);
    }

    // 将QuickAddWidget添加到主窗口中央区域的一个可切换位置
    // 这里我们需要修改UI设计来容纳它，暂时先创建一个简单的集成
    if (m_contentStack && m_contentStack->count() > 0) {
        // 为快速添加功能创建一个新的页面
        auto quickAddPage = new QWidget();
        auto quickAddLayout = new QVBoxLayout(quickAddPage);
        quickAddLayout->addWidget(m_quickAddWidget);

        // 暂时不添加到stack中，等待进一步的UI设计决策
        // m_contentStack->addWidget(quickAddPage);

        // 作为替代，将QuickAddWidget隐藏，通过按钮切换显示
        m_quickAddWidget->setParent(this);
        m_quickAddWidget->hide();
    }
}


void kmemo::setupCustomComponentConnections()
{
    // TaskDetailWidget连接
    if (m_taskDetailWidget) {
        connect(m_taskDetailWidget, &TaskDetailWidget::taskUpdated,
                [this](const Task& task) {
                    m_taskModel->updateTask(task);
                    updateStatusBar();
                });

        connect(m_taskDetailWidget, &TaskDetailWidget::editTaskRequested,
                this, &kmemo::onTaskDetailRequested);

        connect(m_taskDetailWidget, &TaskDetailWidget::deleteTaskRequested,
                this, &kmemo::onTaskDeleteRequested);
    }

    // TaskStatsWidget连接
    if (m_taskStatsWidget) {
        connect(m_taskStatsWidget, &TaskStatsWidget::filterRequested,
                this, &kmemo::onTaskFilterRequested);

        connect(m_taskStatsWidget, &TaskStatsWidget::showAllTasksRequested,
                this, &kmemo::onShowAllTasksRequested);
    }

    // QuickAddWidget连接
    if (m_quickAddWidget) {
        connect(m_quickAddWidget, &QuickAddWidget::taskCreated,
                this, &kmemo::onTaskCreated);

        connect(m_quickAddWidget, &QuickAddWidget::detailedEditRequested,
                this, &kmemo::onTaskDetailRequested);
    }

    // 任务选择变化时更新详情面板
    connect(m_taskListView->selectionModel(), &QItemSelectionModel::selectionChanged,
            [this]() {
                QModelIndexList selected = m_taskListView->selectionModel()->selectedIndexes();
                if (!selected.isEmpty()) {
                    Task task = m_taskModel->getTask(selected.first().row());
                    m_taskDetailWidget->setTask(task);
                } else {
                    m_taskDetailWidget->clearTask();
                }
            });
}

// 自定义组件槽函数实现
void kmemo::onTaskCreated(const Task& task)
{
    if (m_taskModel->addTask(task)) {
        updateStatusBar();
        // 选择新添加的任务
        int taskCount = m_taskModel->getTaskCount();
        if (taskCount > 0) {
            QModelIndex newIndex = m_taskModel->index(taskCount - 1, 0);
            m_taskListView->setCurrentIndex(newIndex);
        }
    } else {
        QMessageBox::warning(this, "错误", "添加任务失败");
    }
}

void kmemo::onTaskDetailRequested(const Task& task)
{
    // TODO: 在阶段5中实现详细编辑对话框
    // 暂时使用简单的输入对话框
    bool ok;
    QString newTitle = QInputDialog::getText(this, "编辑任务", "任务标题:",
                                             QLineEdit::Normal, task.title(), &ok);

    if (ok && !newTitle.isEmpty() && newTitle != task.title()) {
        Task updatedTask = task;
        updatedTask.setTitle(newTitle);
        m_taskModel->updateTask(updatedTask);
    }
}

void kmemo::onTaskDeleteRequested(const Task& task)
{
    int result = QMessageBox::question(this, "确认删除",
                                       QString("确定要删除任务 '%1' 吗?").arg(task.title()),
                                       QMessageBox::Yes | QMessageBox::No);

    if (result == QMessageBox::Yes) {
        // 找到任务在模型中的索引
        for (int i = 0; i < m_taskModel->getTaskCount(); ++i) {
            Task modelTask = m_taskModel->getTask(i);
            if (modelTask.id() == task.id()) {
                m_taskModel->removeTask(i);
                updateStatusBar();
                break;
            }
        }
    }
}

void kmemo::onTaskFilterRequested(const QString& category, TaskStatus status)
{
    // 应用过滤器
    m_taskModel->setFilter(category, status);
    updateStatusBar();

    // 同步过滤器UI
    if (!category.isEmpty()) {
        int categoryIndex = m_categoryComboBox->findData(category);
        if (categoryIndex >= 0) {
            m_categoryComboBox->setCurrentIndex(categoryIndex);
        }
    }

    int statusIndex = m_statusComboBox->findData(static_cast<int>(status));
    if (statusIndex >= 0) {
        m_statusComboBox->setCurrentIndex(statusIndex);
    }
}

void kmemo::onShowAllTasksRequested()
{
    // 清除所有过滤器
    m_taskModel->clearFilter();
    m_categoryComboBox->setCurrentIndex(0); // "All Categories"
    m_statusComboBox->setCurrentIndex(0);   // "All Status"
    updateStatusBar();
}
