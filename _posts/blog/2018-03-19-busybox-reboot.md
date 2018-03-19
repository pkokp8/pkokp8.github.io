---
layout:     post
title: reboot流程
description: 基于busybox的reboot流程
category: blog
---

busybox是一个常用的工具集。一般做一个嵌入式系统，文件系统上的构造，通常使用的是基于busybox的文件系统。(make menuconfig===Settings==='Installation Options')  
之前遇到了一个问题，系统起来后调用reboot，很简单的打印了reboot相关的一些字符串就不动了。什么错误信息都没有。设备挂死什么都做不了，只能硬件断电复位。  

reboot是一个系统调用。一下子不知道如何从内核中寻找到这个reboot的系统调用，于是就想从reboot这个命令入手。  
因为busybox的代码基本是一条命令一个文件，一开始就想找类似reboot.c的文件，然而失败了。  
只找到了reboot.h。但这是一个好的开端，因为既然叫reboot.h，那么里面定义的东西不是给reboot用的，就是给别的函数用的。进去一看，好多“魔数”。  
<pre>
#ifndef RB_HALT_SYSTEM
# if defined(__linux__)
#  define RB_HALT_SYSTEM  0xcdef0123
#  define RB_ENABLE_CAD   0x89abcdef
#  define RB_DISABLE_CAD  0
#  define RB_POWER_OFF    0x4321fedc
#  define RB_AUTOBOOT     0x01234567
# elif defined(RB_HALT)
#  define RB_HALT_SYSTEM  RB_HALT
# endif
#endif
</pre>

这已经很显然了，谁用到了这个魔数，谁就是reboot。至少也是reboot相关的命令。  
随便选一个开始寻找，例如第一个，RB_HALT_SYSTEM。找到了两个地方在使用这个魔数。一个是halt.c，一个是init.c。  
halt本身就和reboot一样是一条系统命令，那么很有可能reboot就是在init.c中。

首先看到了这个函数
<pre>
static void halt_reboot_pwoff(int sig)
{
	const char *m;
	unsigned rb;

	/* We may call run() and it unmasks signals,
	 * including the one masked inside this signal handler.
	 * Testcase which would start multiple reboot scripts:
	 *  while true; do reboot; done
	 * Preventing it:
	 */
	reset_sighandlers_and_unblock_sigs();

	run_shutdown_and_kill_processes();

	m = "halt";
	rb = RB_HALT_SYSTEM;
	if (sig == SIGTERM) {
		m = "reboot";
		rb = RB_AUTOBOOT;
	} else if (sig == SIGUSR2) {
		m = "poweroff";
		rb = RB_POWER_OFF;
	}
	message(L_CONSOLE, "Requesting system %s", m);
	pause_and_low_level_reboot(rb);
	/* not reached */
}
</pre>
note：出问题使用的busybox是1.20，而写博客用的版本是1.28。其中有很大的差异了。但原理上的话是类似的。

这段代码的run_shutdown_and_kill_processes函数中看到了一句“The system is going down NOW”的打印，联系到设备重启时会有类似的打印，也印证了reboot是位于这个地方。  
其实看到这里基本就没什么好看的了，因为最终依然是调用了系统调用：reboot。在busybox中看不到源码也没有后续流程。  

