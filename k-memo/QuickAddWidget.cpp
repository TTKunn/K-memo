#include "QuickAddWidget.h"
#include "IconManager.h"
#include <QDateTime>
#include <QMessageBox>

QuickAddWidget::QuickAddWidget(QWidget *parent)
    : QWidget(parent)
    , m_formValid(false)
{
    setupUI();
    setupConnections();
    setupTemplates();
    setupFormValidation();
}

void QuickAddWidget::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);

    // Create scroll area
    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    m_scrollArea->setStyleSheet("QScrollArea { background: transparent; }");

    m_contentWidget = new QWidget();
    m_contentLayout = new QVBoxLayout(m_contentWidget);
    m_contentLayout->setContentsMargins(16, 16, 16, 16);
    m_contentLayout->setSpacing(16);

    // Quick form section
    m_quickFormFrame = createFormSection(QString("快速添加任务"));
    auto quickFormLayout = new QVBoxLayout();

    // Title input
    auto titleLabel = new QLabel("任务标题:");
    titleLabel->setStyleSheet("font-size: 13px; font-weight: 600; color: #495057; margin-bottom: 4px;");

    m_titleEdit = new QLineEdit();
    m_titleEdit->setPlaceholderText("输入任务标题...");
    m_titleEdit->setStyleSheet(
        "QLineEdit { background: white; border: 1px solid #ced4da; "
        "border-radius: 6px; padding: 8px 12px; font-size: 13px; } "
        "QLineEdit:focus { border-color: #4285f4; }"
    );

    // Description input
    auto descLabel = new QLabel("描述（可选）:");
    descLabel->setStyleSheet("font-size: 13px; font-weight: 600; color: #495057; margin: 8px 0px 4px 0px;");

    m_descriptionEdit = new QTextEdit();
    m_descriptionEdit->setPlaceholderText("添加描述...");
    m_descriptionEdit->setMaximumHeight(60);
    m_descriptionEdit->setStyleSheet(
        "QTextEdit { background: white; border: 1px solid #ced4da; "
        "border-radius: 6px; padding: 8px 12px; font-size: 13px; } "
        "QTextEdit:focus { border-color: #4285f4; }"
    );

    // Category and Priority row
    auto categoryPriorityLayout = new QHBoxLayout();

    auto categoryLabel = new QLabel("分类:");
    categoryLabel->setStyleSheet("font-size: 13px; font-weight: 600; color: #495057;");

    m_categoryCombo = new QComboBox();
    m_categoryCombo->setStyleSheet(
        "QComboBox { background: white; border: 1px solid #ced4da; "
        "border-radius: 6px; padding: 6px 12px; font-size: 13px; } "
        "QComboBox:focus { border-color: #4285f4; }"
    );

    auto priorityLabel = new QLabel("优先级:");
    priorityLabel->setStyleSheet("font-size: 13px; font-weight: 600; color: #495057;");

    m_priorityCombo = new QComboBox();
    m_priorityCombo->setStyleSheet(
        "QComboBox { background: white; border: 1px solid #ced4da; "
        "border-radius: 6px; padding: 6px 12px; font-size: 13px; } "
        "QComboBox:focus { border-color: #4285f4; }"
    );

    auto categoryContainer = new QVBoxLayout();
    categoryContainer->addWidget(categoryLabel);
    categoryContainer->addWidget(m_categoryCombo);

    auto priorityContainer = new QVBoxLayout();
    priorityContainer->addWidget(priorityLabel);
    priorityContainer->addWidget(m_priorityCombo);

    categoryPriorityLayout->addLayout(categoryContainer);
    categoryPriorityLayout->addLayout(priorityContainer);

    // Due time section
    m_setDueTimeCheck = new QCheckBox("设置截止日期");
    m_setDueTimeCheck->setStyleSheet("font-size: 13px; color: #495057; margin: 8px 0px 4px 0px;");

    m_dueTimeEdit = new QDateTimeEdit();
    m_dueTimeEdit->setDateTime(QDateTime::currentDateTime().addDays(1));
    m_dueTimeEdit->setEnabled(false);
    m_dueTimeEdit->setStyleSheet(
        "QDateTimeEdit { background: white; border: 1px solid #ced4da; "
        "border-radius: 6px; padding: 6px 12px; font-size: 13px; } "
        "QDateTimeEdit:focus { border-color: #4285f4; } "
        "QDateTimeEdit:disabled { background: #f8f9fa; color: #6c757d; }"
    );

    m_reminderCheck = new QCheckBox("启用提醒");
    m_reminderCheck->setStyleSheet("font-size: 13px; color: #495057; margin: 4px 0px;");

    // Add all form elements
    quickFormLayout->addWidget(titleLabel);
    quickFormLayout->addWidget(m_titleEdit);
    quickFormLayout->addWidget(descLabel);
    quickFormLayout->addWidget(m_descriptionEdit);
    quickFormLayout->addLayout(categoryPriorityLayout);
    quickFormLayout->addWidget(m_setDueTimeCheck);
    quickFormLayout->addWidget(m_dueTimeEdit);
    quickFormLayout->addWidget(m_reminderCheck);

    // Action buttons
    auto buttonLayout = new QHBoxLayout();

    m_quickAddBtn = new QPushButton("快速添加");
    m_quickAddBtn->setStyleSheet(
        "QPushButton { background: #28a745; color: white; border: none; "
        "border-radius: 6px; padding: 8px 16px; font-weight: 500; } "
        "QPushButton:hover { background: #218838; } "
        "QPushButton:disabled { background: #6c757d; }"
    );
    m_quickAddBtn->setEnabled(false);

    m_addWithDetailsBtn = new QPushButton("详细添加");
    m_addWithDetailsBtn->setStyleSheet(
        "QPushButton { background: #17a2b8; color: white; border: none; "
        "border-radius: 6px; padding: 8px 16px; font-weight: 500; } "
        "QPushButton:hover { background: #138496; } "
        "QPushButton:disabled { background: #6c757d; }"
    );
    m_addWithDetailsBtn->setEnabled(false);

    m_clearFormBtn = new QPushButton("清空");
    m_clearFormBtn->setStyleSheet(
        "QPushButton { background: #6c757d; color: white; border: none; "
        "border-radius: 6px; padding: 8px 16px; font-weight: 500; } "
        "QPushButton:hover { background: #5a6268; }"
    );

    buttonLayout->addWidget(m_quickAddBtn);
    buttonLayout->addWidget(m_addWithDetailsBtn);
    buttonLayout->addWidget(m_clearFormBtn);

    quickFormLayout->addLayout(buttonLayout);

    // Add form layout to frame
    auto formFrameLayout = new QVBoxLayout(m_quickFormFrame);
    formFrameLayout->addLayout(quickFormLayout);

    // Quick actions section
    m_quickActionsFrame = createFormSection(QString("快速操作"));
    auto quickActionsLayout = new QVBoxLayout(m_quickActionsFrame);

    auto actionsGridLayout = new QHBoxLayout();
    actionsGridLayout->setSpacing(8);

    m_addWorkBtn = createQuickActionButton("工作任务", "Work", TaskPriority::Normal, "#17a2b8");
    m_addPersonalBtn = createQuickActionButton("个人事务", "Personal", TaskPriority::Low, "#28a745");
    m_addShoppingBtn = createQuickActionButton("购物清单", "Shopping", TaskPriority::Low, "#ffc107");
    m_addReminderBtn = createQuickActionButton("提醒事项", "Reminders", TaskPriority::High, "#dc3545");

    actionsGridLayout->addWidget(m_addWorkBtn);
    actionsGridLayout->addWidget(m_addPersonalBtn);
    actionsGridLayout->addWidget(m_addShoppingBtn);
    actionsGridLayout->addWidget(m_addReminderBtn);

    quickActionsLayout->addLayout(actionsGridLayout);

    // Templates section
    m_templatesFrame = createFormSection(QString("任务模板"));
    auto templatesLayout = new QVBoxLayout(m_templatesFrame);

    auto templateSelectionLayout = new QHBoxLayout();

    m_templateCombo = new QComboBox();
    m_templateCombo->setStyleSheet(
        "QComboBox { background: white; border: 1px solid #ced4da; "
        "border-radius: 6px; padding: 6px 12px; font-size: 13px; } "
        "QComboBox:focus { border-color: #4285f4; }"
    );

    m_useTemplateBtn = new QPushButton("使用模板");
    m_useTemplateBtn->setStyleSheet(
        "QPushButton { background: #6f42c1; color: white; border: none; "
        "border-radius: 6px; padding: 6px 12px; font-weight: 500; } "
        "QPushButton:hover { background: #5a32a3; }"
    );

    templateSelectionLayout->addWidget(m_templateCombo, 1);
    templateSelectionLayout->addWidget(m_useTemplateBtn);

    templatesLayout->addLayout(templateSelectionLayout);

    // Add all sections to content
    m_contentLayout->addWidget(m_quickFormFrame);
    m_contentLayout->addWidget(m_quickActionsFrame);
    m_contentLayout->addWidget(m_templatesFrame);
    m_contentLayout->addStretch();

    m_scrollArea->setWidget(m_contentWidget);
    m_mainLayout->addWidget(m_scrollArea);

    // Populate combos
    populateCategoryCombo();
    IconManager* iconMgr = IconManager::instance();

    m_priorityCombo->addItem(iconMgr->getPriorityIcon(TaskPriority::Low), "低", static_cast<int>(TaskPriority::Low));
    m_priorityCombo->addItem(iconMgr->getPriorityIcon(TaskPriority::Normal), "普通", static_cast<int>(TaskPriority::Normal));
    m_priorityCombo->addItem(iconMgr->getPriorityIcon(TaskPriority::High), "高", static_cast<int>(TaskPriority::High));
    m_priorityCombo->addItem(iconMgr->getPriorityIcon(TaskPriority::Urgent), "紧急", static_cast<int>(TaskPriority::Urgent));
    m_priorityCombo->setCurrentIndex(1); // Default to Normal
}

