#include "TaskStatsWidget.h"
#include "models/taskmodel.h"
#include "IconManager.h"
#include <QScrollArea>

TaskStatsWidget::TaskStatsWidget(QWidget *parent)
    : QWidget(parent)
    , m_taskModel(nullptr)
    , m_totalTasks(0)
    , m_completedTasks(0)
    , m_pendingTasks(0)
    , m_inProgressTasks(0)
    , m_overdueTasks(0)
{
    setupUI();
    setupConnections();

    // Setup refresh timer
    m_refreshTimer = new QTimer(this);
    m_refreshTimer->setSingleShot(false);
    m_refreshTimer->setInterval(30000); // Refresh every 30 seconds
    connect(m_refreshTimer, &QTimer::timeout, this, &TaskStatsWidget::updateProgress);
}

void TaskStatsWidget::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);

    // Create scroll area
    auto scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setStyleSheet("QScrollArea { background: transparent; }");

    auto contentWidget = new QWidget();
    auto contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setContentsMargins(16, 16, 16, 16);
    contentLayout->setSpacing(16);

    // Statistics cards section
    m_statsFrame = new QFrame();
    m_statsFrame->setStyleSheet(
        "QFrame { background: white; border: 1px solid #e9ecef; "
        "border-radius: 12px; padding: 16px; }"
    );
    auto statsLayout = new QVBoxLayout(m_statsFrame);

    auto statsTitle = new QLabel("任务概览");
    statsTitle->setStyleSheet(
        "font-size: 16px; font-weight: 600; color: #212529; "
        "margin-bottom: 12px;"
    );

    auto statsCardsLayout = new QVBoxLayout();
    statsCardsLayout->setSpacing(8);

    // Create stats cards
    m_totalTasksLabel = new QLabel("0");
    m_completedTasksLabel = new QLabel("0");
    m_pendingTasksLabel = new QLabel("0");
    m_overdueTasksLabel = new QLabel("0");

    // Total tasks card
    auto totalCard = createStatsCard("总任务", "0", "#6c757d");
    totalCard->layout()->addWidget(m_totalTasksLabel);

    // Completed tasks card
    auto completedCard = createStatsCard("已完成", "0", "#28a745");
    completedCard->layout()->addWidget(m_completedTasksLabel);

    // Pending tasks card
    auto pendingCard = createStatsCard("待办", "0", "#ffc107");
    pendingCard->layout()->addWidget(m_pendingTasksLabel);

    // Overdue tasks card
    auto overdueCard = createStatsCard("过期", "0", "#dc3545");
    overdueCard->layout()->addWidget(m_overdueTasksLabel);

    statsCardsLayout->addWidget(totalCard);
    statsCardsLayout->addWidget(completedCard);
    statsCardsLayout->addWidget(pendingCard);
    statsCardsLayout->addWidget(overdueCard);

    statsLayout->addWidget(statsTitle);
    statsLayout->addLayout(statsCardsLayout);

    // Progress section
    m_progressFrame = new QFrame();
    m_progressFrame->setStyleSheet(
        "QFrame { background: white; border: 1px solid #e9ecef; "
        "border-radius: 12px; padding: 16px; }"
    );
    auto progressLayout = new QVBoxLayout(m_progressFrame);

    auto progressTitle = new QLabel("进度");
    progressTitle->setStyleSheet(
        "font-size: 16px; font-weight: 600; color: #212529; "
        "margin-bottom: 12px;"
    );

    // Completion progress
    m_completionLabel = new QLabel("完成度: 0%");
    m_completionLabel->setStyleSheet("font-size: 13px; color: #495057; margin-bottom: 4px;");

    m_completionProgress = new QProgressBar();
    m_completionProgress->setStyleSheet(
        "QProgressBar { background: #f8f9fa; border: 1px solid #e9ecef; "
        "border-radius: 8px; height: 16px; text-align: center; } "
        "QProgressBar::chunk { background: #28a745; border-radius: 7px; }"
    );
    m_completionProgress->setRange(0, 100);
    m_completionProgress->setValue(0);

    // Overdue indicator
    m_overdueLabel = new QLabel("过期率: 0%");
    m_overdueLabel->setStyleSheet("font-size: 13px; color: #495057; margin: 8px 0px 4px 0px;");

    m_overdueProgress = new QProgressBar();
    m_overdueProgress->setStyleSheet(
        "QProgressBar { background: #f8f9fa; border: 1px solid #e9ecef; "
        "border-radius: 8px; height: 16px; text-align: center; } "
        "QProgressBar::chunk { background: #dc3545; border-radius: 7px; }"
    );
    m_overdueProgress->setRange(0, 100);
    m_overdueProgress->setValue(0);

    progressLayout->addWidget(progressTitle);
    progressLayout->addWidget(m_completionLabel);
    progressLayout->addWidget(m_completionProgress);
    progressLayout->addWidget(m_overdueLabel);
    progressLayout->addWidget(m_overdueProgress);

    // Quick filters section
    m_filterFrame = new QFrame();
    m_filterFrame->setStyleSheet(
        "QFrame { background: white; border: 1px solid #e9ecef; "
        "border-radius: 12px; padding: 16px; }"
    );
    auto filterLayout = new QVBoxLayout(m_filterFrame);

    auto filterTitle = new QLabel("快速筛选");
    filterTitle->setStyleSheet(
        "font-size: 16px; font-weight: 600; color: #212529; "
        "margin-bottom: 12px;"
    );

    auto buttonsLayout = new QVBoxLayout();
    buttonsLayout->setSpacing(8);

    m_showAllBtn = createFilterButton("全部任务", "#6c757d");
    m_showPendingBtn = createFilterButton("待办", "#ffc107");
    m_showInProgressBtn = createFilterButton("进行中", "#17a2b8");
    m_showCompletedBtn = createFilterButton("已完成", "#28a745");
    m_showOverdueBtn = createFilterButton("过期", "#dc3545");

    buttonsLayout->addWidget(m_showAllBtn);
    buttonsLayout->addWidget(m_showPendingBtn);
    buttonsLayout->addWidget(m_showInProgressBtn);
    buttonsLayout->addWidget(m_showCompletedBtn);
    buttonsLayout->addWidget(m_showOverdueBtn);

    filterLayout->addWidget(filterTitle);
    filterLayout->addLayout(buttonsLayout);

    // Categories section
    m_categoriesFrame = new QFrame();
    m_categoriesFrame->setStyleSheet(
        "QFrame { background: white; border: 1px solid #e9ecef; "
        "border-radius: 12px; padding: 16px; }"
    );
    m_categoriesLayout = new QVBoxLayout(m_categoriesFrame);

    auto categoriesTitle = new QLabel("分类");
    categoriesTitle->setStyleSheet(
        "font-size: 16px; font-weight: 600; color: #212529; "
        "margin-bottom: 12px;"
    );
    m_categoriesLayout->addWidget(categoriesTitle);

    // Add all sections to content
    contentLayout->addWidget(m_statsFrame);
    contentLayout->addWidget(m_progressFrame);
    contentLayout->addWidget(m_filterFrame);
    contentLayout->addWidget(m_categoriesFrame);
    contentLayout->addStretch();

    scrollArea->setWidget(contentWidget);
    m_mainLayout->addWidget(scrollArea);
}

