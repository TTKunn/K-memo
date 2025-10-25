# K-memo 开发指南

本文档为AI开发助手（Claude）提供K-memo项目的完整开发指导。

---

## 项目概述

K-memo是一款基于Qt/C++开发的Windows桌面任务管理和便签应用。采用MVC架构模式，使用SQLite作为本地数据存储，支持完整的任务管理功能，包括分类、标签、优先级和系统通知等。

### 核心技术栈

- **编程语言：** C++17
- **UI框架：** Qt6 (支持Qt5向后兼容)
- **构建系统：** CMake 3.30.5
- **数据库：** SQLite 3.x (通过Qt::Sql模块)
- **架构模式：** MVC (Model-View-Controller)
- **目标平台：** Windows 11

### 当前开发状态

- **当前阶段：** 🔄 UI重构阶段 - 移动风格界面设计
- **整体进度：** 阶段1-4已完成（76.9%），阶段5进行中
- **开发方法：** RIPER-5工作流（研究→调查→计划→执行→审查）
- **技术方案：** 方案二（平衡方案）- SQLite + QSystemTrayIcon + MVC架构

**已完成功能：**
- ✅ 任务CRUD完整操作
- ✅ 分类和标签系统
- ✅ 优先级管理（低、普通、高、紧急）
- ✅ 状态跟踪（待办、进行中、已完成、已取消）
- ✅ 系统托盘集成
- ✅ Windows通知系统
- ✅ 快捷键支持
- ✅ 全面中文本地化

**已移除功能：**
- ❌ 搜索和过滤功能（用户确认移除）
- ❌ 主题定制功能（用户确认移除）

---

## 项目架构

### 核心组件

#### 应用程序入口
- **主入口：** `k-memo/main.cpp` - 初始化QApplication并显示主窗口
- **主窗口：** `k-memo/kmemo.h/.cpp/.ui` - 继承自QMainWindow的核心应用窗口
- **构建配置：** `k-memo/CMakeLists.txt` - CMake配置，支持Qt5/Qt6自动检测

#### 数据层
- **Task实体类：** `k-memo/models/task.h/.cpp` - 核心任务实体，包含优先级、状态、分类、标签
- **TaskModel数据模型：** `k-memo/models/taskmodel.h/.cpp` - QAbstractListModel，用于任务列表管理
- **DatabaseManager：** `k-memo/database/databasemanager.h/.cpp` - 单例模式的SQLite数据库接口

#### 系统集成层
- **TrayManager：** `k-memo/managers/traymanager.h/.cpp` - 系统托盘管理，显示任务计数
- **NotificationManager：** `k-memo/managers/notificationmanager.h/.cpp` - Windows通知系统集成

#### UI组件层
- **TaskDetailWidget：** `k-memo/TaskDetailWidget.h/.cpp` - 任务详情编辑组件
- **TaskStatsWidget：** `k-memo/TaskStatsWidget.h/.cpp` - 任务统计信息显示
- **QuickAddWidget：** `k-memo/QuickAddWidget.h/.cpp` - 快速添加任务组件
- **IconManager：** `k-memo/IconManager.h/.cpp` - 图标资源统一管理

#### 简化界面组件（开发中）
- **SimpleAddWidget：** `k-memo/widgets/simple/simpleaddwidget.h/.cpp/.ui` - 简化的添加卡片
- **SimpleTaskListWidget：** `k-memo/widgets/simple/simpletasklistwidget.h/.cpp/.ui` - 简化的任务列表

### 技术架构详情

#### 数据流架构
```
UI层 (kmemo.cpp/ui)
    ↕ (Qt信号/槽)
模型层 (TaskModel)
    ↕ (数据库信号)
数据库层 (DatabaseManager - 单例)
    ↕ (SQLite操作)
SQLite数据库 (本地存储)
```

#### 核心设计模式
- **单例模式：** DatabaseManager确保单一数据库连接
- **观察者模式：** Qt信号槽机制实现组件通信
- **模型-视图模式：** TaskModel为UI提供数据抽象
- **工厂模式：** Task创建时使用UUID生成器

#### 组件依赖关系
- **主窗口** → TaskModel → DatabaseManager → SQLite
- **TrayManager** → 主窗口（显示/隐藏控制）
- **NotificationManager** → TrayManager（通知显示）
- **TaskModel** ↔ DatabaseManager（双向数据同步）

---

## 构建命令

### Windows PowerShell构建方式

```powershell
# 进入项目目录
cd k-memo

# 创建构建目录（如不存在）
if (-not (Test-Path build)) { mkdir build }
cd build

# CMake配置（默认使用Visual Studio 2022）
cmake ..

# 编译应用程序（Debug配置）
cmake --build . --config Debug

# 运行应用程序
.\Debug\k-memo.exe
```

