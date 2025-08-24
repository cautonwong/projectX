
#include <stddef.h>
#include <stdint.h>

#define DTO_DEFINITIONS(XX, XX_ARRAY)                                          \
  XX(uint32_t, import_active_power, "1.0.1.7.0.255", 4, 3,                     \
     "∑Li Instantaneous import active power (QI+QIV)")                         \
  XX_ARRAY(uint32_t, voltage, 3, "1.0.31.7.0.255", 4, 3,                       \
           "L1 Instantaneous current")                                         \
  XX(uint32_t, import_active_power, "1.0.1.7.0.255", 4, 3,                     \
     "∑Li Instantaneous import active power (QI+QIV)")                         \
  XX(uint32_t, export_active_power, "1.0.2.7.0.255", 4, 3,                     \
     "∑Li Instantaneous export active power (QII+QIII)")                       \
  XX(uint32_t, import_reactive_power, "1.0.3.7.0.255", 4, 3,                   \
     "∑Li Instantaneous import reactive power (QI+QIV)")                       \
  XX(uint32_t, export_reactive_power, "1.0.4.7.0.255", 4, 3,                   \
     "∑Li Instantaneous export reactive power (QII+QIII)")                     \
  XX(uint32_t, reactive_power_qi, "1.0.5.7.0.255", 4, 3,                       \
     "∑Li Instantaneous reactive power (QI)")                                  \
  XX(uint32_t, reactive_power_qiv, "1.0.6.7.0.255", 4, 3,                      \
     "∑Li Instantaneous reactive power (QIV)")                                 \
  XX(uint32_t, apparent_power, "1.0.9.7.0.255", 4, 3,                          \
     "∑Li Instantaneous import apparent power (QI+QIV)")                       \
  XX(uint32_t, export_apparent_power, "1.0.10.7.0.255", 4, 3,                  \
     "∑Li Instantaneous export apparent power (QII+QIII)")                     \
  XX(uint32_t, power_factor, "1.0.13.7.0.255", 4, 3,                           \
     "∑Li Instantaneous import power factor")                                  \
  XX(uint32_t, export_power_factor, "1.0.14.7.0.255", 4, 3,                    \
     "∑Li Instantaneous export power factor")                                  \
  XX(uint32_t, supply_frequency, "1.0.16.7.0.255", 4, 3,                       \
     "Instantaneous supply frequency")                                         \
  XX(uint32_t, l1_current, "1.0.31.7.0.255", 4, 3, "L1 Instantaneous current") \
  XX(uint32_t, l2_current, "1.0.51.7.0.255", 4, 3, "L2 Instantaneous current") \
  XX(uint32_t, l3_current, "1.0.71.7.0.255", 4, 3, "L3 Instantaneous current") \
  XX(uint32_t, neutral_current, "1.0.91.7.0.255", 4, 3,                        \
     "Neutral Instantaneous current")                                          \
  XX(uint32_t, zero_sequence_current, "1.0.101.7.0.255", 4, 3,                 \
     "Zero-sequence Instantaneous current")                                    \
  XX(uint32_t, l1_voltage, "1.0.32.7.0.255", 4, 3, "L1 Instantaneous voltage") \
  XX(uint32_t, l2_voltage, "1.0.52.7.0.255", 4, 3, "L2 Instantaneous voltage") \
  XX(uint32_t, l3_voltage, "1.0.72.7.0.255", 4, 3, "L3 Instantaneous voltage") \
  XX(uint32_t, zero_sequence_voltage, "1.0.102.7.0.255", 4, 3,                 \
     "Zero-sequence Instantaneous voltage")                                    \
  XX(uint32_t, l1_power_factor, "1.0.33.7.0.255", 4, 3,                        \
     "L1 Instantaneous import power factor")                                   \
  XX(uint32_t, l2_power_factor, "1.0.53.7.0.255", 4, 3,                        \
     "L2 Instantaneous import power factor")                                   \
  XX(uint32_t, l3_power_factor, "1.0.73.7.0.255", 4, 3,                        \
     "L3 Instantaneous import power factor")                                   \
  XX(uint32_t, internal_l1_voltage_max, "1.0.36.0.255", 4, 3,                  \
     "Internal L1 Voltage Maximum")                                            \
  XX(uint32_t, internal_l2_voltage_max, "1.0.56.0.255", 4, 3,                  \
     "Internal L2 Voltage Maximum")                                            \
  XX(uint32_t, internal_l3_voltage_max, "1.0.76.0.255", 4, 3,                  \
     "Internal L3 Voltage Maximum")                                            \
  XX(uint32_t, internal_l1_current_max, "1.0.37.0.255", 4, 3,                  \
     "Internal L1 Current Maximum")                                            \
  XX(uint32_t, internal_l2_current_max, "1.0.57.0.255", 4, 3,                  \
     "Internal L2 Current Maximum")


