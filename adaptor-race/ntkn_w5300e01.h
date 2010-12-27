/*
 *  ntkn_w5300e01.h
 *  
 *
 *  Created by Liang Qu on 12/22/10.
 *  Copyright 2010 NTKN. All rights reserved.
 *
 */

#ifndef __NTKN_W5300E01__
#define __NTKN_W5300E01__

#define GPIO	0x56000000
#define CON		0
#define DATA	1
#define UP		2
#define GPA_OFFSET	0
#define GPB_OFFSET	0x10/4
#define GPC_OFFSET	0x20/4
#define GPD_OFFSET	0x30/4
#define GPE_OFFSET	0x40/4
#define GPF_OFFSET	0x50/4
#define GPG_OFFSET	0x60/4
#define GPH_OFFSET	0x70/4

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef volatile uint8 vuint8;
typedef volatile uint16 vuint16;
typedef volatile uint32 vuint32;

#endif
