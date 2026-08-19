# Student-course-selection-system

## 简介

这是一个基于命令行的学生选课系统，使用 C++23 named modules、`import std;`、SQLiteCpp、SQLite 和 xmake 构建。

项目分为四个模块：

- `scs.domain`：学生、教师和课程领域对象。
- `scs.persistence`：SQLite 连接、schema 和实时写入。
- `scs.service`：选课、成绩和统计业务。
- `scs.frontend`：命令行界面。

项目源码统一使用 `.cppm` 扩展名。`main.cppm` 和测试入口是导入模块的全局翻译单元，因为 C++ 入口函数 `main()` 不能附着到 named module。

## 环境要求

- xmake 3.0 或更高版本
- 支持 C++23 标准库 named module `std` 的编译器和标准库
- Windows：MSVC 14.51 或更新版本
- Linux：GCC 15.2
- macOS：Homebrew GCC 15.2，不支持系统默认 AppleClang

SQLiteCpp 3.3.3 和 SQLite 3.53 由 xmake 自动获取并静态链接；SQLiteCpp 复用项目指定的 SQLite，不会链接第二份数据库引擎。

## 构建与运行

```sh
xmake build
xmake run Student-course-selection-system
```

默认数据库路径为 `data/student_course_selection.db`。可通过首个参数覆盖：

```sh
xmake run Student-course-selection-system -- custom/path/courses.db
```

应用启动时自动创建并严格校验当前 STRICT schema。数据库连接采用 5 秒 busy timeout、外键约束、DELETE journal 和独占锁；同一数据库同一时间只允许一个应用实例。业务变更会立即写入 SQLite，不需要手动保存或加载。

项目不维护 schema 版本或迁移脚本。检测到旧版、不完整或数据不符合当前约束的应用表时，会在事务中直接重建四张应用表并丢弃其中数据；无关的其他用户表不会被主动删除。

## 测试

```sh
xmake build student_course_selection_tests
xmake run student_course_selection_tests
```

CI 会在 Windows、Linux 和 macOS 上分别构建 debug、release 并运行测试。

## 安装

```sh
xmake install -o dist/install-check
```