void TaskStatsWidget::setupConnections()
{
    connect(m_showAllBtn, &QPushButton::clicked, this, &TaskStatsWidget::onShowAllClicked);
    connect(m_showPendingBtn, &QPushButton::clicked, this, &TaskStatsWidget::onShowPendingClicked);
    connect(m_showInProgressBtn, &QPushButton::clicked, this, &TaskStatsWidget::onShowInProgressClicked);
    connect(m_showCompletedBtn, &QPushButton::clicked, this, &TaskStatsWidget::onShowCompletedClicked);
    connect(m_showOverdueBtn, &QPushButton::clicked, this, &TaskStatsWidget::onShowOverdueClicked);
}

void TaskStatsWidget::setTaskModel(TaskModel* model)
{
    if (m_taskModel) {
        disconnect(m_taskModel, nullptr, this, nullptr);
    }

    m_taskModel = model;

    if (m_taskModel) {
        connect(m_taskModel, &TaskModel::taskCountChanged, this, &TaskStatsWidget::onTaskModelChanged);
        refreshStats();
        m_refreshTimer->start();
    } else {
        m_refreshTimer->stop();
    }
}

void TaskStatsWidget::refreshStats()
{
    if (!m_taskModel) {
        return;
    }

    updateStatistics();
    updateProgressBars();
    updateCategoriesDisplay();
}

void TaskStatsWidget::onTaskModelChanged()
{
    refreshStats();
}

void TaskStatsWidget::onShowAllClicked()
{
    emit showAllTasksRequested();
}

void TaskStatsWidget::onShowPendingClicked()
{
    emit filterRequested("", TaskStatus::Pending);
}

void TaskStatsWidget::onShowInProgressClicked()
{
    emit filterRequested("", TaskStatus::InProgress);
}

void TaskStatsWidget::onShowCompletedClicked()
{
    emit filterRequested("", TaskStatus::Completed);
}

void TaskStatsWidget::onShowOverdueClicked()
{
    // For overdue, we'll need to implement custom filtering logic
    emit showAllTasksRequested(); // Placeholder - will filter overdue tasks
}