void QuickAddWidget::setupConnections()
{
    connect(m_quickAddBtn, &QPushButton::clicked, this, &QuickAddWidget::onQuickAddClicked);
    connect(m_addWithDetailsBtn, &QPushButton::clicked, this, &QuickAddWidget::onAddWithDetailsClicked);
    connect(m_clearFormBtn, &QPushButton::clicked, this, &QuickAddWidget::clearForm);
    connect(m_useTemplateBtn, &QPushButton::clicked, this, &QuickAddWidget::onTemplateSelected);

    connect(m_setDueTimeCheck, &QCheckBox::toggled, m_dueTimeEdit, &QDateTimeEdit::setEnabled);

    // Form validation
    connect(m_titleEdit, &QLineEdit::textChanged, this, &QuickAddWidget::onTitleChanged);
    connect(m_categoryCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &QuickAddWidget::onCategoryChanged);
    connect(m_priorityCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &QuickAddWidget::onPriorityChanged);

    // Quick action buttons
    connect(m_addWorkBtn, &QPushButton::clicked, [this]() {
        m_titleEdit->setText("新工作任务");
        m_categoryCombo->setCurrentText("Work");
        m_priorityCombo->setCurrentIndex(1); // Normal
        validateForm();
    });

    connect(m_addPersonalBtn, &QPushButton::clicked, [this]() {
        m_titleEdit->setText("个人任务");
        m_categoryCombo->setCurrentText("Personal");
        m_priorityCombo->setCurrentIndex(0); // Low
        validateForm();
    });

    connect(m_addShoppingBtn, &QPushButton::clicked, [this]() {
        m_titleEdit->setText("购物项目");
        m_categoryCombo->setCurrentText("Shopping");
        m_priorityCombo->setCurrentIndex(0); // Low
        validateForm();
    });

    connect(m_addReminderBtn, &QPushButton::clicked, [this]() {
        m_titleEdit->setText("提醒事项");
        m_categoryCombo->setCurrentText("Reminders");
        m_priorityCombo->setCurrentIndex(2); // High
        m_setDueTimeCheck->setChecked(true);
        m_reminderCheck->setChecked(true);
        validateForm();
    });
}