typedef struct domain_data_s domain_data_t;
struct domain_data_s {
#define XX(type, name, dto_name, dto_size, dto_scale, desc) type name;
#define XX_ARRAY(type, name, size, dto_name, dto_size, dto_scale, desc)        \
  type name[size];
  DTO_DEFINITIONS(XX, XX_ARRAY)
#undef XX
#undef XX_ARRAY
};

enum dto_type {
#define XX(type, name, desc, dto_name, dto_size, dto_scale) DTO_##name,
#define XX_ARRAY(type, name, size, desc, dto_name, dto_size, dto_scale)        \
  DTO_##name,
  DTO_DEFINITIONS(XX, XX_ARRAY)
#undef XX
#undef XX_ARRAY
      DTO_MAX,
};

#define DTO_IMPLEMENTATION
#ifdef DTO_IMPLEMENTATION

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uv.h>

uint32_t dto_sizes[DTO_MAX] = {
#define XX(type, name, dto_name, dto_size, dto_scale, desc) dto_size,
#define XX_ARRAY(type, name, size, dto_name, dto_size, dto_scale, desc)        \
  dto_size,
    DTO_DEFINITIONS(XX, XX_ARRAY)
#undef XX
#undef XX_ARRAY
};

size_t dto_size_getter(int dto_name) {
  if (dto_name >= DTO_MAX) {
    return 0;
  }
  // 根据 "1.0.1.7.0.255" 获取大小
  // 使用最小完美hash重新排序 快速通过dto_name找到dto_type
  // 从而快速找到
  return dto_sizes[dto_name];
}

void on_ev_timer(uv_timer_t *handle) {
  // 固定周期将domain_data推送到bus
}

void work_cb(uv_work_t *req) {
  // 启动从底板读取数据 发送请求
}

void after_work_cb(uv_work_t *req, int status) {
  // chain-style call
  {
    // 将底板回复的数据序列化到domain_data
    domain_data_t *data = (domain_data_t *)req->data;
    // 序列化
    // 结束后发起下一个请求
    uv_queue_work(uv_default_loop(), req, work_cb, after_work_cb);
  }
}

/*
iar 嵌入式mcu cortex-m
1. 将fd扩展成4字节
   bits(24-31) = applet/device/algos/functions id
   bits(0-23) =  applet/device internal data index
   校验和备份之类的操作 由各applet/device自行处理
   例如: 按照index来访问数据
   uint32_t fd = 0x010000020;// appelt id 1, internal data index 2
   hfp_read(fd,buf,&info); // 参数中不需要device了
   hfp_seek(fd, offset);
   按照块来访问数据
   uint32_t fd = 0x010000000; // applet id 1的第0块
   hfp_seed(fd, offset); // 定位 实现整存整取
2. 去掉表格
3. 等同对待设备和applet
4. 设备和applet增加一个preamble
5. preamble中描述自身的信息和依赖
   信息:
   - read/write/ctrl/init/seek/stat()  外部访问接口
   - run_entry()
   - version
   - code size
   - data size
   依赖:
   - 需要的eeprom space和flash space
   - 需要的RAM space
   要能够导出自己的fd,外界可以通过fd来访问内部的资源
6. 所有的device和applet的preamble都链接到一个地址 成为一个数组
   这个数组的索引序号刚好就是fd的高8位
   例如: fd = 0x010000020, 那么fd的高8位是0x01
   那么fd的preamble就是preamble[0x01]
   这个preamble的地址是一个全局变量
   例如: preamble[0x01] = &applet1_preamble;
   这一步通过iar的链接器和宏去完成  这个待解决的问题1

hfp_read函数的大概实现如下:
   1. 通过fd获取applet/device id和index
   2. 根据id获取applet/device的preamble
   3. 根据preamble获取applet/device的data

7. 以上步骤之后
   fd的表格可以不要 但是要支持导出 不然不知道哪个index代表什么数据 这是待解决的问题2
   避免了fd的搜索查找 此过程直接是数组索引得到
   各applet自己去编排自己的资源分布情况
   外部不管这个资源是在RAM还是EEPROM/flash里 或者是别的虚拟的通知之类的
   外部依旧可以做归一化的批量处理
   applet的资源为了更好地支持整存整取 建议按照存取方式去排布自己的资源
   例如:
   - 上电要清零的资源
   - 上电要从ee读到的RAM空间
   - 掉电要存到ee的数据区



*/

#endif
