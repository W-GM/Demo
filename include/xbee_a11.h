#ifndef __XBEE_A11__
#define __XBEE_A11__

// 低32位中，前4位指定油田分公司，后16位指定工程代码
#define PAN_ID 0xffffffffxxxxxxxx  // command ID

// 在发送0x11数据包时，规定profile id(19,20)为石油股票代码的后六位
#define PROFILE_ID_H 0x18
#define PROFILE_ID_L 0x57


#endif