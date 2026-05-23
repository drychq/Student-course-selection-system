# Student-course-selection-system

## 简介

这是一个基于命令行的学生选课系统，使用 C++23 和 xmake 构建。

项目已拆分为 CLI 前端、业务服务、领域模型和文件持久化层。数据文件仍兼容旧版本：

- `students.txt`
- `teachers.txt`
- `courses.txt`
- `student_courses.txt`
- `scores.txt`

## 环境要求

- 支持 C++23 的 C++ 编译器
- xmake 3.0 或更高版本

## 构建与运行

```sh
xmake build
xmake run Student-course-selection-system
```

## 测试

```sh
xmake run student_course_selection_tests
```

## 跨平台安装

使用 xmake 的安装目录参数指定安装位置，不再依赖 Linux-only 的 `sudo`、`/usr/local` 或桌面入口脚本。

```sh
xmake install -o dist/install-check
```

Windows、Linux 和 macOS 都使用同一套 xmake 构建入口。安装后可在指定目录中找到可执行文件和资源文件。
