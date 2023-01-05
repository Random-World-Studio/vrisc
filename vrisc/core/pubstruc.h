/**
 * @file pubstruc.h
 * @author pointer-to-bios (pointer-to-bios@outlook.com)
 * @brief
 * @version 0.1
 * @date 2023-01-01
 *
 * @copyright Copyright (c) 2022 Random World Studio
 *
 */

#ifndef __pubstruc_h__
#define __pubstruc_h__

#include "../types.h"

struct options
{
  u64 mem_size;
  u8 core;
  char *bootloader;         // 启动代码文件
  char *extinsts;           // 扩展指令集路径
  u8 shield_internal_clock; // 是否屏蔽内部时钟
};

typedef struct core
{
  struct regs
  {
    u64 x[16];
    /* flg
    ^0-equal
    ^1-bigger
    ^2-smaller
    ^3-zero
    ^4-signal
    ^5-overflow
    ^6-interrupt enable
    ^7-paging enable
    ^8-privilege level(effective when paging enabled)
      0-kernel
      1-user
    ^9-higher
    ^10-lower
     */
    u64 flg;
    u64 ip;
    u64 usb, ust; // 用户态栈帧
    u64 ksb, kst; // 内核态栈帧
    u64 kpt, upt; // 内核态和用户态页表指针
    u64 ivt;      // 中断向量表
    u64 scp;      // 系统调用指针
  } regs;

  struct interrupt
  {
    u8 triggered;
    u8 int_id;
#define IR_DIV_BY_ZERO 0                // 除数为0
#define IR_NOT_EFFECTIVE_ADDRESS 1      // 无效地址
#define IR_DEVICES 2                    // 外部设备中断
#define IR_CLOCK 3                      // 时钟
#define IR_INSTRUCTION_NOT_RECOGNIZED 4 // 指令无法识别
#define IR_PERMISSION_DENIED 5          // 权限错误

    struct controller
    {
#define LOCAL_INTQUEUE_BUFFER_SIZE 4096
      u8 lock;
      u8 interrupt_queue[LOCAL_INTQUEUE_BUFFER_SIZE]; // 中断队列
      u8 head;
      u8 tail;
#define LOCAL_INTQUEUE_LEN(ctller, len)                           \
  if (ctller.tail < ctller.head)                                  \
  {                                                               \
    len = ctller.tail + LOCAL_INTQUEUE_BUFFER_SIZE - ctller.head; \
  }                                                               \
  else                                                            \
  {                                                               \
    len = ctller.tail - ctller.head;                              \
  }
    } controller;
  } interrupt;

  /*
  在vrisc_core中有一个ipbuff变量用于存储ip转换的物理地址，
  这个ipbuff在执行指令后随ip一同增加，当ipbuff进入下一个物理页
  或执行了跳转类的指令时ipbuff会被重新计算；
  此变量用于表示ipbuff是否需要刷新。
   */
  u8 ipbuff_need_flush;

  /* 上一个指令的长度 */
  u64 incr;

} _core;

/* vrisc设备文件结构 */
typedef struct vrisc_dev
{
  u32 intctl_shm_namestart;
  u32 ioctl_shm_namestart;
  u32 totlen; // 这是strs的总长度
  u8 strs[0];
} vrisc_dev;

/* 锁操作 */
#define u8_lock_unlock(lock) lock = 0;
#define u8_lock_lock(lock) \
  {                        \
    while (lock)           \
      usleep(50);          \
    lock = 1;              \
  }

#endif
