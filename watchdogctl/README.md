## 关于看门狗控制驱动的更新日志

#### 2020/07/30

### [当前版本]

#### 应用层手动写入控制

1. 当前版本为通过应用层向内核驱动每隔小于100s(看门狗的自动复位间隔)的时间写0或写1,从而在内核驱动中生成一个电平跳变,使看门狗不会自动复位.

### [历史版本]

#### 内核调用终端定时器自动控制

### [修改]

1. 修改了设备树中关于看门狗的名称,从而与自定义的看门狗控制驱动名称作区分.
2. 修改了历史版本的看门狗驱动程序名称,为"watchdogctl_1.c"

### [新增]

1. 新增了当前版本的应用层控制程序,在test文件夹下

### [备注]

1. 当前并没有可作为测试用的开发板,故未做实际测试
2. 修改的设备树在/home/wgm/SVN-CQ-PROJECT/长庆油田井场RTU项目/项目资料/硬件资料/米尔核心版资料/米尔/04-Source/MYiR-iMX-Linux
