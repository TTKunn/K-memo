#include "TaskDetailWidget.h"
#include "IconManager.h"
#include <QMessageBox>

TaskDetailWidget::TaskDetailWidget(QWidget *parent)
    : QWidget(parent)
    , m_hasValidTask(false)
{
    setupUI();
    setupConnections();
    setNoSelectionState();
}

void TaskDetailWidget::setupUI()
{
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // 创建滚动区域
    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setFrameShape(QFrame::NoFrame);

    m_contentWidget = new QWidget();
    m_contentLayout = new QVBoxLayout(m_contentWidget);
    m_contentLayout->setContentsMargins(16, 16, 16, 16);
    m_contentLayout->setSpacing(12);

    // 标题显示
    m_titleLabel = new QLabel();
    m_titleLabel->setStyleSheet(
        "font-size: 16px; font-weight: 600; color: #212529; "
        "padding: 8px 0px; word-wrap: true;"
    );
    m_titleLabel->setWordWrap(true);

    // 状态和优先级信息
    auto infoFrame = new QFrame();
    infoFrame->setStyleSheet(
        "QFrame { background: #f8f9fa; border: 1px solid #e9ecef; "
        "border-radius: 8px; padding: 12px; }"
    );
    auto infoLayout = new QVBoxLayout(infoFrame);

    m_statusLabel = new QLabel();
    m_priorityLabel = new QLabel();
    m_categoryLabel = new QLabel();
    m_createTimeLabel = new QLabel();
    m_dueTimeLabel = new QLabel();

    QString labelStyle = "font-size: 13px; color: #495057; margin: 2px 0px;";
    m_statusLabel->setStyleSheet(labelStyle);
    m_priorityLabel->setStyleSheet(labelStyle);
    m_categoryLabel->setStyleSheet(labelStyle);
    m_createTimeLabel->setStyleSheet(labelStyle);
    m_dueTimeLabel->setStyleSheet(labelStyle);

    infoLayout->addWidget(m_statusLabel);
    infoLayout->addWidget(m_priorityLabel);
    infoLayout->addWidget(m_categoryLabel);
    infoLayout->addWidget(m_createTimeLabel);
    infoLayout->addWidget(m_dueTimeLabel);

    // 描述编辑区域
    auto descFrame = new QFrame();
    descFrame->setStyleSheet(
        "QFrame { background: white; border: 1px solid #e9ecef; "
        "border-radius: 8px; padding: 8px; }"
    );
    auto descLayout = new QVBoxLayout(descFrame);

    auto descLabel = new QLabel("描述:");
    descLabel->setStyleSheet("font-size: 14px; font-weight: 600; color: #495057;");

    m_descriptionEdit = new QTextEdit();
    m_descriptionEdit->setStyleSheet(
        "QTextEdit { border: none; background: transparent; "
        "font-size: 13px; color: #212529; }"
    );
    m_descriptionEdit->setMaximumHeight(100);

    descLayout->addWidget(descLabel);
    descLayout->addWidget(m_descriptionEdit);

    // 快速编辑区域
    auto editFrame = new QFrame();
    editFrame->setStyleSheet(
        "QFrame { background: white; border: 1px solid #e9ecef; "
        "border-radius: 8px; padding: 12px; }"
    );
    auto editLayout = new QVBoxLayout(editFrame);

    auto editLabel = new QLabel("快速编辑:");
    editLabel->setStyleSheet("font-size: 14px; font-weight: 600; color: #495057;");

    m_priorityCombo = new QComboBox();
    m_statusCombo = new QComboBox();
    m_dueTimeEdit = new QDateTimeEdit();

    QString comboStyle =
        "QComboBox { background: white; border: 1px solid #ced4da; "
        "border-radius: 4px; padding: 6px 12px; font-size: 13px; } "
        "QComboBox:focus { border-color: #4285f4; }";

    m_priorityCombo->setStyleSheet(comboStyle);
    m_statusCombo->setStyleSheet(comboStyle);
    m_dueTimeEdit->setStyleSheet(comboStyle);

    editLayout->addWidget(editLabel);
    editLayout->addWidget(new QLabel("优先级:"));
    editLayout->addWidget(m_priorityCombo);
    editLayout->addWidget(new QLabel("状态:"));
    editLayout->addWidget(m_statusCombo);
    editLayout->addWidget(new QLabel("截止日期:"));
    editLayout->addWidget(m_dueTimeEdit);

    // 操作按钮
    auto buttonFrame = new QFrame();
    auto buttonLayout = new QVBoxLayout(buttonFrame);

    m_editButton = new QPushButton("编辑任务");
    m_deleteButton = new QPushButton("删除任务");
    m_completeButton = new QPushButton("标记完成");

    m_editButton->setStyleSheet(
        "QPushButton { background: #17a2b8; color: white; border: none; "
        "border-radius: 6px; padding: 8px 16px; font-weight: 500; } "
        "QPushButton:hover { background: #138496; }"
    );
    m_deleteButton->setStyleSheet(
        "QPushButton { background: #dc3545; color: white; border: none; "
        "border-radius: 6px; padding: 8px 16px; font-weight: 500; } "
        "QPushButton:hover { background: #c82333; }"
    );
    m_completeButton->setStyleSheet(
        "QPushButton { background: #28a745; color: white; border: none; "
        "border-radius: 6px; padding: 8px 16px; font-weight: 500; } "
        "QPushButton:hover { background: #218838; }"
    );

    buttonLayout->addWidget(m_editButton);
    buttonLayout->addWidget(m_deleteButton);
    buttonLayout->addWidget(m_completeButton);

    // 无选择状态标签
    m_noSelectionLabel = new QLabel("Select a task to view details");
    m_noSelectionLabel->setAlignment(Qt::AlignCenter);
    m_noSelectionLabel->setStyleSheet(
        "font-size: 14px; color: #6c757d; padding: 40px 20px;"
    );

    // 添加组件到布局
    m_contentLayout->addWidget(m_titleLabel);
    m_contentLayout->addWidget(infoFrame);
    m_contentLayout->addWidget(descFrame);
    m_contentLayout->addWidget(editFrame);
    m_contentLayout->addWidget(buttonFrame);
    m_contentLayout->addWidget(m_noSelectionLabel);
    m_contentLayout->addStretch();

    m_scrollArea->setWidget(m_contentWidget);
    mainLayout->addWidget(m_scrollArea);

    // 填充下拉框
    IconManager* iconMgr = IconManager::instance();

    m_priorityCombo->addItem(iconMgr->getPriorityIcon(TaskPriority::Low), "低", static_cast<int>(TaskPriority::Low));
    m_priorityCombo->addItem(iconMgr->getPriorityIcon(TaskPriority::Normal), "普通", static_cast<int>(TaskPriority::Normal));
    m_priorityCombo->addItem(iconMgr->getPriorityIcon(TaskPriority::High), "高", static_cast<int>(TaskPriority::High));
    m_priorityCombo->addItem(iconMgr->getPriorityIcon(TaskPriority::Urgent), "紧急", static_cast<int>(TaskPriority::Urgent));

    m_statusCombo->addItem(iconMgr->getStatusIcon(TaskStatus::Pending), "待办", static_cast<int>(TaskStatus::Pending));
    m_statusCombo->addItem(iconMgr->getStatusIcon(TaskStatus::InProgress), "进行中", static_cast<int>(TaskStatus::InProgress));
    m_statusCombo->addItem(iconMgr->getStatusIcon(TaskStatus::Completed), "已完成", static_cast<int>(TaskStatus::Completed));
    m_statusCombo->addItem(iconMgr->getStatusIcon(TaskStatus::Cancelled), "已取消", static_cast<int>(TaskStatus::Cancelled));
}

