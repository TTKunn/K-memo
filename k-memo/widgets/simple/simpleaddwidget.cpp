#include "simpleaddwidget.h"
#include "ui_simpleaddwidget.h"
#include <QKeyEvent>

SimpleAddWidget::SimpleAddWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SimpleAddWidget)
{
    ui->setupUi(this);
    setupConnections();

    // 初始化状态
    updateConfirmButtonState();
}

SimpleAddWidget::~SimpleAddWidget()
{
    delete ui;
}

void SimpleAddWidget::setupConnections()
{
    // 连接按钮信号
    connect(ui->confirmButton, &QPushButton::clicked,
            this, &SimpleAddWidget::onConfirmClicked);
    connect(ui->cancelButton, &QPushButton::clicked,
            this, &SimpleAddWidget::onCancelClicked);

    // 连接文本变化信号
    connect(ui->taskContentTextEdit, &QTextEdit::textChanged,
            this, &SimpleAddWidget::onTextChanged);
}

void SimpleAddWidget::clearInput()
{
    ui->taskContentTextEdit->clear();
    updateConfirmButtonState();
}

QString SimpleAddWidget::getTaskContent() const
{
    return ui->taskContentTextEdit->toPlainText().trimmed();
}

void SimpleAddWidget::setTaskContent(const QString &content)
{
    ui->taskContentTextEdit->setPlainText(content);
    updateConfirmButtonState();
}

void SimpleAddWidget::focusInput()
{
    ui->taskContentTextEdit->setFocus();
}

void SimpleAddWidget::showAndFocus()
{
    show();
    raise();
    activateWindow();
    focusInput();
}

void SimpleAddWidget::onConfirmClicked()
{
    if (!validateInput()) {
        return;
    }

    QString content = getTaskContent();
    if (!content.isEmpty()) {
        emit taskAddRequested(content);
        clearInput();
    }
}

void SimpleAddWidget::onCancelClicked()
{
    clearInput();
    emit addCancelled();
}

void SimpleAddWidget::onTextChanged()
{
    updateConfirmButtonState();
}

bool SimpleAddWidget::validateInput() const
{
    QString content = getTaskContent();
    return !content.isEmpty() && content.length() <= 500; // 限制长度
}

void SimpleAddWidget::updateConfirmButtonState()
{
    bool isValid = validateInput();
    ui->confirmButton->setEnabled(isValid);
}

void SimpleAddWidget::keyPressEvent(QKeyEvent *event)
{
    // 支持快捷键
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        if (event->modifiers() & Qt::ControlModifier) {
            // Ctrl+Enter 确认添加
            if (ui->confirmButton->isEnabled()) {
                onConfirmClicked();
            }
        }
    } else if (event->key() == Qt::Key_Escape) {
        // Esc 取消
        onCancelClicked();
    } else {
        QWidget::keyPressEvent(event);
    }
}