### 跨平台构建命令

```bash
# 指定构建类型配置
cmake -DCMAKE_BUILD_TYPE=Debug ..

# 编译指定配置
cmake --build . --config Debug
```

### 开发构建命令

```bash
# 清理构建
cmake --build . --target clean

# 安装到系统（可选）
cmake --build . --target install
```

### Qt Creator集成

在Qt Creator中打开`k-memo/CMakeLists.txt`，使用IDE的集成构建系统。Qt Creator会自动处理CMake配置并提供调试支持。

---

## 项目目录结构

```
E:\Code\project\K-memo\
├── .cursor\                    # Cursor IDE配置
│   └── rules\
│       └── rules.mdc          # 开发规范文件（重要！）
├── k-memo\                    # 主项目源码目录
│   ├── models\                # 数据模型层
│   │   ├── task.h/.cpp       # Task实体类
│   │   └── taskmodel.h/.cpp  # TaskModel数据模型
│   ├── database\              # 数据持久化层
│   │   └── databasemanager.h/.cpp  # 数据库管理器（单例）
│   ├── managers\              # 系统集成管理器
│   │   ├── traymanager.h/.cpp       # 系统托盘管理
│   │   └── notificationmanager.h/.cpp  # 通知管理
│   ├── widgets\               # 自定义UI组件
│   │   └── simple\           # 简化版组件（开发中）
│   │       ├── simpleaddwidget.h/.cpp/.ui
│   │       └── simpletasklistwidget.h/.cpp/.ui
│   ├── icons\                 # SVG图标资源
│   │   ├── actions\          # 操作图标（添加、删除、编辑等）
│   │   ├── categories\       # 分类图标（工作、学习、个人等）
│   │   ├── priority\         # 优先级图标（低、普通、高、紧急）
│   │   ├── status\           # 状态图标（待办、进行中等）
│   │   └── ui\               # 界面图标（折叠、主题等）
│   ├── data\                  # 用户数据目录（gitignore）
│   │   └── .gitkeep          # 占位文件
│   ├── test\                  # 测试文件目录（仅长期测试）
│   │   └── .gitkeep          # 占位文件
│   ├── build\                 # 构建输出目录（gitignore）
│   ├── IconManager.h/.cpp     # 图标管理器
│   ├── TaskDetailWidget.h/.cpp  # 任务详情组件
│   ├── TaskStatsWidget.h/.cpp   # 统计信息组件
│   ├── QuickAddWidget.h/.cpp    # 快速添加组件
│   ├── kmemo.h/.cpp/.ui       # 主窗口
│   ├── kmemo_old.cpp          # 旧版本主窗口（待处理）
│   ├── kmemo.ui.backup        # UI备份文件（待处理）
│   ├── main.cpp               # 程序入口
│   ├── CMakeLists.txt         # CMake构建配置
│   └── resources.qrc          # Qt资源文件
├── project_document\          # 项目文档目录
│   ├── [000]功能模块文档.md   # 功能模块总览（核心文档）
│   ├── [001]K-memo桌面便签应用开发计划.md  # 总体计划
│   ├── [002]K-memo方案二详细实施计划.md    # 详细实施计划
│   └── [003]数据库设计文档.md  # 数据库设计文档
├── CLAUDE.md                  # 本文档 - AI开发指南
├── README.md                  # 项目说明
├── LICENSE                    # 开源许可证
└── .gitignore                 # Git忽略规则

目录用途说明：
- models/     - 核心业务实体类和数据模型
- database/   - 数据库操作相关代码
- managers/   - 系统级管理器（托盘、通知等）
- widgets/    - 可复用的自定义UI组件
- icons/      - 所有SVG图标资源，按功能分类
- data/       - 运行时生成的用户数据（数据库文件等）
- test/       - 仅保留长期测试文件
```

---

## 核心依赖

应用程序需要Qt6（或Qt5）的以下模块：

```cmake
find_package(Qt6 REQUIRED COMPONENTS 
    Widgets    # UI框架
    Sql        # SQLite数据库集成
)

target_link_libraries(k-memo PRIVATE 
    Qt6::Widgets 
    Qt6::Sql
)
```

**其他依赖：**
- CMake 3.10+
- C++17编译器
- Visual Studio 2022（Windows推荐）

---

## 开发规范

### 重要规范文件

⚠️ **必读：** `.cursor/rules/rules.mdc` - 包含完整的开发规范，包括：
- RIPER-5阶段性工作流
- K-memo项目特定规范
- Qt/C++开发规范
- 数据库操作规范
- Git版本控制规范