void TaskDetailWidget::setupConnections()
{
    connect(m_editButton, &QPushButton::clicked, this, &TaskDetailWidget::onEditButtonClicked);
    connect(m_deleteButton, &QPushButton::clicked, this, &TaskDetailWidget::onDeleteButtonClicked);
    connect(m_completeButton, &QPushButton::clicked, this, [this]() {
        if (m_hasValidTask) {
            m_currentTask.setStatus(TaskStatus::Completed);
            emit taskUpdated(m_currentTask);
        }
    });

    connect(m_priorityCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &TaskDetailWidget::onPriorityChanged);
    connect(m_statusCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &TaskDetailWidget::onStatusChanged);
    connect(m_descriptionEdit, &QTextEdit::textChanged, this, &TaskDetailWidget::onDescriptionChanged);
}

void TaskDetailWidget::setTask(const Task& task)
{
    m_currentTask = task;
    m_hasValidTask = true;
    updateTaskDisplay();
}

void TaskDetailWidget::clearTask()
{
    m_hasValidTask = false;
    setNoSelectionState();
}

void TaskDetailWidget::updateTaskDisplay()
{
    if (!m_hasValidTask) {
        setNoSelectionState();
        return;
    }

    // 显示详情组件，隐藏无选择状态
    m_noSelectionLabel->hide();
    m_titleLabel->show();

    // 更新任务信息
    m_titleLabel->setText(m_currentTask.title());

    m_statusLabel->setText(QString("状态: %1").arg(m_currentTask.statusString()));
    m_priorityLabel->setText(QString("优先级: %1").arg(m_currentTask.priorityString()));
    m_categoryLabel->setText(QString("分类: %1").arg(m_currentTask.category().isEmpty() ? "无" : m_currentTask.category()));
    m_createTimeLabel->setText(QString("创建时间: %1").arg(m_currentTask.createTime().toString("yyyy-MM-dd hh:mm")));

    if (m_currentTask.dueTime().isValid()) {
        m_dueTimeLabel->setText(QString("截止时间: %1").arg(m_currentTask.dueTime().toString("yyyy-MM-dd hh:mm")));
    } else {
        m_dueTimeLabel->setText("截止时间: 未设置");
    }

    m_descriptionEdit->setPlainText(m_currentTask.description());

    // 更新快速编辑控件
    m_priorityCombo->setCurrentIndex(static_cast<int>(m_currentTask.priority()) - 1);
    m_statusCombo->setCurrentIndex(static_cast<int>(m_currentTask.status()));

    if (m_currentTask.dueTime().isValid()) {
        m_dueTimeEdit->setDateTime(m_currentTask.dueTime());
    } else {
        m_dueTimeEdit->setDateTime(QDateTime::currentDateTime().addDays(1));
    }

    // 更新按钮状态
    m_completeButton->setEnabled(m_currentTask.status() != TaskStatus::Completed);
}

