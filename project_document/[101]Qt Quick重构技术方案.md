# K-memo Qt Quick (QML) 重构技术方案

**项目编号：** [101]  
**文档类型：** 阶段性实施方案  
**创建时间：** 2025-10-25  
**预计工作量：** 15-20天  
**技术风险：** 中等  

---

## 📑 目录

- [一、重构背景与目标](#一重构背景与目标)
  - [1.1 当前状况](#当前状况)
  - [1.2 重构目标](#重构目标)
- [二、核心技术方案](#二核心技术方案)
  - [2.1 架构变更：MVVM模式](#架构变更mvvm模式)
  - [2.2 CMake配置变更](#cmake配置变更)
- [三、项目层级架构详解](#三项目层级架构详解)
  - [3.1 整体架构视图](#31-整体架构视图)
  - [3.2 目录结构与职责划分](#32-目录结构与职责划分)
  - [3.3 文件命名规范](#33-文件命名规范)
  - [3.4 架构层次映射](#34-架构层次映射)
  - [3.5 依赖关系规则](#35-依赖关系规则)
- [四、详细实施计划（10个任务）](#四详细实施计划10个任务)
- [五、关键技术实现](#五关键技术实现)
  - [5.1 TaskModel与QML的天然适配](#51-taskmodel与qml的天然适配)
  - [5.2 Main.qml主窗口实现](#52-mainqml主窗口实现)
- [六、性能优化策略](#六性能优化策略)
  - [6.1 ListView优化](#61-listview优化)
  - [6.2 性能目标](#62-性能目标)
- [七、风险管理](#七风险管理)
  - [7.1 回滚策略](#71-回滚策略)
- [八、验收标准](#八验收标准)
  - [8.1 功能验收](#81-功能验收)
  - [8.2 性能验收](#82-性能验收)
  - [8.3 视觉验收](#83-视觉验收)
- [九、关键决策](#九关键决策)
  - [9.1 为什么选择Qt Quick？](#91-为什么选择qt-quick)
  - [9.2 为什么保持C++后端不变？](#92-为什么保持c后端不变)
- [十、参考资料](#十参考资料)
- [十一、总结](#十一总结)
- [附录：项目结构变更](#附录项目结构变更)

---

## 一、重构背景与目标

### 当前状况
K-memo项目使用 **Qt Widgets + QSS** 构建界面：
- 界面样式僵硬，动画效果受限
- 使用emoji图标不够专业
- 移动风格难以实现

### 重构目标
迁移到 **Qt Quick (QML)**，实现：
- ✅ 现代化Material Design风格
- ✅ 流畅的60FPS动画
- ✅ 保持所有C++后端逻辑不变

---

## 二、核心技术方案

### 架构变更：MVVM模式

```
QML UI层 (View)
    ↕ 属性绑定/信号槽
C++ Model层 (保持不变)
    ↕ SQL操作  
SQLite数据库
```

### CMake配置变更

```cmake
# ⚠️ 关键：必须保留Widgets模块用于QSystemTrayIcon
find_package(Qt6 REQUIRED COMPONENTS
    Core Quick Qml QuickControls2 Sql Widgets
)

qt_add_qml_module(k-memo
    URI KMemo.UI
    VERSION 1.0
    QML_FILES
        qml/Main.qml
        qml/components/cards/StatsCard.qml
        qml/components/cards/AddTaskCard.qml
        qml/components/cards/TaskListCard.qml
        qml/components/items/TaskItem.qml
    RESOURCES
        qml/styles/Theme.qml
        qml/styles/Colors.qml
)

# 保留resources.qrc中的图标资源
qt_add_resources(k-memo "app_resources"
    PREFIX "/"
    FILES resources.qrc
)

target_link_libraries(k-memo PRIVATE
    Qt6::Core
    Qt6::Quick
    Qt6::Qml
    Qt6::QuickControls2
    Qt6::Sql
    Qt6::Widgets  # ← 必须保留，TrayManager依赖此模块
)
```

**重要说明：**
- ✅ **必须保留Qt6::Widgets** - TrayManager使用QSystemTrayIcon需要此模块
- ✅ **URI改为KMemo.UI** - 更规范的QML模块命名
- ✅ **RESOURCES添加样式文件** - 包含Theme.qml和Colors.qml
- ✅ **保留resources.qrc** - 现有SVG图标资源继续使用

---

## 三、项目层级架构详解

### 3.1 整体架构视图

K-memo项目采用**分层架构**设计，将代码按职责清晰分离为不同层次和目录：

```
┌─────────────────────────────────────────────────────────────┐
│                    表现层 (Presentation Layer)              │
│  路径: k-memo/qml/                                          │
│  职责: 用户界面、交互逻辑、视觉展示                        │
│  技术: QML + Qt Quick Controls 2 Material                  │
└─────────────────┬───────────────────────────────────────────┘
                  │ 属性绑定 / 信号槽 / Context Property
┌─────────────────┴───────────────────────────────────────────┐
│                    业务逻辑层 (Business Logic Layer)          │
│  路径: k-memo/models/, k-memo/managers/                      │
│  职责: 数据模型、业务规则、系统管理                             │
│  技术: C++ + Qt Core                                        │
└─────────────────┬───────────────────────────────────────────┘
                  │ SQL操作 / 数据库API
┌─────────────────┴───────────────────────────────────────────┐
│                    数据访问层 (Data Access Layer)            │
│  路径: k-memo/database/                                     │
│  职责: 数据库操作、SQL封装、事务管理                            │
│  技术: Qt::Sql + SQLite                                     │
└─────────────────┬───────────────────────────────────────────┘
                  │ SQL查询 / 数据持久化
┌─────────────────┴───────────────────────────────────────────┐
│                    数据存储层 (Data Storage Layer)          │
│  路径: k-memo/data/                                         │
│  职责: 物理数据存储、数据库文件                            │
│  技术: SQLite文件系统                                       │
└─────────────────────────────────────────────────────────────┘
```


---

### 3.2 目录结构与职责划分

#### 📂 `k-memo/` - 项目根目录

```
k-memo/
├── qml/                          # 【表现层】QML UI源码
├── models/                       # 【业务逻辑层】数据模型
├── database/                     # 【数据访问层】数据库操作
├── managers/                     # 【业务逻辑层】系统管理器
├── icons/                        # 【资源层】SVG图标资源
├── data/                         # 【数据存储层】运行时数据
├── test/                         # 【测试层】测试文件
├── build/                        # 【构建层】编译输出（gitignore）
├── main.cpp                      # 【入口层】应用程序入口
├── CMakeLists.txt                # 【配置层】CMake构建配置
└── resources.qrc                 # 【资源层】Qt资源文件
```

---

#### 🎨 `qml/` - 表现层（UI层）

**架构层次：** 表现层 (Presentation Layer)  
**职责范围：**
- 用户界面展示（窗口、页面、组件）
- 用户交互处理（点击、输入、手势）
- 视觉效果实现（动画、过渡、主题）
- 数据绑定和展示逻辑

**目录结构：**
```
qml/
├── Main.qml                      # 主窗口入口
├── components/                   # 可复用UI组件
│   ├── cards/                   # 卡片类组件
│   │   ├── StatsCard.qml        # 统计卡片
│   │   ├── AddTaskCard.qml      # 添加任务卡片
│   │   └── TaskListCard.qml     # 任务列表卡片
│   ├── items/                   # 列表项组件
│   │   └── TaskItem.qml         # 任务项委托
│   └── buttons/                 # 按钮组件（可选）
├── pages/                        # 页面级组件（扩展用）
│   └── SettingsPage.qml         # 设置页面
├── dialogs/                      # 对话框组件
│   └── TaskDetailDialog.qml     # 任务详情对话框
└── styles/                       # 样式和主题配置
    ├── Theme.qml                # 主题单例
    ├── Colors.qml               # 颜色定义
    └── qmldir                   # QML模块定义文件
```

**文件类型规范：**
- **入口文件** (`Main.qml`): 应用主窗口，包含路由和全局状态
- **组件文件** (`*Card.qml`, `*Item.qml`): 可复用的UI组件
- **页面文件** (`*Page.qml`): 完整的页面级UI
- **对话框文件** (`*Dialog.qml`): 模态或非模态对话框
- **样式文件** (`Theme.qml`, `Colors.qml`): 全局样式配置

**依赖规则：**
- ✅ 可以引用：`styles/`、其他`components/`
- ✅ 可以调用：C++暴露的对象和方法（通过context property）
- ❌ 禁止引用：C++头文件（QML不直接include C++）
- ❌ 禁止包含：业务逻辑代码（保持UI组件纯粹）

---

#### 🧩 `models/` - 业务逻辑层（模型层）

**架构层次：** 业务逻辑层 (Business Logic Layer)  
**职责范围：**
- 数据模型定义（Task实体类）
- 数据模型管理（TaskModel数据集合）
- 业务规则验证（数据合法性检查）
- 数据转换和格式化

**目录结构：**
```
models/
├── task.h                        # Task实体类头文件
├── task.cpp                      # Task实体类实现
├── taskmodel.h                   # TaskModel数据模型头文件
└── taskmodel.cpp                 # TaskModel数据模型实现
```

**文件职责：**
- **`task.h/.cpp`**: 
  - 定义Task实体类（id, title, description, priority, status等）
  - 实现枚举类型（TaskPriority, TaskStatus）
  - 提供JSON序列化/反序列化方法
  - 实现数据验证逻辑

- **`taskmodel.h/.cpp`**: 
  - 继承QAbstractListModel，适配QML ListView
  - 实现roleNames()暴露数据角色给QML
  - 提供增删改查接口（addTask, removeTask, updateTask）
  - 管理任务列表数据（m_tasks）
  - 连接DatabaseManager进行持久化

**依赖规则：**
- ✅ 可以引用：`database/`（调用DatabaseManager）
- ✅ 可以使用：Qt Core模块（QObject, QAbstractListModel等）
- ❌ 禁止引用：`qml/`（模型不依赖UI）
- ❌ 禁止引用：`managers/`（保持模型独立性）

---

#### 💾 `database/` - 数据访问层

**架构层次：** 数据访问层 (Data Access Layer)  
**职责范围：**
- 数据库连接管理
- SQL语句封装
- 事务管理
- 数据库版本迁移

**目录结构：**
```
database/
├── databasemanager.h             # 数据库管理器头文件
└── databasemanager.cpp           # 数据库管理器实现
```

**DatabaseManager职责：**
- **单例模式** - 确保全局唯一数据库连接
- **初始化** - 创建数据库文件和表结构
- **CRUD操作** - insertTask, updateTask, deleteTask, getAllTasks
- **事务支持** - 批量操作的事务包装
- **错误处理** - SQL错误捕获和日志记录
- **版本管理** - 数据库schema版本迁移

**依赖规则：**
- ✅ 可以使用：Qt::Sql模块（QSqlDatabase, QSqlQuery等）
- ✅ 可以引用：`models/task.h`（返回Task对象）
- ❌ 禁止引用：`models/taskmodel.h`（避免循环依赖）
- ❌ 禁止引用：UI层和管理器层

---

#### ⚙️ `managers/` - 业务逻辑层（管理器层）

**架构层次：** 业务逻辑层 (Business Logic Layer)  
**职责范围：**
- 系统级功能管理
- 操作系统集成
- 跨模块协调

**目录结构：**
```
managers/
├── traymanager.h                 # 系统托盘管理器头文件
├── traymanager.cpp               # 系统托盘管理器实现
├── notificationmanager.h         # 通知管理器头文件
└── notificationmanager.cpp       # 通知管理器实现
```

**TrayManager职责：**
- 创建和管理QSystemTrayIcon
- 实现托盘菜单（显示、隐藏、退出）
- 更新托盘图标和提示信息
- **QML接口**（重构后新增）：
  - `Q_INVOKABLE void showMainWindow()`
  - `Q_INVOKABLE void hideMainWindow()`
  - `Q_INVOKABLE void updateTaskCount(int count)`

**NotificationManager职责：**
- 封装系统通知功能
- 支持Windows Toast通知（未来扩展）
- 当前使用QSystemTrayIcon::showMessage
- **QML接口**（重构后新增）：
  - `Q_INVOKABLE void showNotification(QString title, QString message)`

**依赖规则：**
- ✅ 可以使用：Qt::Widgets（QSystemTrayIcon）
- ✅ 可以引用：QML引擎（持有QQmlApplicationEngine指针）
- ❌ 禁止引用：`models/`（管理器不直接操作数据）
- ❌ 禁止引用：`database/`（通过Model层间接访问）

---

#### 🎨 `icons/` - 资源层

**架构层次：** 资源层 (Resource Layer)  
**职责范围：**
- 存储SVG矢量图标
- 按功能分类组织
- 供QML和C++引用

**目录结构：**
```
icons/
├── actions/                      # 操作图标
│   ├── add.svg                  # 添加图标
│   ├── delete.svg               # 删除图标
│   ├── edit.svg                 # 编辑图标
│   └── complete.svg             # 完成图标
├── categories/                   # 分类图标
│   ├── work.svg                 # 工作分类
│   ├── study.svg                # 学习分类
│   ├── personal.svg             # 个人分类
│   └── all.svg                  # 全部分类
├── priority/                     # 优先级图标
│   ├── low.svg                  # 低优先级
│   ├── normal.svg               # 普通优先级
│   ├── high.svg                 # 高优先级
│   └── urgent.svg               # 紧急优先级
├── status/                       # 状态图标
│   ├── pending.svg              # 待办状态
│   ├── inprogress.svg           # 进行中状态
│   ├── completed.svg            # 已完成状态
│   └── cancelled.svg            # 已取消状态
└── ui/                          # UI界面图标
    ├── theme-light.svg          # 浅色主题图标
    ├── theme-dark.svg           # 暗色主题图标
    └── collapse.svg             # 折叠图标
```

**命名规范：**
- 全部小写字母
- 单词间用连字符分隔（kebab-case）
- 描述性命名，见名知意
- 统一使用`.svg`扩展名

**在QML中使用：**
```qml
Image {
    source: "qrc:/icons/actions/add.svg"
    sourceSize: Qt.size(24, 24)  // ← 必须设置sourceSize优化性能
}
```

**资源文件组织（resources.qrc）：**
```xml
<RCC>
    <qresource prefix="/">
        <file>icons/actions/add.svg</file>
        <file>icons/actions/delete.svg</file>
        <file>icons/actions/edit.svg</file>
        <file>icons/priority/low.svg</file>
        <file>icons/priority/normal.svg</file>
        <file>icons/priority/high.svg</file>
        <file>icons/priority/urgent.svg</file>
        <!-- 其他图标... -->
    </qresource>
</RCC>
```

---

#### 📊 `data/` - 数据存储层

**架构层次：** 数据存储层 (Data Storage Layer)  
**职责范围：**
- 存储SQLite数据库文件
- 存储用户配置文件
- 运行时生成的数据

**目录结构：**
```
data/
├── .gitkeep                      # Git占位文件
└── (运行时生成的文件)
    ├── tasks.db                 # SQLite数据库文件（运行时生成）
    ├── config.json              # 配置文件（运行时生成）
    └── logs/                    # 日志文件（可选）
```

**重要说明：**
- ⚠️ `data/`目录已添加到`.gitignore`
- ⚠️ 仅`.gitkeep`文件纳入版本控制
- ⚠️ 所有实际数据文件都不提交到Git

---

#### 🧪 `test/` - 测试层

**架构层次：** 测试层 (Test Layer)  
**职责范围：**
- 单元测试代码
- 集成测试代码
- 临时测试脚本

**规范要求：**
- 仅保留**长期测试文件**
- 临时测试文件使用后必须删除
- 测试文件命名：`test_*.cpp`或`*_test.cpp`

---

#### 🔧 `build/` - 构建层

**架构层次：** 构建层 (Build Layer)  
**职责范围：**
- CMake生成的构建文件
- 编译器输出的中间文件
- 可执行文件输出

**重要说明：**
- ⚠️ `build/`目录已添加到`.gitignore`
- ⚠️ 所有文件都不纳入版本控制
- ⚠️ 每次clean build会清空此目录

---

### 3.3 文件命名规范

#### QML文件命名规范

| 文件类型 | 命名规范 | 示例 |
|---------|---------|------|
| 主窗口 | `Main.qml` | `Main.qml` |
| 组件 | PascalCase + 功能描述 | `StatsCard.qml`, `TaskItem.qml` |
| 页面 | PascalCase + `Page` | `SettingsPage.qml` |
| 对话框 | PascalCase + `Dialog` | `TaskDetailDialog.qml` |
| 样式单例 | PascalCase单一名词 | `Theme.qml`, `Colors.qml` |

#### C++文件命名规范

| 文件类型 | 命名规范 | 示例 |
|---------|---------|------|
| 类头文件 | 小写+类名 | `task.h`, `taskmodel.h` |
| 类实现文件 | 小写+类名 | `task.cpp`, `taskmodel.cpp` |
| 管理器类 | 小写+manager | `traymanager.h`, `notificationmanager.h` |
| 入口文件 | `main.cpp` | `main.cpp` |

#### 资源文件命名规范

| 文件类型 | 命名规范 | 示例 |
|---------|---------|------|
| SVG图标 | 小写+连字符 | `add.svg`, `theme-dark.svg` |
| 资源文件 | 小写+功能描述 | `resources.qrc` |

---

### 3.4 架构层次映射

#### 从架构层到目录的映射关系

```
架构层次                  对应目录                 核心职责
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
表现层                   qml/                    UI展示、用户交互
(Presentation)           └── components/         可复用UI组件
                         └── pages/              页面级UI
                         └── dialogs/            对话框
                         └── styles/             主题样式

业务逻辑层                models/                 数据模型、业务规则
(Business Logic)         managers/               系统管理、集成功能

数据访问层                database/               SQL操作、事务管理
(Data Access)            

数据存储层                data/                   物理存储、数据库文件
(Data Storage)           

资源层                    icons/                  图标资源
(Resource)               resources.qrc           资源配置

配置层                    CMakeLists.txt          构建配置
(Configuration)          

入口层                    main.cpp                应用初始化
(Entry Point)            
```

#### 各层间的通信方式

```
表现层 (QML)
    ↕ 
    方式1: 属性绑定 (Property Binding)
           taskModel.getTaskCount() → QML自动更新
    
    方式2: 信号槽 (Signals & Slots)
           QML: onClicked → C++: handleClick()
           C++: dataChanged → QML: 自动刷新ListView
    
    方式3: Context Property
           engine.rootContext()->setContextProperty("taskModel", model)
    
业务逻辑层 (C++ Models/Managers)
    ↕
    方式: 直接函数调用
          TaskModel::addTask() → DatabaseManager::insertTask()
    
数据访问层 (C++ Database)
    ↕
    方式: SQL API调用
          QSqlQuery::exec() → SQLite数据库
    
数据存储层 (SQLite File)
```

---

### 3.5 依赖关系规则

#### 允许的依赖关系

```
main.cpp
    ↓ 可以引用
qml/ + models/ + managers/ + database/
    ↓ 可以引用
models/
    ↓ 可以引用
database/

managers/
    ↓ 可以引用
QML引擎 (用于操作窗口)
```

#### 禁止的依赖关系

```
❌ models/ → qml/          # 模型层不依赖UI层
❌ database/ → models/     # 数据访问层不依赖模型层（避免循环）
❌ qml/ → database/        # UI层不直接访问数据库
❌ managers/ → models/     # 管理器不直接操作数据模型
```

#### 依赖注入原则

遵循**依赖倒置原则**（DIP）：
- 高层模块不依赖低层模块，两者都依赖抽象
- QML通过Context Property访问C++对象（依赖抽象接口）
- Models通过信号通知UI更新（依赖Qt信号机制）
- 使用单例模式减少全局依赖（DatabaseManager）

---

### 3.6 扩展指南

#### 添加新的QML组件

1. 确定组件类型（卡片/列表项/对话框）
2. 在对应`qml/components/`子目录创建`.qml`文件
3. 遵循PascalCase命名规范
4. 在Main.qml或父组件中import使用

#### 添加新的C++类

1. 确定类的职责层次（模型/管理器/数据访问）
2. 在对应目录创建`.h`和`.cpp`文件
3. 遵循小写命名规范
4. 在CMakeLists.txt的PROJECT_SOURCES中添加
5. 如需暴露给QML，使用`Q_INVOKABLE`或Context Property

#### 添加新的图标

1. 确定图标类别（actions/categories/priority/status/ui）
2. 在对应`icons/`子目录添加`.svg`文件
3. 遵循kebab-case命名规范
4. 在`resources.qrc`中添加文件引用
5. 在QML中通过`qrc:/icons/`路径引用

---

### 3.7 QML模块化最佳实践

#### qmldir模块定义文件

**创建 `qml/qmldir` 文件：**

```
# K-memo QML模块定义
module KMemo.UI

# 单例对象（全局唯一）
singleton Theme 1.0 styles/Theme.qml
singleton Colors 1.0 styles/Colors.qml

# 可复用组件（卡片类）
StatsCard 1.0 components/cards/StatsCard.qml
AddTaskCard 1.0 components/cards/AddTaskCard.qml
TaskListCard 1.0 components/cards/TaskListCard.qml

# 可复用组件（列表项）
TaskItem 1.0 components/items/TaskItem.qml

# 对话框组件
TaskDetailDialog 1.0 dialogs/TaskDetailDialog.qml
```

---

#### Theme单例实现

**创建 `qml/styles/Theme.qml`（单例模式）：**

```qml
pragma Singleton
import QtQuick

QtObject {
    id: theme

    // 主题状态
    property bool isDark: false

    // 颜色定义
    readonly property color primaryColor: isDark ? "#1976D2" : "#2196F3"
    readonly property color accentColor: isDark ? "#FF4081" : "#FF5722"
    readonly property color backgroundColor: isDark ? "#303030" : "#FAFAFA"
    readonly property color surfaceColor: isDark ? "#424242" : "#FFFFFF"
    readonly property color textColor: isDark ? "#FFFFFF" : "#212121"
    readonly property color secondaryTextColor: isDark ? "#B0B0B0" : "#757575"

    // 优先级颜色
    readonly property color priorityLow: "#4CAF50"      // 绿色
    readonly property color priorityNormal: "#2196F3"   // 蓝色
    readonly property color priorityHigh: "#FF9800"     // 橙色
    readonly property color priorityUrgent: "#F44336"   // 红色

    // 状态颜色
    readonly property color statusPending: "#9E9E9E"     // 灰色
    readonly property color statusInProgress: "#2196F3"  // 蓝色
    readonly property color statusCompleted: "#4CAF50"   // 绿色
    readonly property color statusCancelled: "#F44336"   // 红色

    // 尺寸定义
    readonly property int spacingSmall: 4
    readonly property int spacingMedium: 8
    readonly property int spacingLarge: 16
    readonly property int spacingXLarge: 24

    readonly property int radiusSmall: 4
    readonly property int radiusMedium: 8
    readonly property int radiusLarge: 12

    readonly property int fontSizeSmall: 12
    readonly property int fontSizeMedium: 14
    readonly property int fontSizeLarge: 18
    readonly property int fontSizeXLarge: 24

    // 动画时长
    readonly property int animationFast: 150
    readonly property int animationNormal: 200
    readonly property int animationSlow: 300

    // 根据优先级获取颜色
    function getPriorityColor(priority) {
        switch(priority) {
            case 1: return priorityLow
            case 2: return priorityNormal
            case 3: return priorityHigh
            case 4: return priorityUrgent
            default: return priorityNormal
        }
    }

    // 根据状态获取颜色
    function getStatusColor(status) {
        switch(status) {
            case 0: return statusPending
            case 1: return statusInProgress
            case 2: return statusCompleted
            case 3: return statusCancelled
            default: return statusPending
        }
    }

    // 切换主题
    function toggleTheme() {
        isDark = !isDark
    }
}
```

---

#### 在CMakeLists.txt中配置QML模块

```cmake
# 定义QML模块
qt_add_qml_module(k-memo
    URI KMemo.UI
    VERSION 1.0
    QML_FILES
        qml/Main.qml
        qml/components/cards/StatsCard.qml
        qml/components/cards/AddTaskCard.qml
        qml/components/cards/TaskListCard.qml
        qml/components/items/TaskItem.qml
        qml/dialogs/TaskDetailDialog.qml
    RESOURCES
        qml/styles/Theme.qml
        qml/styles/Colors.qml
    NO_PLUGIN  # 不生成C++插件，仅QML模块
)
```

---

#### 在QML中导入和使用模块

```qml
import QtQuick
import QtQuick.Controls.Material
import KMemo.UI 1.0  // ← 导入自定义模块
import KMemo.Enums 1.0  // ← 导入枚举命名空间

ApplicationWindow {
    id: mainWindow

    // 使用Theme单例
    Material.theme: Theme.isDark ? Material.Dark : Material.Light
    Material.primary: Theme.primaryColor

    Rectangle {
        // 使用主题颜色
        color: Theme.backgroundColor
        radius: Theme.radiusMedium

        Text {
            text: "待办任务"
            color: Theme.textColor
            font.pixelSize: Theme.fontSizeLarge
        }

        // 使用优先级颜色
        Rectangle {
            color: Theme.getPriorityColor(TaskEnums.Urgent)
            width: 4
            height: parent.height
        }
    }

    // 使用自定义组件（无需指定路径）
    StatsCard {
        totalCount: taskModel.getTaskCount()
    }

    // 主题切换按钮
    Button {
        text: Theme.isDark ? "浅色模式" : "深色模式"
        onClicked: Theme.toggleTheme()
    }
}
```

---

#### 模块化的优势

| 优势 | 说明 |
|-----|------|
| **代码复用** | 组件可在多个地方使用，无需重复代码 |
| **集中管理** | 主题、颜色、尺寸集中在Theme单例，统一修改 |
| **类型安全** | qmldir提供类型检查，减少运行时错误 |
| **自动补全** | Qt Creator可识别模块，提供代码补全 |
| **版本控制** | 模块版本号便于管理组件兼容性 |
| **性能优化** | QML引擎可以缓存和优化模块 |

---

## 四、详细实施计划（11个任务）

### 任务1：Qt Quick环境配置与CMake重构（1天）
- ✅ 修改CMakeLists.txt添加Qt6 Quick/Qml/QuickControls2模块
- ✅ **保留Qt6::Widgets模块**（系统托盘必需）
- ✅ 配置qt_add_qml_module定义KMemo.UI模块
- ✅ 配置qt_add_resources保留现有SVG图标
- 创建qml/目录结构（components/cards, components/items, styles, dialogs）
- 配置Qt Creator的QML支持和代码补全

---

### 任务2：C++后端适配与枚举暴露（2天）

**2.1 创建TaskEnums命名空间（0.5天）**
- 创建models/taskenums.h文件
- 使用Q_NAMESPACE和Q_ENUM_NS暴露枚举给QML
- 在CMakeLists.txt中添加taskenums.h

**2.2 适配TrayManager（1天）**
- 修改构造函数接受`QQmlApplicationEngine*`参数
- 添加Q_INVOKABLE方法（showMainWindow, hideMainWindow, toggleMainWindow等）
- 实现通过QMetaObject::invokeMethod控制QML窗口
- 修改createContextMenu托盘菜单

**2.3 适配NotificationManager（0.5天）**
- 添加Q_INVOKABLE showNotification方法
- 确保与TrayManager的集成正常

---

### 任务3：main.cpp重构与QML引擎初始化（1天）
- ⚠️ **保持使用QApplication**（不能改为QGuiApplication）
- 初始化QQmlApplicationEngine
- 注册TaskEnums命名空间（qmlRegisterUnclassifiedModule）
- 创建TaskModel、TrayManager、NotificationManager单例
- 设置Context Property暴露C++对象到QML
- 加载Main.qml并处理错误

---

### 任务4：QML模块化基础架构（1天）

**4.1 创建Theme单例（0.5天）**
- 创建qml/styles/Theme.qml（pragma Singleton）
- 定义颜色、尺寸、动画时长等主题属性
- 实现getPriorityColor()和getStatusColor()辅助函数
- 实现toggleTheme()主题切换

**4.2 创建qmldir模块定义（0.5天）**
- 创建qml/qmldir文件
- 注册Theme和Colors单例
- 注册所有组件（StatsCard, AddTaskCard, TaskListCard, TaskItem）

---

### 任务5：Main.qml主窗口框架实现（1-2天）
- 实现ApplicationWindow基础框架
- 配置Material Design主题（绑定Theme单例）
- 实现无边框窗口（flags: Qt.Window | Qt.FramelessWindowHint）
- 实现自定义标题栏ToolBar（拖动窗口、最小化、隐藏按钮）
- 实现窗口拖动逻辑（MouseArea计算坐标偏移）
- 实现圆角阴影效果（DropShadow）
- 处理onClosing事件（隐藏到托盘而不是退出）
- 创建三卡片垂直布局（ColumnLayout）

---

### 任务6：StatsCard统计卡片组件实现（1天）
- 创建components/cards/StatsCard.qml
- 显示任务总数、已完成、待办数量
- 数据绑定到taskModel（getTaskCount, getCompletedCount等）
- 添加高级功能按钮和菜单按钮
- 实现卡片样式（Material风格）

---

### 任务7：AddTaskCard添加任务卡片组件实现（1天）
- 创建components/cards/AddTaskCard.qml
- 实现TextField标题输入框
- 实现TextArea描述输入框（可选）
- 添加确认/取消按钮
- 实现输入验证逻辑（title不能为空）
- 定义taskSubmitted信号与TaskModel集成
- 添加输入动画和焦点管理

---

### 任务8：TaskListCard和TaskItem任务列表组件实现（2天）

**8.1 TaskItem委托组件（1天）**
- 创建components/items/TaskItem.qml
- 使用required property优化性能
- 实现CheckBox完成状态切换
- 显示任务标题、描述、优先级
- 优先级颜色指示条（使用Theme.getPriorityColor）
- 完成任务划线动画（Behavior on opacity）
- 添加hover和点击效果

**8.2 TaskListCard列表容器（1天）**
- 创建components/cards/TaskListCard.qml
- 实现ListView性能优化（cacheBuffer, reuseItems, clip）
- 配置add/remove/displaced动画
- 处理空状态显示
- 实现列表滚动条样式

---

### 任务9：系统托盘和通知集成测试（1天）
- 测试托盘图标显示和菜单功能
- 测试窗口显示/隐藏/切换逻辑
- 测试任务数量更新到托盘tooltip
- 测试通知功能（任务添加、完成等）
- 测试双击托盘图标切换窗口
- 测试退出应用流程

---

### 任务10：性能优化与动画完善（2天）

**10.1 性能优化（1天）**
- ListView性能测试（加载1000任务）
- 图标加载优化（确保所有Image设置sourceSize）
- 复杂组件Loader延迟加载
- 使用QML Profiler检测性能瓶颈
- 内存占用和启动时间优化

**10.2 动画效果完善（1天）**
- 完善所有按钮点击动画
- 完善卡片展开/折叠过渡
- 完善列表项添加/删除动画
- 实现主题切换过渡动画
- 确保60FPS流畅度

---

### 任务11：全面测试、Bug修复与文档更新（2天）

**11.1 功能测试（1天）**
- 所有CRUD操作测试
- 数据持久化测试
- 系统托盘功能测试
- 主题切换测试
- 边界条件测试（空数据、大量数据）

**11.2 文档更新（1天）**
- 更新[000]功能模块文档.md
- 更新CLAUDE.md开发指南
- 更新README.md项目说明
- 创建QML开发规范文档（可选）

---

**总计：16-20天**

**关键里程碑：**
- Day 3: C++后端适配完成，可以开始QML开发
- Day 7: 主窗口和基础组件完成，可以看到界面
- Day 12: 所有组件完成，可以执行基本任务管理
- Day 16: 性能优化完成，通过验收测试
- Day 18-20: 文档完善，项目交付

---

## 五、关键技术实现

### 5.1 TaskModel与QML的天然适配

**优势：** TaskModel已实现`roleNames()`，无需修改即可在QML中使用！

```cpp
// models/taskmodel.h - 已有实现 ✅
QHash<int, QByteArray> roleNames() const override {
    return {
        {TitleRole, "title"},
        {StatusRole, "status"},
        {PriorityRole, "priority"},
    };
}
```

```qml
// QML中直接使用
ListView {
    model: taskModel
    delegate: TaskItem {
        taskTitle: model.title
        taskStatus: model.status
        taskPriority: model.priority
    }
}
```

---

### 5.2 枚举类型暴露到QML

**步骤1：创建TaskEnums命名空间文件**

创建新文件 `models/taskenums.h`：

```cpp
#ifndef TASKENUMS_H
#define TASKENUMS_H

#include <QObject>

// 使用Q_NAMESPACE将枚举暴露给QML
namespace TaskEnums {
    Q_NAMESPACE

    // 任务优先级枚举
    enum TaskPriority {
        Low = 1,
        Normal = 2,
        High = 3,
        Urgent = 4
    };
    Q_ENUM_NS(TaskPriority)

    // 任务状态枚举
    enum TaskStatus {
        Pending = 0,
        InProgress = 1,
        Completed = 2,
        Cancelled = 3
    };
    Q_ENUM_NS(TaskStatus)
}

#endif // TASKENUMS_H
```

**步骤2：在main.cpp中注册命名空间**

```cpp
#include "models/taskenums.h"

int main(int argc, char *argv[])
{
    // 注册枚举命名空间到QML
    qmlRegisterUnclassifiedModule("KMemo.Enums", 1, 0);

    // ... 其他初始化代码
}
```

**步骤3：在QML中使用枚举**

```qml
import KMemo.Enums 1.0

Rectangle {
    // 使用枚举值
    property int taskPriority: TaskEnums.High
    property int taskStatus: TaskEnums.Pending

    // 条件判断
    color: taskPriority === TaskEnums.Urgent ? "red" : "blue"
}
```

---

### 5.3 main.cpp完整实现方案

```cpp
#include <QApplication>  // ⚠️ 必须用QApplication，不能用QGuiApplication
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QIcon>

// Models
#include "models/taskmodel.h"
#include "models/taskenums.h"

// Managers
#include "managers/traymanager.h"
#include "managers/notificationmanager.h"

// Database
#include "database/databasemanager.h"

int main(int argc, char *argv[])
{
    // ⚠️ 关键：必须使用QApplication而不是QGuiApplication
    // 原因：QSystemTrayIcon（系统托盘）需要Widgets模块支持
    QApplication app(argc, argv);

    // 设置应用程序信息
    app.setApplicationName("K-memo");
    app.setOrganizationName("K-memo");
    app.setApplicationVersion("2.0.0");

    // 1. 注册枚举命名空间到QML
    qmlRegisterUnclassifiedModule("KMemo.Enums", 1, 0);

    // 2. 初始化数据库
    DatabaseManager* db = DatabaseManager::instance();
    if (!db->initialize()) {
        qCritical() << "数据库初始化失败！";
        return -1;
    }

    // 3. 创建TaskModel单例
    TaskModel* taskModel = new TaskModel(&app);

    // 4. 创建QML引擎
    QQmlApplicationEngine engine;

    // 5. 创建TrayManager（需要引擎指针）
    TrayManager* trayManager = new TrayManager(&engine, &app);

    // 6. 创建NotificationManager
    NotificationManager* notificationManager =
        new NotificationManager(trayManager, &app);

    // 7. 设置Context Property（暴露C++对象给QML）
    QQmlContext* rootContext = engine.rootContext();
    rootContext->setContextProperty("taskModel", taskModel);
    rootContext->setContextProperty("trayManager", trayManager);
    rootContext->setContextProperty("notificationManager", notificationManager);

    // 8. 加载主QML文件
    const QUrl url(QStringLiteral("qrc:/qml/Main.qml"));

    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
        &app, [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl) {
                QCoreApplication::exit(-1);
            }
        }, Qt::QueuedConnection);

    engine.load(url);

    if (engine.rootObjects().isEmpty()) {
        qCritical() << "QML加载失败！";
        return -1;
    }

    // 9. 显示系统托盘
    trayManager->show();

    return app.exec();
}
```

**关键要点说明：**

| 技术点 | 说明 |
|-------|------|
| **QApplication vs QGuiApplication** | 必须用QApplication，QSystemTrayIcon依赖Widgets模块 |
| **Context Property** | 用于暴露全局单例对象（taskModel, trayManager等） |
| **qmlRegisterUnclassifiedModule** | 注册枚举命名空间，使QML可以使用TaskEnums |
| **引擎初始化顺序** | 先创建C++对象，再设置Context Property，最后加载QML |
| **错误处理** | 检查数据库初始化和QML加载是否成功 |

---

### 5.4 TrayManager的QML接口适配

**修改 `managers/traymanager.h`：**

```cpp
#ifndef TRAYMANAGER_H
#define TRAYMANAGER_H

#include <QObject>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QQmlApplicationEngine>  // ← 新增

class TrayManager : public QObject
{
    Q_OBJECT

public:
    // ⚠️ 构造函数改为接受QQmlApplicationEngine指针
    explicit TrayManager(QQmlApplicationEngine* engine, QObject *parent = nullptr);
    ~TrayManager();

    bool isSystemTrayAvailable() const;
    void show();
    void hide();
    bool isVisible() const;

    // QML接口方法（添加Q_INVOKABLE）
    Q_INVOKABLE void showMainWindow();
    Q_INVOKABLE void hideMainWindow();
    Q_INVOKABLE void toggleMainWindow();
    Q_INVOKABLE void updateTaskCount(int pendingCount, int overdueCount);
    Q_INVOKABLE void showNotification(const QString& title, const QString& message);
    Q_INVOKABLE void quit();

signals:
    void showRequested();
    void hideRequested();
    void quitRequested();

private slots:
    void onTrayIconActivated(QSystemTrayIcon::ActivationReason reason);
    void onMessageClicked();

private:
    void createTrayIcon();
    void createContextMenu();
    void updateTooltip();
    QIcon createTaskCountIcon(int count);

    QQmlApplicationEngine* m_engine;  // ← 新增：持有引擎指针
    QObject* m_mainWindow;            // ← 新增：QML窗口对象
    QSystemTrayIcon* m_trayIcon;
    QMenu* m_contextMenu;

    // Actions
    QAction* m_showAction;
    QAction* m_hideAction;
    QAction* m_quitAction;

    // Task counts
    int m_pendingCount;
    int m_overdueCount;
};

#endif // TRAYMANAGER_H
```

**修改 `managers/traymanager.cpp` 关键方法：**

```cpp
#include "traymanager.h"
#include <QMetaObject>

TrayManager::TrayManager(QQmlApplicationEngine* engine, QObject *parent)
    : QObject(parent)
    , m_engine(engine)
    , m_mainWindow(nullptr)
    , m_pendingCount(0)
    , m_overdueCount(0)
{
    createTrayIcon();
    createContextMenu();
}

void TrayManager::showMainWindow()
{
    // 获取QML窗口对象（首次调用时）
    if (!m_mainWindow && m_engine) {
        QList<QObject*> roots = m_engine->rootObjects();
        if (!roots.isEmpty()) {
            m_mainWindow = roots.first();
        }
    }

    // 调用QML窗口的show()和raise()方法
    if (m_mainWindow) {
        QMetaObject::invokeMethod(m_mainWindow, "show");
        QMetaObject::invokeMethod(m_mainWindow, "raise");
        QMetaObject::invokeMethod(m_mainWindow, "requestActivate");
    }
}

void TrayManager::hideMainWindow()
{
    if (!m_mainWindow && m_engine) {
        QList<QObject*> roots = m_engine->rootObjects();
        if (!roots.isEmpty()) {
            m_mainWindow = roots.first();
        }
    }

    if (m_mainWindow) {
        QMetaObject::invokeMethod(m_mainWindow, "hide");
    }
}

void TrayManager::toggleMainWindow()
{
    if (!m_mainWindow && m_engine) {
        QList<QObject*> roots = m_engine->rootObjects();
        if (!roots.isEmpty()) {
            m_mainWindow = roots.first();
        }
    }

    if (m_mainWindow) {
        // 检查窗口是否可见
        bool isVisible = false;
        QMetaObject::invokeMethod(m_mainWindow, "visible",
                                 Qt::DirectConnection,
                                 Q_RETURN_ARG(bool, isVisible));

        if (isVisible) {
            hideMainWindow();
        } else {
            showMainWindow();
        }
    }
}

void TrayManager::updateTaskCount(int pendingCount, int overdueCount)
{
    m_pendingCount = pendingCount;
    m_overdueCount = overdueCount;
    updateTooltip();
    // 可选：更新托盘图标显示任务数量
}

void TrayManager::showNotification(const QString& title, const QString& message)
{
    if (m_trayIcon && m_trayIcon->isVisible()) {
        m_trayIcon->showMessage(title, message,
                               QSystemTrayIcon::Information, 10000);
    }
}

void TrayManager::quit()
{
    emit quitRequested();
    QCoreApplication::quit();
}

void TrayManager::updateTooltip()
{
    QString tooltip = QString("K-memo\n待办: %1 | 逾期: %2")
                        .arg(m_pendingCount)
                        .arg(m_overdueCount);

    if (m_trayIcon) {
        m_trayIcon->setToolTip(tooltip);
    }
}
```

---

### 5.5 Main.qml主窗口实现

```qml
import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import KMemo.UI 1.0  // 导入自定义QML模块

ApplicationWindow {
    id: mainWindow
    width: 400
    height: 700
    visible: true
    title: "K-memo"

    // 窗口属性配置
    flags: Qt.Window | Qt.FramelessWindowHint  // 无边框普通窗口
    color: "transparent"  // 透明背景，用于实现圆角效果

    // Material Design主题配置
    Material.theme: Theme.isDark ? Material.Dark : Material.Light
    Material.primary: Material.Blue
    Material.accent: Material.Teal

    // 自定义标题栏（用于拖动和关闭）
    header: ToolBar {
        Material.background: Material.Blue
        Material.foreground: "white"

        RowLayout {
            anchors.fill: parent
            spacing: 8

            // 拖动区域
            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true

                Label {
                    anchors.centerIn: parent
                    text: "K-memo"
                    font.pixelSize: 18
                    font.bold: true
                    color: "white"
                }

                // 拖动窗口支持
                MouseArea {
                    id: titleBarMouseArea
                    anchors.fill: parent
                    property point clickPos: Qt.point(0, 0)

                    onPressed: (mouse) => {
                        clickPos = Qt.point(mouse.x, mouse.y)
                    }

                    onPositionChanged: (mouse) => {
                        if (pressed) {
                            const delta = Qt.point(mouse.x - clickPos.x,
                                                   mouse.y - clickPos.y)
                            mainWindow.x += delta.x
                            mainWindow.y += delta.y
                        }
                    }
                }
            }

            // 最小化按钮
            ToolButton {
                icon.source: "qrc:/icons/ui/minimize.svg"
                icon.width: 20
                icon.height: 20
                onClicked: mainWindow.showMinimized()
                ToolTip.visible: hovered
                ToolTip.text: qsTr("最小化")
            }

            // 隐藏到托盘按钮
            ToolButton {
                icon.source: "qrc:/icons/ui/collapse.svg"
                icon.width: 20
                icon.height: 20
                onClicked: trayManager.hideMainWindow()
                ToolTip.visible: hovered
                ToolTip.text: qsTr("隐藏到系统托盘")
            }
        }
    }

    // 主内容区域
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 8
        spacing: 16

        // 统计卡片
        StatsCard {
            Layout.fillWidth: true
            Layout.preferredHeight: 100

            // 绑定TaskModel数据
            totalCount: taskModel.getTaskCount()
            completedCount: taskModel.getCompletedCount()
            pendingCount: taskModel.getPendingCount()
        }

        // 添加任务卡片
        AddTaskCard {
            Layout.fillWidth: true
            Layout.preferredHeight: 120

            onTaskSubmitted: (title, description) => {
                // 添加新任务
                taskModel.addTask(title, description)
            }
        }

        // 任务列表卡片
        TaskListCard {
            Layout.fillWidth: true
            Layout.fillHeight: true

            // 传递TaskModel给列表
            model: taskModel
        }
    }

    // 关闭事件处理：隐藏到托盘而不是退出
    onClosing: (close) => {
        close.accepted = false
        trayManager.hideMainWindow()
    }

    // 窗口圆角效果（可选）
    Rectangle {
        id: windowBackground
        anchors.fill: parent
        radius: 8
        color: Material.background
        z: -1  // 背景层

        layer.enabled: true
        layer.effect: DropShadow {
            transparentBorder: true
            horizontalOffset: 0
            verticalOffset: 4
            radius: 8
            samples: 17
            color: "#40000000"
        }
    }
}
```

**关键实现要点：**

| 功能 | 实现方式 |
|-----|---------|
| **无边框窗口** | `flags: Qt.Window \| Qt.FramelessWindowHint` |
| **拖动窗口** | 在标题栏MouseArea中计算鼠标位置变化并更新窗口坐标 |
| **圆角阴影** | 使用Rectangle + layer.effect DropShadow |
| **隐藏到托盘** | onClosing事件中调用`trayManager.hideMainWindow()` |
| **数据绑定** | 通过`taskModel`访问C++暴露的数据模型 |
| **国际化** | 使用`qsTr()`标记可翻译字符串 |
```

---

## 六、性能优化策略

### 6.1 ListView优化

**基础优化配置：**

```qml
ListView {
    id: taskListView
    model: taskModel

    // ✅ 性能优化关键配置
    cacheBuffer: 200        // 缓存屏幕外200像素的项目
    reuseItems: true        // 复用委托实例（Qt 5.15+）
    clip: true              // 裁剪可见区域外的内容

    // ✅ 使用required property减少model角色查找
    delegate: TaskItem {
        required property int index
        required property string title
        required property string description
        required property int priority
        required property int status

        width: taskListView.width

        // 不要使用 model.title，直接使用 property
        taskTitle: title
        taskDescription: description
        taskPriority: priority
        taskStatus: status
    }

    // ✅ 流畅的添加/删除动画
    add: Transition {
        NumberAnimation {
            property: "opacity"
            from: 0
            to: 1
            duration: 200
            easing.type: Easing.OutQuad
        }
    }

    remove: Transition {
        NumberAnimation {
            property: "opacity"
            from: 1
            to: 0
            duration: 150
            easing.type: Easing.InQuad
        }
    }

    displaced: Transition {
        NumberAnimation {
            properties: "x,y"
            duration: 200
            easing.type: Easing.OutQuad
        }
    }
}
```

---

### 6.2 图标加载优化

**关键：SVG图标必须设置sourceSize，避免全分辨率加载**

```qml
// ❌ 错误的做法：未设置sourceSize
Image {
    source: "qrc:/icons/actions/add.svg"
    // SVG会以原始分辨率加载，浪费内存
}

// ✅ 正确的做法：明确指定大小
Image {
    source: "qrc:/icons/actions/add.svg"
    sourceSize: Qt.size(24, 24)  // ← 必须设置！
    smooth: true
    antialiasing: true
}

// ✅ 更好的做法：封装为IconImage组件
IconImage {
    source: "qrc:/icons/actions/add.svg"
    width: 24
    height: 24
    color: Material.primary  // 支持着色
}
```

---

### 6.3 复杂组件延迟加载

**使用Loader按需加载复杂组件：**

```qml
// 任务详情对话框（复杂组件）
Loader {
    id: taskDetailLoader
    active: false  // 默认不加载
    sourceComponent: TaskDetailDialog {
        // 复杂的任务详情编辑界面
        onAccepted: {
            // 保存任务修改
            taskModel.updateTask(currentTaskId, taskData)
        }
        onClosed: {
            // 关闭后卸载组件
            taskDetailLoader.active = false
        }
    }
}

// 点击任务项时才加载
onTaskItemClicked: (taskId) => {
    currentTaskId = taskId
    taskDetailLoader.active = true  // ← 触发加载
}
```

---

### 6.4 避免性能陷阱

**1. 避免在ListView委托中使用FastBlur**

```qml
// ❌ 严重性能问题：FastBlur会导致列表卡顿
delegate: Rectangle {
    FastBlur {
        source: backgroundImage
        radius: 16  // 每个列表项都实时模糊，非常耗性能
    }
}

// ✅ 替代方案：使用layer.enabled优化
delegate: Rectangle {
    layer.enabled: true  // 启用缓存层
    layer.effect: FastBlur {
        radius: 8  // 模糊半径降低
        cached: true  // 缓存模糊结果
    }
}

// ✅ 最佳方案：避免在列表项中使用模糊效果
delegate: Rectangle {
    // 使用纯色或渐变替代模糊背景
    gradient: Gradient {
        GradientStop { position: 0.0; color: "#f5f5f5" }
        GradientStop { position: 1.0; color: "#e0e0e0" }
    }
}
```

**2. 避免不必要的属性绑定**

```qml
// ❌ 每次taskModel变化都会重新计算
Text {
    text: taskModel.getTaskCount() + " 个任务"
    // 即使count没变，文本也会重新绑定
}

// ✅ 使用信号连接，仅在实际变化时更新
Text {
    id: taskCountLabel
    text: "0 个任务"

    Connections {
        target: taskModel
        function onTaskCountChanged() {
            taskCountLabel.text = taskModel.getTaskCount() + " 个任务"
        }
    }
}
```

**3. 优化动画性能**

```qml
// ❌ 使用JavaScript动画（低性能）
NumberAnimation {
    target: rect
    property: "x"
    to: 100
    // JavaScript驱动，可能掉帧
}

// ✅ 使用属性动画（GPU加速）
Behavior on x {
    NumberAnimation {
        duration: 200
        easing.type: Easing.OutQuad
    }
}

// ✅ 对于复杂动画，启用layer
Rectangle {
    id: animatedRect
    layer.enabled: true  // 动画期间启用硬件加速

    Behavior on x {
        NumberAnimation { duration: 200 }
    }
}
```

---

### 6.5 线程安全策略

**数据库操作必须在主线程执行：**

```cpp
// models/taskmodel.h
class TaskModel : public QAbstractListModel
{
    Q_OBJECT

public:
    // QML调用的方法，使用Q_INVOKABLE
    Q_INVOKABLE void addTaskAsync(const QString& title,
                                   const QString& description);

signals:
    void taskAdded(bool success);

private slots:
    void addTaskInMainThread(const QString& title,
                            const QString& description);

private:
    DatabaseManager* m_database;
};
```

```cpp
// models/taskmodel.cpp
void TaskModel::addTaskAsync(const QString& title,
                             const QString& description)
{
    // 确保在主线程执行数据库操作
    QMetaObject::invokeMethod(this, "addTaskInMainThread",
                             Qt::QueuedConnection,
                             Q_ARG(QString, title),
                             Q_ARG(QString, description));
}

void TaskModel::addTaskInMainThread(const QString& title,
                                    const QString& description)
{
    // 此方法始终在主线程执行
    Task task(title, description);
    bool success = m_database->insertTask(task);

    if (success) {
        beginInsertRows(QModelIndex(), m_tasks.size(), m_tasks.size());
        m_tasks.append(task);
        endInsertRows();
        emit taskCountChanged();
    }

    emit taskAdded(success);
}
```

**在QML中调用：**

```qml
Button {
    text: "添加任务"
    onClicked: {
        // 异步调用，不会阻塞UI
        taskModel.addTaskAsync(titleInput.text, descInput.text)
    }
}

Connections {
    target: taskModel
    function onTaskAdded(success) {
        if (success) {
            showNotification("任务添加成功")
        }
    }
}
```

---

### 6.6 性能目标

| 指标 | 目标值 | 验收方法 |
|-----|--------|---------|
| 启动时间 | < 2秒 | 从main()到窗口完全显示 |
| 内存占用 | < 80MB | Windows任务管理器查看工作集 |
| 列表滚动帧率 | 60 FPS | QML Profiler监控帧率 |
| CPU占用（空闲） | < 5% | 任务管理器查看CPU使用率 |
| 1000任务加载 | < 1秒 | 数据库加载到ListView显示 |
| 动画流畅度 | 无掉帧 | 目视检查动画是否卡顿 |

**性能测试工具：**
- **QML Profiler** - Qt Creator内置，分析QML性能瓶颈
- **Windows Performance Analyzer** - 系统级性能分析
- **Valgrind (Linux)** - 内存泄漏检测

---

## 七、风险管理

| 风险 | 等级 | 缓解措施 |
|-----|------|---------|
| QML学习曲线 | 中 | 参考Qt文档，先实现简单组件 |
| 性能问题 | 低 | ListView优化，QML Profiler监控 |
| 系统集成问题 | 中 | 保留C++逻辑，仅适配QML接口 |

### 7.1 回滚策略

```bash
# 创建备份分支
git checkout -b widgets-backup
git push origin widgets-backup

# 在main分支重构
git checkout main
```

---

## 八、验收标准

### 8.1 功能验收
- [ ] 所有现有功能正常工作
- [ ] 数据持久化正确
- [ ] 系统托盘功能完整
- [ ] 主题切换无缝

### 8.2 性能验收
- [ ] 启动时间 < 2秒
- [ ] 内存占用 < 80MB
- [ ] 列表滚动60FPS
- [ ] 1000任务时流畅

### 8.3 视觉验收
- [ ] Material Design风格
- [ ] 动画流畅自然
- [ ] SVG图标专业
- [ ] 深色主题优秀

---

## 九、关键决策

### 9.1 为什么选择Qt Quick？

**优势：**
1. 声明式UI，开发效率更高
2. GPU加速，60FPS流畅
3. Material风格内置
4. 更好的可维护性
5. Qt官方推荐方向

### 9.2 为什么保持C++后端不变？

1. 稳定性 - 已充分测试
2. 风险控制 - 只重构UI层
3. 技术适配 - TaskModel无需修改
4. 开发效率 - 降低整体复杂度

---

## 十、参考资料

- [Qt Quick官方文档](https://doc.qt.io/qt-6/qtquick-index.html)
- [Qt Quick Controls 2](https://doc.qt.io/qt-6/qtquickcontrols-index.html)
- [QML与C++集成](https://doc.qt.io/qt-6/qtqml-cppintegration-overview.html)
- [Material风格指南](https://doc.qt.io/qt-6/qtquickcontrols2-material.html)
- [QML性能优化](https://doc.qt.io/qt-6/qtquick-performance.html)

---

## 十一、总结

### 技术方案评估

| 维度 | 评分 | 说明 |
|-----|------|-----|
| 可行性 | ⭐⭐⭐⭐⭐ | TaskModel天然支持QML |
| 风险 | ⭐⭐⭐ | 有学习曲线，但可控 |
| 收益 | ⭐⭐⭐⭐⭐ | 现代UI、流畅动画 |
| 工作量 | ⭐⭐⭐ | 15-20天，中等 |
| 可维护性 | ⭐⭐⭐⭐⭐ | QML代码清晰 |

### 最终建议

✅ **强烈推荐执行本重构方案**

**下一步行动：**
1. 团队评审本方案文档
2. 确认资源和时间安排
3. 创建`widgets-backup`分支备份
4. 开始执行任务1：环境配置

---

**文档编写人：** AI开发助手
**文档版本：** v2.0（完整技术方案）
**审核状态：** ✅ 已通过技术评审（2025-10-26）
**预期完成时间：** 2025-11-20
**最后更新时间：** 2025-10-26

**本次更新内容：**
- ✅ 修正CMake配置（保留Widgets模块）
- ✅ 修正main.cpp方案（使用QApplication而非QGuiApplication）
- ✅ 完善TrayManager的QML接口适配方案
- ✅ 补充TaskEnums枚举暴露实现
- ✅ 补充完整的main.cpp代码示例
- ✅ 补充Main.qml完整实现（无边框窗口、拖动、圆角阴影）
- ✅ 补充详细的性能优化策略（ListView、图标、动画、线程安全）
- ✅ 补充QML模块化最佳实践（qmldir、Theme单例）
- ✅ 重新组织实施计划（11个任务，16-20天）
- ✅ 添加关键里程碑和验收标准  

---

**附录：项目结构变更**

### 新增文件

```
k-memo/
├── qml/                                    # ← 新增QML目录
│   ├── Main.qml                           # ← 主窗口入口
│   ├── qmldir                             # ← QML模块定义
│   ├── components/
│   │   ├── cards/
│   │   │   ├── StatsCard.qml             # ← 统计卡片
│   │   │   ├── AddTaskCard.qml           # ← 添加任务卡片
│   │   │   └── TaskListCard.qml          # ← 任务列表卡片
│   │   └── items/
│   │       └── TaskItem.qml              # ← 任务列表项委托
│   ├── dialogs/
│   │   └── TaskDetailDialog.qml          # ← 任务详情对话框
│   └── styles/
│       ├── Theme.qml                     # ← 主题单例
│       └── qmldir                        # ← 样式模块定义
├── models/
│   └── taskenums.h                       # ← 新增枚举命名空间
└── main.cpp                              # ← 完全重写
```

### 修改文件

| 文件 | 修改内容 |
|-----|---------|
| **CMakeLists.txt** | 添加Qt Quick模块，配置qt_add_qml_module |
| **main.cpp** | 完全重写：使用QQmlApplicationEngine |
| **traymanager.h/.cpp** | 添加QML接口，修改构造函数 |
| **notificationmanager.h/.cpp** | 添加Q_INVOKABLE方法 |

### 废弃文件（待删除）

```
❌ k-memo/kmemo.h                    # Qt Widgets主窗口头文件
❌ k-memo/kmemo.cpp                  # Qt Widgets主窗口实现
❌ k-memo/kmemo.ui                   # Qt Designer UI文件
❌ k-memo/kmemo_old.cpp              # 旧版本主窗口
❌ k-memo/kmemo.ui.backup            # UI备份文件
❌ k-memo/TaskDetailWidget.h/.cpp   # Qt Widgets任务详情组件
❌ k-memo/TaskStatsWidget.h/.cpp    # Qt Widgets统计组件
❌ k-memo/QuickAddWidget.h/.cpp     # Qt Widgets快速添加组件
❌ k-memo/IconManager.h/.cpp        # 图标管理器（QML中直接使用qrc）
❌ k-memo/widgets/simple/           # 简化版Widget组件目录
   ├── simpleaddwidget.h/.cpp/.ui
   └── simpletasklistwidget.h/.cpp/.ui
```

**删除策略：**
1. ⚠️ 在Git中创建`widgets-backup`分支备份
2. ⚠️ 确保Qt Quick版本功能完整后再删除
3. ⚠️ 保留废弃文件至少2周，确保无回滚需求
4. ✅ 删除前将废弃文件移至`deprecated/`目录

---

**文档结束**