### 核心开发原则

1. **遵循RIPER-5工作流：** 研究→调查→计划→执行→审查
2. **所有代码注释使用中文**
3. **Qt对象必须正确设置父子关系** - 避免内存泄漏
4. **信号槽使用新式语法** - 编译期检查
5. **数据库操作必须检查返回值** - 错误处理
6. **UI文本全部中文化** - 用户体验一致性

### 代码质量检查清单

完成功能后必须检查：
- [ ] 所有QObject派生对象是否正确设置父对象？
- [ ] 信号槽连接是否使用新式语法？
- [ ] 数据库操作返回值是否全部检查？
- [ ] UI操作是否全部在UI线程执行？
- [ ] 所有用户可见文本是否已中文化？
- [ ] 代码注释是否使用中文？

---

## 数据模型架构

### Task实体类

**支持的功能：**
- 优先级：低(1)、普通(2)、高(3)、紧急(4)
- 状态：待办(0)、进行中(1)、已完成(2)、已取消(3)
- 分类和多标签支持
- 提醒功能
- JSON序列化/反序列化
- UUID唯一标识

**核心枚举：**
```cpp
enum class TaskPriority { Low=1, Normal=2, High=3, Urgent=4 };
enum class TaskStatus { Pending=0, InProgress=1, Completed=2, Cancelled=3 };
```

### 数据库架构

**核心表结构：**
- **tasks表：** 任务主表，包含时间戳和元数据
- **task_tags表：** 多对多关系，任务和标签关联
- **app_config表：** 应用配置存储

**数据库特性：**
- 版本控制和迁移支持
- 外键约束和级联删除
- 索引优化查询性能
- 事务支持批量操作

详细设计请参考：`project_document/[003]数据库设计文档.md`

---

## 实现状态与下一步

### 已完成组件 ✅

**第一阶段：基础架构（已完成）**
- ✅ 项目结构：完整的MVC目录组织
- ✅ CMake配置：Qt6::Widgets + Qt6::Sql模块配置完成
- ✅ 核心类接口：所有主要类的接口定义完成
- ✅ 构建环境：在VSCode + Visual Studio 2022中成功编译运行

**第二阶段：核心功能（已完成）**
- ✅ Task实体类：完整实现
- ✅ DatabaseManager：完整的CRUD操作和数据库管理
- ✅ TaskModel：完整的QAbstractListModel实现
- ✅ 主界面UI：基础功能界面

**第三阶段：增强功能（已完成）**
- ✅ 分类和标签系统
- ✅ 优先级和状态管理
- ✅ 系统托盘集成
- ✅ 图标管理系统

**第四阶段：系统集成（已完成）**
- ✅ 通知系统
- ✅ 快捷键支持
- ✅ 全面中文本地化
- ✅ 自定义UI组件（TaskDetailWidget、TaskStatsWidget、QuickAddWidget）

### 进行中 🚧

**第五阶段：UI重构（当前阶段）**
- [ ] 任务5.1：主界面UI布局重构（移动风格）
- [ ] 任务5.2：SimpleAddWidget组件实现
- [ ] 任务5.3：SimpleTaskListWidget组件实现
- [ ] 任务5.4：复杂功能菜单集成

进度详见：`project_document/[002]K-memo方案二详细实施计划.md`

### 开发指南

1. **严格遵循RIPER-5工作流** - 每个任务都经过完整流程
2. **及时更新进度文档** - 完成任务后立即更新实施计划
3. **维护MVC架构分离** - 保持各层职责清晰
4. **使用Qt信号槽机制** - 实现组件间通信
5. **完成后更新功能模块文档** - `[000]功能模块文档.md`

---

## 自动生成文件

以下文件由Qt和CMake自动生成，**不应手动编辑：**

- `ui_*.h` - 由`.ui`文件生成
- `moc_*.cpp/h` - Qt元对象编译器输出
- `qrc_*.cpp` - Qt资源编译器输出
- `build/`目录下的所有文件

---

## 项目文档体系

### 核心文档（000-099）

- **[000]功能模块文档.md** - 所有功能模块总览和状态追踪
- **[001]K-memo桌面便签应用开发计划.md** - 项目总体计划
- **[002]K-memo方案二详细实施计划.md** - 详细实施计划和进度追踪
- **[003]数据库设计文档.md** - 数据库架构和表设计
- **[004-099]** - 预留给其他核心文档

### 阶段性文档（100-199）

用于记录各阶段的实施方案、技术方案、实现报告等

### 测试修复文档（200-299）

用于功能测试、bug修复方案、模块完善升级方案等

