# libmsx

[![CMake on multiple platforms](https://github.com/cautonwong/libmsx/actions/workflows/cmake-multi-platform.yml/badge.svg)](https://github.com/cautonwong/libmsx/actions/workflows/cmake-multi-platform.yml)


微服务开发基础库
----

- 构建系统cmake
- 工作流github workflow
- 质量控制ASAN

--------

## 开始

1. 复制`CMakeUserPresets.json.template`并重命名为`CMakeUserPresets.json`
2. 修改自己的工具链位置
3. 编写代码
```
struct my_service{
    service_t *base;
    // my other fields;
};

int main()
{
    service_init(); // 解析配置文件 注册回调  添加http route
    service_run();
    service_destroy();
}

```


## 已有微服务

```
south: 南向设备的发现注册抄读和控制 调用南向设备通信协议 按照任务间隔采集设备数据,经IPC存入core-data
north: 北向主站 调用北向主站通信协议 IPC给core-data和core-command
core-data: 数据中心 注册中心
core-command: 从北往南的命令(点抄或者阻塞式的设备控制)
```
