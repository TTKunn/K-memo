#include "simpletasklistwidget.h"
#include "ui_simpletasklistwidget.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QLabel>
#include <QFrame>
#include <QString>

SimpleTaskListWidget::SimpleTaskListWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SimpleTaskListWidget)
    , m_statusFilter(-1)
    , m_priorityFilter(-1)
{
    ui->setupUi(this);

    // 连接信号
    connect(ui->taskListWidget, &QListWidget::itemDoubleClicked,
            this, &SimpleTaskListWidget::onTaskItemDoubleClicked);

    // 初始显示空状态
    updateEmptyState();
    updateCountDisplay();
}

SimpleTaskListWidget::~SimpleTaskListWidget()
{
    delete ui;
}

void SimpleTaskListWidget::setTasks(const QList<Task> &tasks)
{
    clearTasks();
    m_tasks = tasks;

    for (const Task &task : tasks) {
        addTask(task);
    }

    applyFilters();
    updateCountDisplay();
    updateEmptyState();
}

void SimpleTaskListWidget::addTask(const Task &task)
{
    // 避免重复添加
    if (m_itemMap.contains(task.id())) {
        updateTask(task);
        return;
    }

    // 添加到任务列表
    m_tasks.append(task);

    // 创建列表项
    QListWidgetItem *item = new QListWidgetItem(ui->taskListWidget);
    item->setData(Qt::UserRole, task.id());

    // 创建任务Widget
    QWidget *taskWidget = createTaskItemWidget(task);
    item->setSizeHint(taskWidget->sizeHint());

    // 设置到列表中
    ui->taskListWidget->addItem(item);
    ui->taskListWidget->setItemWidget(item, taskWidget);

    // 保存映射
    m_itemMap[task.id()] = item;

    // 更新显示
    applyFilters();
    updateCountDisplay();
    updateEmptyState();

    emit taskCountChanged(getTaskCount(), getCompletedCount());
}

void SimpleTaskListWidget::removeTask(const QString &taskId)
{
    // 从映射中移除
    if (m_itemMap.contains(taskId)) {
        QListWidgetItem *item = m_itemMap[taskId];
        delete ui->taskListWidget->takeItem(ui->taskListWidget->row(item));
        m_itemMap.remove(taskId);
    }

    // 从任务列表中移除
    for (auto it = m_tasks.begin(); it != m_tasks.end(); ++it) {
        if (it->id() == taskId) {
            m_tasks.erase(it);
            break;
        }
    }

    // 更新显示
    updateCountDisplay();
    updateEmptyState();

    emit taskCountChanged(getTaskCount(), getCompletedCount());
}

void SimpleTaskListWidget::updateTask(const Task &task)
{
    // 更新任务列表中的数据
    for (auto &existingTask : m_tasks) {
        if (existingTask.id() == task.id()) {
            existingTask = task;
            break;
        }
    }

    // 更新UI显示
    if (m_itemMap.contains(task.id())) {
        QListWidgetItem *item = m_itemMap[task.id()];
        QWidget *taskWidget = createTaskItemWidget(task);
        item->setSizeHint(taskWidget->sizeHint());
        ui->taskListWidget->setItemWidget(item, taskWidget);
    }

    // 更新显示
    applyFilters();
    updateCountDisplay();

    emit taskCountChanged(getTaskCount(), getCompletedCount());
}

void SimpleTaskListWidget::clearTasks()
{
    ui->taskListWidget->clear();
    m_itemMap.clear();
    m_tasks.clear();

    updateCountDisplay();
    updateEmptyState();

    emit taskCountChanged(0, 0);
}

int SimpleTaskListWidget::getTaskCount() const
{
    return m_tasks.size();
}

int SimpleTaskListWidget::getCompletedCount() const
{
    int count = 0;
    for (const Task &task : m_tasks) {
        if (task.status() == TaskStatus::Completed) {
            count++;
        }
    }
    return count;
}

void SimpleTaskListWidget::refreshList()
{
    QList<Task> tasks = m_tasks;
    setTasks(tasks);
}

void SimpleTaskListWidget::filterByStatus(int status)
{
    m_statusFilter = status;
    applyFilters();
}

void SimpleTaskListWidget::filterByPriority(int priority)
{
    m_priorityFilter = priority;
    applyFilters();
}

void SimpleTaskListWidget::clearFilters()
{
    m_statusFilter = -1;
    m_priorityFilter = -1;
    applyFilters();
}