void TaskDetailWidget::setNoSelectionState()
{
    m_noSelectionLabel->show();
    m_titleLabel->hide();
}

void TaskDetailWidget::onEditButtonClicked()
{
    if (m_hasValidTask) {
        emit editTaskRequested(m_currentTask);
    }
}

void TaskDetailWidget::onDeleteButtonClicked()
{
    if (m_hasValidTask) {
        int result = QMessageBox::question(this, "确认删除",
            QString("确定要删除任务 \"%1\" 吗？").arg(m_currentTask.title()),
            QMessageBox::Yes | QMessageBox::No);

        if (result == QMessageBox::Yes) {
            emit deleteTaskRequested(m_currentTask);
        }
    }
}

void TaskDetailWidget::onPriorityChanged()
{
    if (m_hasValidTask) {
        TaskPriority newPriority = static_cast<TaskPriority>(m_priorityCombo->currentData().toInt());
        m_currentTask.setPriority(newPriority);
        emit taskUpdated(m_currentTask);
        updateTaskDisplay();
    }
}

void TaskDetailWidget::onStatusChanged()
{
    if (m_hasValidTask) {
        TaskStatus newStatus = static_cast<TaskStatus>(m_statusCombo->currentData().toInt());
        m_currentTask.setStatus(newStatus);
        emit taskUpdated(m_currentTask);
        updateTaskDisplay();
    }
}

void TaskDetailWidget::onDescriptionChanged()
{
    if (m_hasValidTask) {
        m_currentTask.setDescription(m_descriptionEdit->toPlainText());
        emit taskUpdated(m_currentTask);
    }
}