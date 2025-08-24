#include <bits/types/struct_iovec.h>
#include <netdb.h>
#include <queue.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <uv.h>

// 采集档案配置单元（基于 6001）
typedef struct {
  unsigned long config_id;     // 配置序号
  char comm_addr[12];          // 通信地址 (TSA)
  unsigned char protocol_type; // 规约类型 (0=未知, 1=DL/T645-1997,
                               // 2=DL/T645-2007, 3=DL/T698.45)
  unsigned char
      wiring_mode; // 接线方式 (0=未知, 1=单相, 2=三相三线, 3=三相四线)
  unsigned long rated_voltage; // 额定电压 (单位: V)
  unsigned long rated_current; // 额定电流 (单位: A)
} AcquisitionConfig;

// 任务配置单元（基于 6013）
typedef struct {
  unsigned int task_id;      // 任务ID
  unsigned int exec_freq;    // 执行频率 (单位: 秒)
  char start_time[20];       // 开始时间 (date_time_s)
  char end_time[20];         // 结束时间
  unsigned char scheme_type; // 方案类型 (1=普通采集方案)
  unsigned int scheme_id;    // 方案编号
  char comm_addr[12];        // 采集通信地址 (TSA)
  struct uv__queue q;
  uv_async_t async;
  uv_mutex_t mutex;
  uv_cond_t cond;
  unsigned long last_exec_time; // 上次执行时间 (Unix 时间戳)
} TaskConfig;

// 采集任务监控单元（基于 6035）
typedef struct {
  unsigned int task_id; // 任务ID
  unsigned char status; // 任务状态 (0=未执行, 1=执行中, 2=已执行)
  char start_time[20];  // 任务执行开始时间
  char end_time[20];    // 任务执行结束时间
  unsigned long total_count;     // 采集总数量
  unsigned long success_count;   // 采集成功数量
  unsigned long first_exec_time; // 第一次执行时间 (Unix 时间戳)
  unsigned long exec_count;      // 已执行次数
} TaskMonitor;

#define ACQ_IMPLEMENTATION
#ifdef ACQ_IMPLEMENTATION

// 全局变量
uv_loop_t *loop;
uv_timer_t ntp_timer;

#define MAX_METERS 3
#define MAX_TASKS 2

AcquisitionConfig meters[MAX_METERS] = {
    {1, "123456789012", 2, 3, 220,
     5}, // 电能表1: DL/T645-2007, 三相四线, 220V, 5A
    {2, "987654321098", 1, 1, 220,
     10}, // 电能表2: DL/T645-1997, 单相, 220V, 10A
    {3, "456789123456", 3, 2, 380,
     15} // 电能表3: DL/T698.45, 三相三线, 380V, 15A
};

TaskConfig tasks[MAX_TASKS] = {
    {1, 10, "2025-04-06 10:00:00", "2025-04-06 12:00:00", 1, 1,
     "123456789012"}, // 任务1: 每10秒采集电能表1
    {2, 15, "2025-04-06 10:00:00", "2025-04-06 12:00:00", 1, 2,
     "987654321098"} // 任务2: 每15秒采集电能表2
};

TaskMonitor task_monitors[MAX_TASKS];

