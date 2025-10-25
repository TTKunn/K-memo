#ifndef SIMPLEADDWIDGET_H
#define SIMPLEADDWIDGET_H

#include <QWidget>
#include <QTextEdit>
#include <QPushButton>
#include <QLabel>
#include <QString>
#include <QKeyEvent>

QT_BEGIN_NAMESPACE
namespace Ui { class SimpleAddWidget; }
QT_END_NAMESPACE

/**
 * @brief 简化的任务添加组件
 *
 * 提供简洁的任务添加界面，用于快速创建基本任务。
 * 与原有的QuickAddWidget相比，该组件更加简化，专注于核心功能。
 */
class SimpleAddWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SimpleAddWidget(QWidget *parent = nullptr);
    ~SimpleAddWidget();

    /**
     * @brief 清空输入内容
     */
    void clearInput();

    /**
     * @brief 获取输入的任务内容
     * @return 任务内容文本
     */
    QString getTaskContent() const;

    /**
     * @brief 设置输入内容
     * @param content 要设置的内容
     */
    void setTaskContent(const QString &content);

    /**
     * @brief 设置焦点到输入框
     */
    void focusInput();

public slots:
    /**
     * @brief 显示组件并聚焦输入框
     */
    void showAndFocus();

signals:
    /**
     * @brief 确认添加任务信号
     * @param content 任务内容
     */
    void taskAddRequested(const QString &content);

    /**
     * @brief 取消添加信号
     */
    void addCancelled();

private slots:
    /**
     * @brief 处理确认按钮点击
     */
    void onConfirmClicked();

    /**
     * @brief 处理取消按钮点击
     */
    void onCancelClicked();

    /**
     * @brief 处理文本内容变化
     */
    void onTextChanged();

private:
    /**
     * @brief 初始化UI连接
     */
    void setupConnections();

    /**
     * @brief 验证输入内容
     * @return 内容是否有效
     */
    bool validateInput() const;

    /**
     * @brief 更新确认按钮状态
     */
    void updateConfirmButtonState();

    /**
     * @brief 处理键盘事件
     */
    void keyPressEvent(QKeyEvent *event) override;

private:
    Ui::SimpleAddWidget *ui;
};

#endif // SIMPLEADDWIDGET_H