### 技术专题文档（300-399）

用于技术使用说明、教学文档、最佳实践等

---

## 开发环境

### 推荐配置

- **IDE：** VSCode（主要） + Qt Creator（辅助）
- **编译器：** Visual Studio 2022 (MSVC 19.44.35213.0)
- **构建系统：** CMake 3.30.5
- **Qt版本：** Qt6（自动检测，支持Qt5降级）
- **数据库：** SQLite（通过Qt6::Sql模块）

### 开发工具

- **Qt Designer** - UI界面设计
- **Git** - 版本控制
- **CMake** - 构建配置
- **Qt Creator** - 调试和性能分析

---

## 重要开发注意事项

### 内存管理

⚠️ **Qt对象生命周期：**
- 所有QObject派生类必须指定父对象或手动管理
- 优先使用Qt的父子对象机制自动管理内存
- 非Qt对象使用智能指针（std::unique_ptr、std::shared_ptr）

### 线程安全

⚠️ **UI线程规则：**
- 禁止在非UI线程直接操作UI元素
- 数据库连接不能跨线程使用
- 使用信号槽机制在线程间传递数据

### 数据库操作

⚠️ **强制规范：**
- 必须通过`DatabaseManager::instance()`访问数据库
- 所有数据库操作必须检查返回值
- 批量操作必须使用事务包装

### 中文本地化

⚠️ **必须遵守：**
- 所有用户可见的文本使用中文
- 所有代码注释使用中文
- 错误消息和提示信息使用中文

---

## 测试策略

1. **单元测试：** 每个核心模块完成后测试
2. **集成测试：** 测试组件间的交互
3. **功能测试：** 验证完整的用户功能流程
4. **性能测试：** 确保响应速度和内存占用合理
5. **UI测试：** 验证界面响应和用户体验

**测试环境：**
- Windows 11（主要目标平台）
- 真实用户数据场景模拟

---

## 常见问题与解决方案

### 编译问题

**问题：Include路径错误**
```cmake
# 解决方案：确保CMakeLists.txt中设置了包含目录
target_include_directories(k-memo PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
```

**问题：Qt模块缺失**
```cmake
# 解决方案：检查find_package和target_link_libraries
find_package(Qt6 REQUIRED COMPONENTS Widgets Sql)
target_link_libraries(k-memo PRIVATE Qt6::Widgets Qt6::Sql)
```

### 运行时问题

**问题：数据库连接失败**
- 检查`data/`目录是否存在
- 检查数据库文件路径配置
- 查看错误日志获取详细信息

**问题：UI元素不显示**
- 确认`.ui`文件已正确配置
- 检查是否调用了`ui->setupUi(this)`
- 验证父子关系是否正确设置

---

## Git工作流程

### Commit Message格式

```
[类型] 简要描述（不超过50字符）

详细说明（可选）
- 变更点1
- 变更点2

关联任务：[任务编号]
```

**类型标签：**
- `[功能]` - 新功能实现
- `[修复]` - Bug修复
- `[重构]` - 代码重构
- `[文档]` - 文档更新
- `[优化]` - 性能优化
- `[UI]` - UI界面调整

### 提交时机

必须提交的节点：
1. 完成一个完整的功能模块
2. 完成一个阶段的所有任务
3. 修复一个重要bug
4. 完成文档的重大更新
5. 每日工作结束前（如有未提交的有效变更）

---

## 性能指标

### 目标性能

- **启动时间：** < 3秒
- **任务列表加载：** < 1秒
- **内存占用：** < 100MB
- **CPU占用：** < 5%（空闲时）

### 优化建议

- 实现任务列表分页加载
- 为数据库查询添加适当索引
- 使用异步操作处理耗时任务
- 优化图标加载和缓存机制

---

## 后续扩展计划

### 第二阶段功能（待定）

1. **云同步功能**
   - 用户注册和登录
   - 任务数据云端同步
   - 多设备数据共享

2. **数据分析功能**
   - 任务完成率统计
   - 工作效率分析
   - 生产力报告

3. **高级功能**
   - 任务模板
   - 批量导入导出
   - 团队协作（可选）

---

## 相关链接

- **Qt官方文档：** https://doc.qt.io/
- **SQLite文档：** https://www.sqlite.org/docs.html
- **CMake文档：** https://cmake.org/documentation/

---

## 维护信息

**文档维护人：** AI开发助手  
**最后更新时间：** 2025-10-25  
**文档版本：** v2.0（中文版）  
**项目版本：** v1.0-alpha  
**下次审查时间：** 2025-11-25  

---

**注意：** 本文档会随项目进展持续更新，请定期查阅以获取最新信息。