void SimpleTaskListWidget::onTaskCheckStateChanged()
{
    QCheckBox *checkBox = qobject_cast<QCheckBox*>(sender());
    if (!checkBox) return;

    QString taskId = checkBox->property("taskId").toString();
    bool completed = checkBox->isChecked();

    // 更新任务状态
    for (auto &task : m_tasks) {
        if (task.id() == taskId) {
            task.setStatus(completed ? TaskStatus::Completed : TaskStatus::Pending);
            break;
        }
    }

    // 更新显示
    updateCountDisplay();

    emit taskStatusChanged(taskId, completed);
    emit taskCountChanged(getTaskCount(), getCompletedCount());
}

void SimpleTaskListWidget::onTaskItemDoubleClicked(QListWidgetItem *item)
{
    if (!item) return;

    QString taskId = item->data(Qt::UserRole).toString();
    emit taskDoubleClicked(taskId);
}

QWidget* SimpleTaskListWidget::createTaskItemWidget(const Task &task)
{
    QWidget *widget = new QWidget();
    QHBoxLayout *layout = new QHBoxLayout(widget);
    layout->setContentsMargins(8, 4, 8, 4);
    layout->setSpacing(12);

    // 优先级指示器
    QFrame *priorityIndicator = new QFrame();
    priorityIndicator->setFixedSize(4, 32);
    priorityIndicator->setStyleSheet(QString("background: %1; border-radius: 2px;")
                                   .arg(getPriorityColor(task.priority())));
    layout->addWidget(priorityIndicator);

    // 任务复选框
    QCheckBox *checkBox = new QCheckBox();
    checkBox->setText(task.title());
    checkBox->setChecked(task.status() == TaskStatus::Completed);
    checkBox->setProperty("taskId", task.id());

    // 设置已完成任务的样式
    if (task.status() == TaskStatus::Completed) {
        checkBox->setStyleSheet("QCheckBox { color: #9aa0a6; text-decoration: line-through; }");
    } else {
        checkBox->setStyleSheet("QCheckBox { color: #3c4043; }");
    }

    connect(checkBox, &QCheckBox::stateChanged,
            this, &SimpleTaskListWidget::onTaskCheckStateChanged);

    layout->addWidget(checkBox, 1);

    // 截止时间标签（如果有）
    if (task.dueTime().isValid()) {
        QLabel *dueLabel = new QLabel();
        QString dueText = task.dueTime().toString("MM/dd");
        dueLabel->setText(dueText);
        dueLabel->setStyleSheet("font-size: 11px; color: #ea4335; padding: 2px 6px; "
                               "background: #fce8e6; border-radius: 4px;");
        layout->addWidget(dueLabel);
    }

    widget->setLayout(layout);
    return widget;
}

QString SimpleTaskListWidget::getPriorityColor(TaskPriority priority) const
{
    switch (priority) {
        case TaskPriority::Urgent:
            return "#ea4335";  // 紧急 - 红色
        case TaskPriority::High:
            return "#ff9800";  // 高优先级 - 橙色
        case TaskPriority::Normal:
            return "#4285f4";  // 普通 - 蓝色
        case TaskPriority::Low:
            return "#34a853";  // 低优先级 - 绿色
        default:
            return "#dadce0";  // 默认 - 灰色
    }
}

void SimpleTaskListWidget::updateCountDisplay()
{
    int total = getTaskCount();
    int completed = getCompletedCount();

    QString countText;
    if (total == 0) {
        countText = "0 items";
    } else {
        countText = QString("%1 items (%2 completed)").arg(total).arg(completed);
    }

    ui->countLabel->setText(countText);
}

void SimpleTaskListWidget::updateEmptyState()
{
    bool isEmpty = (getTaskCount() == 0);

    if (isEmpty) {
        ui->contentStack->setCurrentWidget(ui->emptyStatePage);
    } else {
        ui->contentStack->setCurrentWidget(ui->taskListPage);
    }
}

void SimpleTaskListWidget::applyFilters()
{
    for (int i = 0; i < ui->taskListWidget->count(); ++i) {
        QListWidgetItem *item = ui->taskListWidget->item(i);
        QString taskId = item->data(Qt::UserRole).toString();

        // 查找对应的任务
        Task task;
        bool found = false;
        for (const Task &t : m_tasks) {
            if (t.id() == taskId) {
                task = t;
                found = true;
                break;
            }
        }

        if (found) {
            bool visible = isTaskVisible(task);
            item->setHidden(!visible);
        }
    }
}

bool SimpleTaskListWidget::isTaskVisible(const Task &task) const
{
    // 状态过滤
    if (m_statusFilter != -1) {
        if (static_cast<int>(task.status()) != m_statusFilter) {
            return false;
        }
    }

    // 优先级过滤
    if (m_priorityFilter != -1) {
        if (static_cast<int>(task.priority()) != m_priorityFilter) {
            return false;
        }
    }

    return true;
}