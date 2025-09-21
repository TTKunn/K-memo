#include "IconManager.h"

IconManager* IconManager::m_instance = nullptr;

IconManager* IconManager::instance()
{
    if (!m_instance) {
        m_instance = new IconManager();
    }
    return m_instance;
}

IconManager::IconManager()
{
    loadIcons();
}

void IconManager::loadIcons()
{
    // Priority icons
    m_iconCache["priority_low"] = QIcon(":/icons/icons/priority/low.svg");
    m_iconCache["priority_normal"] = QIcon(":/icons/icons/priority/normal.svg");
    m_iconCache["priority_high"] = QIcon(":/icons/icons/priority/high.svg");
    m_iconCache["priority_urgent"] = QIcon(":/icons/icons/priority/urgent.svg");

    // Status icons
    m_iconCache["status_pending"] = QIcon(":/icons/icons/status/pending.svg");
    m_iconCache["status_inprogress"] = QIcon(":/icons/icons/status/inprogress.svg");
    m_iconCache["status_completed"] = QIcon(":/icons/icons/status/completed.svg");
    m_iconCache["status_cancelled"] = QIcon(":/icons/icons/status/cancelled.svg");

    // Action icons
    m_iconCache["action_add"] = QIcon(":/icons/icons/actions/add.svg");
    m_iconCache["action_delete"] = QIcon(":/icons/icons/actions/delete.svg");
    m_iconCache["action_edit"] = QIcon(":/icons/icons/actions/edit.svg");
    m_iconCache["action_complete"] = QIcon(":/icons/icons/actions/complete.svg");

    // Category icons
    m_iconCache["category_work"] = QIcon(":/icons/icons/categories/work.svg");
    m_iconCache["category_personal"] = QIcon(":/icons/icons/categories/personal.svg");
    m_iconCache["category_study"] = QIcon(":/icons/icons/categories/study.svg");
    m_iconCache["category_all"] = QIcon(":/icons/icons/categories/all.svg");

    // UI icons
    m_iconCache["ui_theme_light"] = QIcon(":/icons/icons/ui/theme-light.svg");
    m_iconCache["ui_theme_dark"] = QIcon(":/icons/icons/ui/theme-dark.svg");
    m_iconCache["ui_collapse"] = QIcon(":/icons/icons/ui/collapse.svg");
}

QIcon IconManager::getPriorityIcon(TaskPriority priority) const
{
    switch(priority) {
        case TaskPriority::Low: return m_iconCache["priority_low"];
        case TaskPriority::Normal: return m_iconCache["priority_normal"];
        case TaskPriority::High: return m_iconCache["priority_high"];
        case TaskPriority::Urgent: return m_iconCache["priority_urgent"];
        default: return m_iconCache["priority_normal"];
    }
}

QIcon IconManager::getStatusIcon(TaskStatus status) const
{
    switch(status) {
        case TaskStatus::Pending: return m_iconCache["status_pending"];
        case TaskStatus::InProgress: return m_iconCache["status_inprogress"];
        case TaskStatus::Completed: return m_iconCache["status_completed"];
        case TaskStatus::Cancelled: return m_iconCache["status_cancelled"];
        default: return m_iconCache["status_pending"];
    }
}

QIcon IconManager::getActionIcon(const QString& action) const
{
    return m_iconCache.value("action_" + action, QIcon());
}

QIcon IconManager::getCategoryIcon(const QString& category) const
{
    return m_iconCache.value("category_" + category, m_iconCache["category_all"]);
}

QIcon IconManager::getUIIcon(const QString& uiElement) const
{
    return m_iconCache.value("ui_" + uiElement, QIcon());
}