// 获取当前时间字符串
void get_current_time(char *buffer, size_t size) {
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  snprintf(buffer, size, "%04d-%02d-%02d %02d:%02d:%02d", t->tm_year + 1900,
           t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
}

// 模拟采集数据
unsigned long simulate_acquisition(const char *comm_addr) {
  srand(time(NULL) ^ (unsigned long)comm_addr);
  return (rand() % 10) + 90; // 模拟成功采集 90-100 条数据
}

// 存储采集数据到文件
void store_acquisition_data(unsigned int task_id, const char *comm_addr,
                            unsigned long success_count) {
  FILE *fp = fopen("acquisition_data.log", "a");
  if (!fp) {
    fprintf(stderr, "Failed to open acquisition_data.log\n");
    return;
  }

  char time_str[20];
  get_current_time(time_str, sizeof(time_str));
  fprintf(fp, "%u,%s,%s,%lu\n", task_id, time_str, comm_addr, success_count);
  fclose(fp);
}

// 任务执行回调
void execute_task(uv_timer_t *handle) {
  TaskMonitor *monitor = (TaskMonitor *)handle->data;
  TaskConfig *task = NULL;

  // 查找对应的任务配置
  for (int i = 0; i < MAX_TASKS; i++) {
    if (tasks[i].task_id == monitor->task_id) {
      task = &tasks[i];
      break;
    }
  }

  if (!task) {
    printf("Task %u not found\n", monitor->task_id);
    uv_timer_stop(handle);
    return;
  }

  // 检查任务是否在有效时间范围内
  time_t now = time(NULL);
  struct tm start_tm, end_tm;
  sscanf(task->start_time, "%d-%d-%d %d:%d:%d", &start_tm.tm_year,
         &start_tm.tm_mon, &start_tm.tm_mday, &start_tm.tm_hour,
         &start_tm.tm_min, &start_tm.tm_sec);
  sscanf(task->end_time, "%d-%d-%d %d:%d:%d", &end_tm.tm_year, &end_tm.tm_mon,
         &end_tm.tm_mday, &end_tm.tm_hour, &end_tm.tm_min, &end_tm.tm_sec);
  start_tm.tm_year -= 1900;
  start_tm.tm_mon -= 1;
  end_tm.tm_year -= 1900;
  end_tm.tm_mon -= 1;
  time_t start_time = mktime(&start_tm);
  time_t end_time = mktime(&end_tm);

  if (now < start_time || now > end_time) {
    monitor->status = 2; // 已执行（超出时间范围）
    printf("Task %u stopped: Out of time range\n", monitor->task_id);
    uv_timer_stop(handle);
    return;
  }

  // 执行采集任务
  monitor->status = 1; // 执行中
  if (monitor->first_exec_time == 0) {
    monitor->first_exec_time = start_time; // 使用 start_time 作为基准
    get_current_time(monitor->start_time, sizeof(monitor->start_time));
  }

  monitor->exec_count++;
  monitor->total_count += 100; // 每次采集尝试 100 条
  unsigned long success = simulate_acquisition(task->comm_addr);
  monitor->success_count += success;

  printf("Task %u executed: CommAddr=%s, Total=%lu, Success=%lu\n",
         monitor->task_id, task->comm_addr, monitor->total_count,
         monitor->success_count);

  // 存储采集数据
  store_acquisition_data(monitor->task_id, task->comm_addr, success);

  // 检查是否到达结束时间
  if (now >= end_time) {
    monitor->status = 2; // 已执行
    get_current_time(monitor->end_time, sizeof(monitor->end_time));
    printf("Task %u completed: EndTime=%s\n", monitor->task_id,
           monitor->end_time);
    uv_timer_stop(handle);
    return;
  }

  // 计算下一次触发时间（基于 start_time 的绝对时间调度）
  unsigned long next_exec_time =
      monitor->first_exec_time + (monitor->exec_count * task->exec_freq);
  unsigned long now_ms = uv_now(loop);
  unsigned long next_ms = next_exec_time * 1000; // 转换为毫秒
  unsigned long delay_ms;

  if (next_ms <= now_ms) {
    // 如果已经落后，调整到下一个时间点
    monitor->exec_count =
        (now_ms / 1000 - monitor->first_exec_time) / task->exec_freq + 1;
    next_exec_time =
        monitor->first_exec_time + (monitor->exec_count * task->exec_freq);
    next_ms = next_exec_time * 1000;
  }

  delay_ms = (next_ms > now_ms) ? (next_ms - now_ms) : 0;

  // 检测时间调整（人为调整或 NTP 同步）
  static unsigned long last_now = 0;
  if (last_now != 0 && abs((long)(now_ms - last_now) - (long)delay_ms) > 1000) {
    printf("Time adjustment detected, rescheduling tasks\n");
    monitor->first_exec_time = now; // 重置基准时间
    monitor->exec_count = 0;
    next_exec_time = now + task->exec_freq;
    next_ms = next_exec_time * 1000;
    delay_ms = (next_ms > now_ms) ? (next_ms - now_ms) : 0;
  }
  last_now = now_ms;

  // 重新调度定时器
  uv_timer_stop(handle);
  uv_timer_start(handle, execute_task, delay_ms, 0);
}

int main() {
  loop = uv_default_loop();

  // 初始化采集档案
  printf("Acquisition Configs:\n");
  for (int i = 0; i < MAX_METERS; i++) {
    printf("Meter %lu: CommAddr=%s, Protocol=%u, Wiring=%u, Voltage=%luV, "
           "Current=%luA\n",
           meters[i].config_id, meters[i].comm_addr, meters[i].protocol_type,
           meters[i].wiring_mode, meters[i].rated_voltage,
           meters[i].rated_current);
  }

  // 初始化任务和监控
  uv_timer_t task_timers[MAX_TASKS];
  for (int i = 0; i < MAX_TASKS; i++) {
    // 初始化任务监控
    task_monitors[i].task_id = tasks[i].task_id;
    task_monitors[i].status = 0; // 未执行
    strcpy(task_monitors[i].start_time, "");
    strcpy(task_monitors[i].end_time, "");
    task_monitors[i].total_count = 0;
    task_monitors[i].success_count = 0;
    task_monitors[i].first_exec_time = 0;
    task_monitors[i].exec_count = 0;

    // 打印任务配置
    printf("Task %u: Freq=%u, StartTime=%s, EndTime=%s, SchemeType=%u, "
           "SchemeID=%u, CommAddr=%s\n",
           tasks[i].task_id, tasks[i].exec_freq, tasks[i].start_time,
           tasks[i].end_time, tasks[i].scheme_type, tasks[i].scheme_id,
           tasks[i].comm_addr);

    // 计算初始延迟（基于 start_time 的绝对时间调度）
    struct tm start_tm;
    sscanf(tasks[i].start_time, "%d-%d-%d %d:%d:%d", &start_tm.tm_year,
           &start_tm.tm_mon, &start_tm.tm_mday, &start_tm.tm_hour,
           &start_tm.tm_min, &start_tm.tm_sec);
    start_tm.tm_year -= 1900;
    start_tm.tm_mon -= 1;
    time_t start_time = mktime(&start_tm);
    time_t now = time(NULL);
    unsigned long delay_ms = (start_time > now) ? (start_time - now) * 1000 : 0;

    // 启动定时任务
    uv_timer_init(loop, &task_timers[i]);
    task_timers[i].data = &task_monitors[i];
    uv_timer_start(&task_timers[i], execute_task, delay_ms,
                   0); // 初始触发基于 start_time
  }

  // 运行事件循环
  uv_run(loop, UV_RUN_DEFAULT);

  uv_loop_close(loop);
  return 0;
}

struct dto_t;
typedef int (*serializer_fn)(struct dto_t *dto, char *buf, size_t len);
typedef int (*deserializer_fn)(struct dto_t *dto, char *buf, size_t len);

#define SERIALIZER                                                             \
  serializer_fn serialize;                                                     \
  deserializer_fn deserialize;

struct dto_t {
  SERIALIZER
};

typedef struct TaskConfig_s TaskConfig;
typedef struct TaskMonitor_s TaskMonitor;
typedef struct AcquisitionConfig_s AcquisitionConfig;

struct transfer_object_s {
  SERIALIZER
  uint32_t offset;
};

int task_deserializer_fn(struct dto_t *dto, char *buf, size_t len) {
  struct transfer_object_s *obj = (struct transfer_object_s *)dto;
  if (len < sizeof(struct transfer_object_s)) {
    return -1;
  }

  struct uv__queue *q;
  struct TaskConfig tasks;
  struct transfer_object_s *obj;

  while (!uv__queue_empty(q)) {
    q = uv__queue_head(&tasks);
    uv__queue_remove(q);
    uv__queue_init(q);
    uv__queue_insert_tail(q, &tasks);
    if (obj->deserializer(obj, buf, len) == -1) {
      return -1;
    }

    buf += sizeof(struct dto_t);
    len -= sizeof(struct dto_t);
  }

  memcpy(obj, buf, sizeof(struct transfer_object_s));
  return 0;
}

struct task_s {
  TaskConfig *task;
  struct uv__queue q;
  TaskMonitor *monitor;
  uv_timer_t timer;// 任务的执行
  void (*start_fn)(struct task_s *task);
  void (*stop_fn)(struct task_s *task);
}

void async_cb(uv_async_t *handle) {
  // 收到task的add/delete/modify请求 在主循环中调用
  // 在此管理tasks节点 序列化和反序列化
  struct task_s *task;
  {
    // add
    struct task_s *task = malloc(sizeof(struct task_s));
    task->task = malloc(sizeof(TaskConfig));
    task->monitor = malloc(sizeof(TaskMonitor));
    task->start_fn = execute_task;
    task->stop_fn = execute_task;
    uv__queue_insert_tail(&task->q, &task->q);
  }
  {
    // modify
  }
  {
    // delete
    uv__queue_remove(&task->q);
    uv__queue_init(&task->q);
    free(task->task);
    free(task->monitor);
    free(task);
  }
}
#endif

typedef struct transport_s transport_t;

struct transport_s {
  uv_timer_t timeout;
  uv_timer_t timer;
  struct uv__queue req;
  struct uv__queue wait;
  uv_mutex_t mutex;
  uv_cond_t cond;
  uv_async_t async;
  struct uv__queue q;
  int fd;
  int id; // 端口号
};

void on_timer(uv_timer_t *handle) {
  printf("Timer triggered\n");
  // 将超时结果发往请求方
  uv_async_send();
  // 发起下一个请求
}

void on_ev_timer(uv_timer_t *handle) {
  printf("ev triggered\n");
  struct uv__queue *q;
  struct transport_s *port = (struct transport_s *)handle->data;
  struct uv__queue *req_q;

  if (uv__queue_empty(&port->wait)) {
    // 如果空闲则启动执行请求
    q = uv__queue_head(&port->req);
    uv__queue_remove(q);
    uv_serial_write();
    uv__queue_insert_tail(q, &port->wait);
  } else {
    uv_timer_start(&port->timer, on_ev_timer, 1000, 1000);
  }
}

void on_south_read() {
  if (!uv__queue_empty(&wq)) {
    // 将正确结果发往请求方
    uv_async_send();
    uv_timer_stop(&timer);
  }

  // 发起下一个请求
  if (!uv__queue_empty(&req_q)) {
    struct uv__queue *q;
    q = uv__queue_head(&req_q);
    uv__queue_remove(q);
    uv_serial_write();
    uv__queue_insert_tail(q, &wq);
    uv_timer_start(&timer, on_timer, 1000, 0);
  }
}

void on_south_write() { uv_timer_start(&timer, on_south_write, 1000, 0); }

void south_async_cb(uv_async_t *handle) {
  // 处理request的添加
  struct request_t *req;
  req = (transport_t *)handle->data;
  uv__queue_insert_tail(&port->req, &req->q);
}

void south_thread(void *arg) {
  transport_t *port = (transport_t *)arg;
  uv_loop_t *loop = malloc(sizeof(uv_loop_t));
  uv_loop_init(loop);

  uv_timer_init(loop, &port->timer);
  uv_timer_start(&port->timer, on_ev_timer, 1000, 1000);
  uv_handle_set_data(&port->timer, port);
  uv_async_init(loop, &port->async, south_async_cb);

  uv_timer_init(loop, &port->timeout);
  // uv_timer_start(&port->timeout, on_timer, 1000, 0);

  uv_run(loop, UV_RUN_DEFAULT);
}

struct device_s{
  struct uv__queue q;
  // 南向设备基类
};

struct request_t {
  uv_buf_t buf;
  uv_async_t async;
  struct uv__queue q;
};

typedef struct scheduler_s scheduler_t;
struct scheduler_s {
  uv_tty_t tty;     // 查看当前的任务状态
  uv_async_t async; // 异步通知
  uv_mutex_t mutex;
  uv_cond_t cond;
  uv_loop_t *loop;
  uv_timer_t timer;
  struct uv__queue tasks;   // 任务列表
  struct uv__queue ports;   // 端口列表
  struct uv__queue devices; // 档案列表
  // mempool req的内存从mempool中分配
};

scheduler_t *scheduler_init() {
  scheduler_t *scheduler = malloc(sizeof(scheduler_t));
  if (!scheduler) {
    fprintf(stderr, "Failed to allocate memory for scheduler\n");
    return NULL;
  }

  uv__queue_init(&scheduler->tasks);
  uv__queue_init(&scheduler->ports);
  uv__queue_init(&scheduler->devices);
  uv_mutex_init(&scheduler->mutex);
  uv_cond_init(&scheduler->cond);
  scheduler->loop = uv_default_loop();
  if (!scheduler->loop) {
    fprintf(stderr, "Failed to get default loop\n");
    free(scheduler);
    return NULL;
  }
  uv_async_init(scheduler->loop, &scheduler->async, async_cb);
  scheduler->async.data = scheduler;

  return scheduler;
}

void scheduler_free(scheduler_t *scheduler) {
  if (scheduler) {
    free(scheduler);
  }
}

void scheduler_add_task(scheduler_t *scheduler, TaskConfig *task) {
  // 添加任务到任务列表
  uv_mutex_lock(&scheduler->mutex);
  uv__queue_insert_tail(&scheduler->tasks, &task->q);
  uv_mutex_unlock(&scheduler->mutex);
  uv_async_send(&scheduler->async);
}

void queue_south_req(transport_t *port) {
  struct request_t *req = malloc(sizeof(struct request_t));
  req->port = port;
  req->id = port->id;
  uint8_t *buf = malloc(1024);
  req->buf = uv_buf_init(buf, 1024);
  req->buf.len = 1024;
  
  // 决定是normal还是list 可能也会切换ID 组合ID拆分成子ID
  buildup_req(req); // 构建协议请求

  // TODO 这里有问题  多次提交可能会漏
  port->async.data = req;
  uv_async_send(&port->async);
}

void scheduler_start() {
  transport_t *port;
  uv_thread_t *south;
  struct uv__queue *q;
  uv_loop_t *loop = uv_default_loop();
  scheduler_t *scheduler = scheduler_init();
  uv_loop_set_data(loop, scheduler);

  uv__queue_foreach(q, &scheduler->ports){
    port = uv__queue_data(q, transport_t, q);
    south = malloc(sizeof(uv_thread_t));
    uv_thread_create(south, south_thread, port);
  }

  uv_thread_t north;
  uv_thread_create(&north, north_thread, port);
  
  uv_run(loop, UV_RUN_DEFAULT);
}