void QuickAddWidget::setupTemplates()
{
    m_templates.clear();

    // Create some default templates
    Task workMeeting("Team Meeting", "Weekly team sync");
    workMeeting.setCategory("Work");
    workMeeting.setPriority(TaskPriority::Normal);
    m_templates.append(workMeeting);

    Task projectDeadline("Project Deadline", "Complete project deliverables");
    projectDeadline.setCategory("Work");
    projectDeadline.setPriority(TaskPriority::High);
    m_templates.append(projectDeadline);

    Task groceryShopping("Grocery Shopping", "Weekly grocery run");
    groceryShopping.setCategory("Shopping");
    groceryShopping.setPriority(TaskPriority::Low);
    m_templates.append(groceryShopping);

    Task doctorAppointment("Doctor Appointment", "Annual health checkup");
    doctorAppointment.setCategory("Health");
    doctorAppointment.setPriority(TaskPriority::Normal);
    m_templates.append(doctorAppointment);

    updateTemplateCombo();
}

void QuickAddWidget::setupFormValidation()
{
    validateForm();
}

void QuickAddWidget::clearForm()
{
    m_titleEdit->clear();
    m_descriptionEdit->clear();
    m_categoryCombo->setCurrentIndex(0);
    m_priorityCombo->setCurrentIndex(1); // Normal
    m_setDueTimeCheck->setChecked(false);
    m_reminderCheck->setChecked(false);
    m_dueTimeEdit->setDateTime(QDateTime::currentDateTime().addDays(1));
    validateForm();
}

void QuickAddWidget::setDefaultCategory(const QString& category)
{
    m_defaultCategory = category;
    int index = m_categoryCombo->findText(category);
    if (index >= 0) {
        m_categoryCombo->setCurrentIndex(index);
    }
}