但是此处有一个提示，那就是那堆神奇的魔数。  
因为在busybox下的字符串不一定与内核下的字符串相同，基于非全词匹配的字符串的搜索太耗时间，而0xcdef0123之类的又有大小写的区分  
和busybox一样，先找找内核下是否有类似reboot.c或reboot.h的文件。  
果然有,include/linux/reboot.h:
<pre>
#define	LINUX_REBOOT_CMD_RESTART	0x01234567
#define	LINUX_REBOOT_CMD_HALT		0xCDEF0123
#define	LINUX_REBOOT_CMD_CAD_ON		0x89ABCDEF
#define	LINUX_REBOOT_CMD_CAD_OFF	0x00000000
#define	LINUX_REBOOT_CMD_POWER_OFF	0x4321FEDC
#define	LINUX_REBOOT_CMD_RESTART2	0xA1B2C3D4
#define	LINUX_REBOOT_CMD_SW_SUSPEND	0xD000FCE2
#define	LINUX_REBOOT_CMD_KEXEC		0x45584543
</pre>
这些字符串与busybox下的定义，除了大小写和宏名字外一模一样，那么很明显，reboot的系统调用使用了这些数字。  
随便选择一个，例如LINUX_REBOOT_CMD_RESTART，于是找到了位于sys.c中的一个switch/case  
<pre>
	switch (cmd) {
	case LINUX_REBOOT_CMD_RESTART:
		kernel_restart(NULL);
		break;
</pre>
运气比较好，不用继续找了，因为看函数名就知道。reboot如果不走到这个kernel_restart分支，我头拿下来下饭。  
<pre>
void kernel_restart(char *cmd)
{
	kernel_restart_prepare(cmd);
	disable_nonboot_cpus();
	if (!cmd)
		printk(KERN_EMERG "Restarting system.\n");
	else
		printk(KERN_EMERG "Restarting system with command '%s'.\n", cmd);
	kmsg_dump(KMSG_DUMP_RESTART);
	machine_restart(cmd);
}
</pre>
正好这个函数里有一句很明显的提示“Restarting system.\n”。  
这在我见过的正常reboot的设备里以及这款无法reboot的设备中都有打印。那么问题肯定处在这个之后。很有可能是machine_restart  
<pre>
void machine_restart(char *cmd)
{
	machine_shutdown();

	/* Flush the console to make sure all the relevant messages make it
	 * out to the console drivers */
	arm_machine_flush_console();

	arm_pm_restart(reboot_mode, cmd);

	/* Give a grace period for failure to restart of 1s */
	mdelay(1000);

	/* Whoops - the platform was unable to reboot. Tell the user! */
	printk("Reboot failed -- System halted\n");
	local_irq_disable();
	while (1);
}
</pre>
machine_shutdown做的内容就是禁止抢占。因为reboot是一个优先级非常高的指令。没人希望reboot执行了一半，基于时间片轮转的linux说，等一下，时间片到了，我先让这个进程跑一会儿。  
arm_machine_flush_console看名字猜功能系列。冲刷一下串口中的数据。让该打印的都打印出来。万一调用reboot之前系统发生了一些异常，打印还没遇到\n，那么在缓冲区中的数据很有可能就没了。这不利于调试。  
跳开arm_pm_restart(reboot_mode, cmd)。后面就是一句delay和printk。如果问题不出在arm_pm_restart(reboot_mode, cmd)，那么打印一定是应该有的。  
搜索非常顺利，这个函数指针在定义时就赋值了.  

## 这里说一下，并不是所有内核、所有平台、所有芯片都会这么写。具体需要分析。 ##

<pre>
/* arch/arm/kernel/process.c */
void (*arm_pm_restart)(char str, const char *cmd) = arm_machine_restart;
</pre>

于是找到  
<pre>
static void arm_machine_restart(char mode, const char *cmd)
{
	/*
	 * Tell the mm system that we are going to reboot -
	 * we may need it to insert some 1:1 mappings so that
	 * soft boot works.
	 */
	setup_mm_for_reboot();

#ifndef CONFIG_MP_PLATFORM_ARM
	/* Clean and invalidate caches */
	flush_cache_all();

	/* Turn off caching */
	cpu_proc_fin();

	/* Push out any further dirty data, and ensure cache is empty */
	flush_cache_all();
#endif
	/*
	 * Now call the architecture specific reboot code.
	 */
	arch_reset(mode, cmd);			//那么重点肯定是这里
}
</pre>

<pre>

static inline void arch_reset(char mode, const char *cmd)
{
    *(volatile unsigned long*)(REG_PM_MISC_BASE + 0xB8) = 0x79;    // reg_top_sw_rst   
}
</pre>
#define REG_PM_MISC_BASE     0xFD005C00  

可见这款产品就是往寄存器  
0xFD005C00 + 0xB8里写了一个0x79  
而内核空间是有一个mmu在工作的，因此这里的0xfd005C00是一个虚拟地址  
见IO_ADDRESS的定义  
<pre>
=========================================================
#define IO_SPACE_LIMIT 0xffffffff

/* Constants of AGATE RIU */
#define IO_VIRT         0xfd000000  // Define VM start address
#define IO_PHYS         0x1f000000  // Define bus start address
#define IO_OFFSET       ((IO_VIRT) - (IO_PHYS))
#ifndef CONFIG_OC_ETM
#define IO_SIZE         0x00400000
#else
#define IO_SIZE         0x00f20000
#endif

#define io_p2v(pa)      ((pa) + IO_OFFSET)
#define io_v2p(va)      ((va) - IO_OFFSET)
#define IO_ADDRESS(x)   io_p2v(x)
=========================================================
</pre>

各个平台的IO_ADDRESS意思大同小异。可以看到虚拟地址和物理地址的差值就是一个IO_OFFSET  
也就是0xfd000000-0x1f000000  那实际上重启的寄存器就是0x1F005CB8   
于是在没有mmu的uboot下执行了mw 0x1F005CB8 0x79，果然卡住了。   


从代码上分析来看，系统的执行逻辑没有问题，那么只能是这个寄存器有问题。  
咨询了芯片厂家，恩，寄存器错了。。。。  
然后把代码和芯片厂家提供的原始内核代码一比较，发现有修改痕迹。。。。我就不继续黑了。。。  