void TaskStatsWidget::updateProgress()
{
    updateProgressBars();
}

void TaskStatsWidget::updateStatistics()
{
    if (!m_taskModel) {
        m_totalTasks = m_completedTasks = m_pendingTasks = m_inProgressTasks = m_overdueTasks = 0;
        return;
    }

    auto tasks = m_taskModel->getAllTasks();
    m_totalTasks = tasks.size();
    m_completedTasks = 0;
    m_pendingTasks = 0;
    m_inProgressTasks = 0;
    m_overdueTasks = 0;

    for (const auto& task : tasks) {
        switch (task.status()) {
            case TaskStatus::Completed:
                m_completedTasks++;
                break;
            case TaskStatus::Pending:
                m_pendingTasks++;
                break;
            case TaskStatus::InProgress:
                m_inProgressTasks++;
                break;
            case TaskStatus::Cancelled:
                break;
        }

        if (task.isOverdue()) {
            m_overdueTasks++;
        }
    }

    // Update labels
    m_totalTasksLabel->setText(QString::number(m_totalTasks));
    m_completedTasksLabel->setText(QString::number(m_completedTasks));
    m_pendingTasksLabel->setText(QString::number(m_pendingTasks));
    m_overdueTasksLabel->setText(QString::number(m_overdueTasks));
}

void TaskStatsWidget::updateProgressBars()
{
    if (m_totalTasks == 0) {
        m_completionProgress->setValue(0);
        m_completionLabel->setText("完成度: 0%");
        m_overdueProgress->setValue(0);
        m_overdueLabel->setText("过期率: 0%");
        return;
    }

    int completionPercent = (m_completedTasks * 100) / m_totalTasks;
    int overduePercent = (m_overdueTasks * 100) / m_totalTasks;

    m_completionProgress->setValue(completionPercent);
    m_completionLabel->setText(QString("完成度: %1%").arg(completionPercent));

    m_overdueProgress->setValue(overduePercent);
    m_overdueLabel->setText(QString("过期率: %1%").arg(overduePercent));
}

QFrame* TaskStatsWidget::createStatsCard(const QString& title, const QString& value, const QString& color)
{
    auto card = new QFrame();
    card->setStyleSheet(
        QString("QFrame { background: %1; border: none; border-radius: 8px; "
               "padding: 12px; margin: 2px; }").arg(color)
    );

    auto layout = new QHBoxLayout(card);
    layout->setContentsMargins(8, 8, 8, 8);

    auto titleLabel = new QLabel(title);
    titleLabel->setStyleSheet("color: white; font-size: 13px; font-weight: 500;");

    layout->addWidget(titleLabel);
    layout->addStretch();

    return card;
}

QPushButton* TaskStatsWidget::createFilterButton(const QString& text, const QString& color)
{
    auto button = new QPushButton(text);
    button->setStyleSheet(
        QString("QPushButton { background: %1; color: white; border: none; "
               "border-radius: 6px; padding: 8px 16px; font-weight: 500; "
               "text-align: left; } "
               "QPushButton:hover { background: %2; }")
        .arg(color)
        .arg(QString(color).replace("#", "#").replace("6", "5").replace("7", "6").replace("8", "7").replace("9", "8"))
    );
    button->setCursor(Qt::PointingHandCursor);
    return button;
}

void TaskStatsWidget::updateCategoriesDisplay()
{
    if (!m_taskModel) {
        return;
    }

    // Clear existing category widgets
    while (m_categoriesLayout->count() > 1) { // Keep the title
        QLayoutItem* item = m_categoriesLayout->takeAt(1);
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }

    // Get all categories from tasks
    QMap<QString, int> categoryCounts;
    auto tasks = m_taskModel->getAllTasks();

    for (const auto& task : tasks) {
        QString category = task.category().isEmpty() ? "未分类" : task.category();
        categoryCounts[category]++;
    }

    // Create category buttons
    for (auto it = categoryCounts.begin(); it != categoryCounts.end(); ++it) {
        auto categoryBtn = new QPushButton(QString("%1 (%2)").arg(it.key()).arg(it.value()));
        categoryBtn->setStyleSheet(
            "QPushButton { background: #f8f9fa; color: #495057; border: 1px solid #e9ecef; "
            "border-radius: 6px; padding: 6px 12px; text-align: left; } "
            "QPushButton:hover { background: #e9ecef; }"
        );
        categoryBtn->setCursor(Qt::PointingHandCursor);

        connect(categoryBtn, &QPushButton::clicked, [this, category = it.key()]() {
            emit filterRequested(category == "未分类" ? "" : category, TaskStatus::Pending);
        });

        m_categoriesLayout->addWidget(categoryBtn);
    }
}