void QuickAddWidget::fillFormFromTemplate(const Task& templateTask)
{
    m_titleEdit->setText(templateTask.title());
    m_descriptionEdit->setPlainText(templateTask.description());

    int categoryIndex = m_categoryCombo->findText(templateTask.category());
    if (categoryIndex >= 0) {
        m_categoryCombo->setCurrentIndex(categoryIndex);
    }

    int priorityIndex = static_cast<int>(templateTask.priority()) - 1;
    if (priorityIndex >= 0 && priorityIndex < m_priorityCombo->count()) {
        m_priorityCombo->setCurrentIndex(priorityIndex);
    }

    if (templateTask.dueTime().isValid()) {
        m_setDueTimeCheck->setChecked(true);
        m_dueTimeEdit->setDateTime(templateTask.dueTime());
    }

    m_reminderCheck->setChecked(templateTask.reminderEnabled());
    validateForm();
}

void QuickAddWidget::onQuickAddClicked()
{
    if (!isFormValid()) {
        return;
    }

    Task newTask = createTaskFromForm();
    emit taskCreated(newTask);
    clearForm();
}

void QuickAddWidget::onAddWithDetailsClicked()
{
    if (!isFormValid()) {
        return;
    }

    Task newTask = createTaskFromForm();
    emit detailedEditRequested(newTask);
    clearForm();
}

void QuickAddWidget::onTemplateSelected()
{
    int currentIndex = m_templateCombo->currentIndex();
    if (currentIndex >= 0 && currentIndex < m_templates.size()) {
        fillFormFromTemplate(m_templates[currentIndex]);
    }
}

void QuickAddWidget::onTitleChanged()
{
    validateForm();
}

void QuickAddWidget::onCategoryChanged()
{
    validateForm();
}

void QuickAddWidget::onPriorityChanged()
{
    validateForm();
}

void QuickAddWidget::validateForm()
{
    m_formValid = isFormValid();
    updateAddButtonState();
}

void QuickAddWidget::updateAddButtonState()
{
    m_quickAddBtn->setEnabled(m_formValid);
    m_addWithDetailsBtn->setEnabled(m_formValid);
}

Task QuickAddWidget::createTaskFromForm() const
{
    Task task(m_titleEdit->text(), m_descriptionEdit->toPlainText());

    task.setCategory(m_categoryCombo->currentText());
    task.setPriority(static_cast<TaskPriority>(m_priorityCombo->currentData().toInt()));

    if (m_setDueTimeCheck->isChecked()) {
        task.setDueTime(m_dueTimeEdit->dateTime());
    }

    task.setReminderEnabled(m_reminderCheck->isChecked());

    return task;
}

bool QuickAddWidget::isFormValid() const
{
    return !m_titleEdit->text().trimmed().isEmpty();
}

QFrame* QuickAddWidget::createFormSection(const QString& title)
{
    auto frame = new QFrame();
    frame->setStyleSheet(
        "QFrame { background: white; border: 1px solid #e9ecef; "
        "border-radius: 12px; padding: 16px; }"
    );

    auto layout = new QVBoxLayout(frame);
    layout->setContentsMargins(0, 0, 0, 0);

    auto titleLabel = new QLabel(title);
    titleLabel->setStyleSheet(
        "font-size: 16px; font-weight: 600; color: #212529; "
        "margin-bottom: 12px;"
    );
    layout->addWidget(titleLabel);

    return frame;
}

QPushButton* QuickAddWidget::createQuickActionButton(const QString& text, const QString& category, TaskPriority priority, const QString& color)
{
    auto button = new QPushButton(text);
    button->setStyleSheet(
        QString("QPushButton { background: %1; color: white; border: none; "
               "border-radius: 6px; padding: 8px 12px; font-weight: 500; "
               "font-size: 12px; } "
               "QPushButton:hover { background: %2; }")
        .arg(color)
        .arg(QString(color).replace("#", "#").replace("6", "5").replace("7", "6").replace("8", "7").replace("9", "8"))
    );
    button->setCursor(Qt::PointingHandCursor);
    return button;
}

void QuickAddWidget::populateCategoryCombo()
{
    m_categoryCombo->clear();
    m_categoryCombo->addItem("None", "");
    m_categoryCombo->addItem("Work", "Work");
    m_categoryCombo->addItem("Personal", "Personal");
    m_categoryCombo->addItem("Shopping", "Shopping");
    m_categoryCombo->addItem("Health", "Health");
    m_categoryCombo->addItem("Education", "Education");
    m_categoryCombo->addItem("Finance", "Finance");
    m_categoryCombo->addItem("Home", "Home");
    m_categoryCombo->addItem("Travel", "Travel");
    m_categoryCombo->addItem("Reminders", "Reminders");
}

void QuickAddWidget::updateTemplateCombo()
{
    m_templateCombo->clear();
    m_templateCombo->addItem("选择模板...", -1);

    for (int i = 0; i < m_templates.size(); ++i) {
        m_templateCombo->addItem(m_templates[i].title(), i);
    }
}