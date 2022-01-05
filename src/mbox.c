/*
    Copyright © 2021 Michal Schulz <michal.schulz@gmx.de>
    https://github.com/michalsc

    This Source Code Form is subject to the terms of the
    Mozilla Public License, v. 2.0. If a copy of the MPL was not distributed
    with this file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/


#include <exec/types.h>
#include <exec/execbase.h>
#include <exec/io.h>
#include <exec/errors.h>

#include <proto/exec.h>
#include <proto/expansion.h>
#include <proto/devicetree.h>

#include <libraries/configregs.h>
#include <libraries/configvars.h>

#include <stdint.h>

/* status register flags */

#define MBOX_TX_FULL (1UL << 31)
#define MBOX_RX_EMPTY (1UL << 30)
#define MBOX_CHANMASK 0xF

extern APTR MailBox;
UBYTE __req[256];

static inline uint32_t LE32(uint32_t x) { return __builtin_bswap32(x); }

static uint32_t mbox_recv(uint32_t channel)
{
	volatile uint32_t *mbox_read = (uint32_t*)(MailBox);
	volatile uint32_t *mbox_status = (uint32_t*)((uintptr_t)MailBox + 0x18);
	uint32_t response, status;

	do
	{
		do
		{
			status = LE32(*mbox_status);
			asm volatile("nop");
		}
		while (status & MBOX_RX_EMPTY);

		asm volatile("nop");
		response = LE32(*mbox_read);
		asm volatile("nop");
	}
	while ((response & MBOX_CHANMASK) != channel);

	return (response & ~MBOX_CHANMASK);
}

static void mbox_send(uint32_t channel, uint32_t data)
{
	volatile uint32_t *mbox_write = (uint32_t*)((uintptr_t)MailBox + 0x20);
	volatile uint32_t *mbox_status = (uint32_t*)((uintptr_t)MailBox + 0x18);
	uint32_t status;

	data &= ~MBOX_CHANMASK;
	data |= channel & MBOX_CHANMASK;

	do
	{
		status = LE32(*mbox_status);
		asm volatile("nop");
	}
	while (status & MBOX_TX_FULL);

	asm volatile("nop");
	*mbox_write = LE32(data);
}

ULONG get_core_temperature()
{
    struct ExecBase *SysBase = *(struct ExecBase **)4;

    ULONG *FBReq = (ULONG*)(((ULONG)__req + 31) & ~31);
    ULONG len = 8*4;

    FBReq[0] = LE32(4*8);
    FBReq[1] = 0;
    FBReq[2] = LE32(0x00030006);
    FBReq[3] = LE32(8);
    FBReq[4] = 0;
    FBReq[5] = 0;
    FBReq[6] = 0;
    FBReq[7] = 0;

    CacheClearE(FBReq, len, CACRF_ClearD);
    mbox_send(8, (ULONG)FBReq);
    mbox_recv(8);

    return LE32(FBReq[6]);
}

ULONG get_core_voltage()
{
    struct ExecBase *SysBase = *(struct ExecBase **)4;

    ULONG *FBReq = (ULONG*)(((ULONG)__req + 31) & ~31);
    ULONG len = 8*4;

    FBReq[0] = LE32(4*8);
    FBReq[1] = 0;
    FBReq[2] = LE32(0x00030003);
    FBReq[3] = LE32(8);
    FBReq[4] = 0;
    FBReq[5] = LE32(1);
    FBReq[6] = 0;
    FBReq[7] = 0;

    CacheClearE(FBReq, len, CACRF_ClearD);
    mbox_send(8, (ULONG)FBReq);
    mbox_recv(8);

    return LE32(FBReq[6]);
}
