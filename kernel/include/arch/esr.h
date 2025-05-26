#ifndef ESR_H
#define ESR_H

#include <stdint.h>
#include <stddef.h>


#define ESR_EC_SHIFT    26
#define ESR_EC_WIDTH    6

#define ESR_EC_MASK     (0x3F << ESR_EC_SHIFT)
#define ESR_EC_TYPE(x)  (((x) & ESR_EC_MASK) >> ESR_EC_SHIFT)

#define ESR_FSC		    (0x3F)
#define ESR_FSC_TYPE	(0x3C)
#define ESR_FSC_LEVEL	(0x03)
#define ESR_FSC_EXTABT	(0x10)
#define ESR_FSC_MTE		(0x11)
#define ESR_FSC_SERROR	(0x11)
#define ESR_FSC_ACCESS	(0x08)
#define ESR_FSC_FAULT	(0x04)
#define ESR_FSC_PERM	(0x0C)

#define ESR_FSC_ACCESS_LEVEL(l) (ESR_FSC_ACCESS + (l))
#define ESR_FSC_PERM_LEVEL(l)   (ESR_FSC_PERM + (l))

#define ESR_FSC_FAULT_NL    (0x2C)
#define ESR_FSC_FAULT_L     (((l) < 0 ? ESR_FSC_FAULT_NL : ESR_FSC_FAULT) + (l))

#define ESR_EC_UNKNOWN	    (0x00)
#define ESR_EC_WFx		    (0x01)
#define ESR_EC_CP15_32	    (0x03)
#define ESR_EC_CP15_64	    (0x04)
#define ESR_EC_CP14_MR	    (0x05)
#define ESR_EC_CP14_LS	    (0x06)
#define ESR_EC_FP_ASIMD	    (0x07)
#define ESR_EC_CP10_ID	    (0x08)
#define ESR_EC_PAC		    (0x09)
#define ESR_EC_CP14_64	    (0x0C)
#define ESR_EC_BTI		    (0x0D)
#define ESR_EC_ILL		    (0x0E)
#define ESR_EC_SVC32	    (0x11)
#define ESR_EC_HVC32	    (0x12)
#define ESR_EC_SMC32	    (0x13)
#define ESR_EC_SVC64	    (0x15)
#define ESR_EC_HVC64	    (0x16)
#define ESR_EC_SMC64	    (0x17)
#define ESR_EC_SYS64	    (0x18)
#define ESR_EC_SVE		    (0x19)
#define ESR_EC_ERET		    (0x1a)
#define ESR_EC_FPAC		    (0x1C)
#define ESR_EC_SME		    (0x1D)
#define ESR_EC_IMP_DEF	    (0x1f)
#define ESR_EC_IABT_LOW	    (0x20)
#define ESR_EC_IABT_CUR	    (0x21)
#define ESR_EC_PC_ALIGN	    (0x22)
#define ESR_EC_DABT_LOW	    (0x24)
#define ESR_EC_DABT_CUR	    (0x25)
#define ESR_EC_SP_ALIGN	    (0x26)
#define ESR_EC_MOPS		    (0x27)
#define ESR_EC_FP_EXC32	    (0x28)
#define ESR_EC_FP_EXC64	    (0x2C)
#define ESR_EC_SERROR	    (0x2F)
#define ESR_EC_BREAKPT_LOW	(0x30)
#define ESR_EC_BREAKPT_CUR	(0x31)
#define ESR_EC_SOFTSTP_LOW	(0x32)
#define ESR_EC_SOFTSTP_CUR	(0x33)
#define ESR_EC_WATCHPT_LOW	(0x34)
#define ESR_EC_WATCHPT_CUR	(0x35)
#define ESR_EC_BKPT32	    (0x38)
#define ESR_EC_VECTOR32	    (0x3A)
#define ESR_EC_BRK64	    (0x3C)
#define ESR_EC_MAX		    (0x3F)

#endif