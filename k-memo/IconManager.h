#ifndef ICONMANAGER_H
#define ICONMANAGER_H

#include <QIcon>
#include <QHash>
#include <QString>
#include "models/task.h"

class IconManager
{
public:
    static IconManager* instance();

    // 获取优先级图标
    QIcon getPriorityIcon(TaskPriority priority) const;

    // 获取状态图标
    QIcon getStatusIcon(TaskStatus status) const;

    // 获取操作图标
    QIcon getActionIcon(const QString& action) const;

    // 获取分类图标
    QIcon getCategoryIcon(const QString& category) const;

    // 获取UI图标
    QIcon getUIIcon(const QString& uiElement) const;

private:
    IconManager();
    void loadIcons();

    static IconManager* m_instance;
    QHash<QString, QIcon> m_iconCache;
};

#endif // ICONMANAGER_H