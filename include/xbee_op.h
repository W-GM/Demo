/**
 * @file xbee_op.h
 * @author your name (you@domain.com)
 * @brief   xbee at / remote at / tx / rx 操作函数
 * @version 0.1
 * @date 2020-03-17
 *
 * @copyright Copyright (c) 2020
 *
 */

#ifndef __XBEE_OP_H__
#define __XBEE_OP_H__

#include "xbee.h"

#define CHANGE

int xbeeAtCmd(XBee   & xbee,
               uint8_t *atCmd,
               uint8_t *setVale,
               uint8_t  valeLength);
void xbeeRemoteAtCmd(XBee         & xbee,
                     XBeeAddress64& remoteAddr,
                     uint8_t       *atCmd,
                     uint8_t       *setVale,
                     uint8_t        valeLength);

void xbeeTx(XBee        & xbee,
            uint8_t      *payload,
            uint8_t       payloadlen,
            XBeeAddress64 addr64,
            uint8_t       frameID);

int xbeeRx(XBee   & xbee,
           uint8_t *data,
           int     *len,
           uint64_t     *slave_addr);

#endif // ifndef __XBEE_OP_H__
