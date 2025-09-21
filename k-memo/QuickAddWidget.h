#ifndef QUICKADDWIDGET_H
#define QUICKADDWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QTextEdit>
#include <QComboBox>
#include <QPushButton>
#include <QDateTimeEdit>
#include <QFrame>
#include <QLabel>
#include <QCheckBox>
#include <QScrollArea>
#include "models/task.h"

class QuickAddWidget : public QWidget
{
    Q_OBJECT

public:
    explicit QuickAddWidget(QWidget *parent = nullptr);

    void clearForm();
    void setDefaultCategory(const QString& category);
    void fillFormFromTemplate(const Task& templateTask);

public slots:
    void onQuickAddClicked();
    void onAddWithDetailsClicked();
    void onTemplateSelected();

signals:
    void taskCreated(const Task& task);
    void detailedEditRequested(const Task& task);

private slots:
    void onTitleChanged();
    void onCategoryChanged();
    void onPriorityChanged();
    void validateForm();

private:
    void setupUI();
    void setupConnections();
    void setupTemplates();
    void updateAddButtonState();
    Task createTaskFromForm() const;
    bool isFormValid() const;

    // UI Components
    QVBoxLayout* m_mainLayout;
    QScrollArea* m_scrollArea;
    QWidget* m_contentWidget;
    QVBoxLayout* m_contentLayout;

    // Form sections
    QFrame* m_quickFormFrame;
    QFrame* m_templatesFrame;
    QFrame* m_optionsFrame;

    // Quick form
    QLineEdit* m_titleEdit;
    QTextEdit* m_descriptionEdit;
    QComboBox* m_categoryCombo;
    QComboBox* m_priorityCombo;
    QDateTimeEdit* m_dueTimeEdit;
    QCheckBox* m_setDueTimeCheck;
    QCheckBox* m_reminderCheck;

    // Action buttons
    QPushButton* m_quickAddBtn;
    QPushButton* m_addWithDetailsBtn;
    QPushButton* m_clearFormBtn;

    // Templates
    QComboBox* m_templateCombo;
    QPushButton* m_useTemplateBtn;

    // Quick action buttons
    QFrame* m_quickActionsFrame;
    QPushButton* m_addWorkBtn;
    QPushButton* m_addPersonalBtn;
    QPushButton* m_addShoppingBtn;
    QPushButton* m_addReminderBtn;

    // Form state
    bool m_formValid;
    QString m_defaultCategory;

    // Template tasks
    QList<Task> m_templates;

    // Helper methods
    QFrame* createFormSection(const QString& title);
    QPushButton* createQuickActionButton(const QString& text, const QString& category, TaskPriority priority, const QString& color);
    void setupFormValidation();
    void populateCategoryCombo();
    void updateTemplateCombo();
};

#endif // QUICKADDWIDGET_H