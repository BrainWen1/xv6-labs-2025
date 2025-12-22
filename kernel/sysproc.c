#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "vm.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  kexit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return kfork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return kwait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int t;
  int n;

  argint(0, &n);
  argint(1, &t);
  addr = myproc()->sz;

  if(t == SBRK_EAGER || n < 0) {
    if(growproc(n) < 0) {
      return -1;
    }
  } else {
    // Lazily allocate memory for this process: increase its memory
    // size but don't allocate memory. If the processes uses the
    // memory, vmfault() will allocate it.
    if(addr + n < addr)
      return -1;
    myproc()->sz += n;
  }
  return addr;
}

uint64
sys_pause(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  if(n < 0)
    n = 0;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kkill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

uint64
sys_interpose(void)
{
  // 1. 定义变量，用来接收用户态传过来的参数
  int mask;                // 接收第一个参数：沙箱掩码
  char path_buf[MAXPATH];  // 接收第二个参数：路径（内核临时缓冲区）
  struct proc *p;          // 指向当前进程的结构体

  // 2. 获取当前进程的结构体（myproc()是xv6提供的函数，返回当前运行的进程）
  p = myproc();
  if(p == 0) {  // 极端情况：当前无进程，直接返回错误
    return -1;
  }

  // 3. 读取第一个参数（mask，存在a0寄存器，对应第0个参数）
  // argint(参数序号, 保存到的变量地址)：
  argint(0, &mask);

  // 4. 读取第二个参数（path，存在a1寄存器，对应第1个参数）
  // argstr(参数序号, 内核缓冲区, 缓冲区长度)：安全复制用户态字符串到内核
  if(argstr(1, path_buf, MAXPATH) < 0) {
    return -1;  // 读取失败，返回-1
  }

  // 5. 把读取到的参数存到当前进程的结构体里（核心！）
  p->sandbox_mask = mask;  // 存掩码
  // safestrcpy：xv6提供的安全字符串复制函数，避免越界
  safestrcpy(p->sandbox_path, path_buf, MAXPATH);

  // 6. 成功执行，返回0（用户态会收到0，表示成功）
  return 0;
}
