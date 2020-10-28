#pragma once

#define PACKET_SIZE		(256)

#define IR_PROTOCOL_MASK 0x07ff

#define RAW_CODES       0x0001		/* for internal use only */
#define RC5             0x0002		/* IR data follows RC5 protocol */
#define SHIFT_ENC		RC5			/* IR data is shift encoded (name obsolete) */
#define RC6             0x0004		/* IR data follows RC6 protocol */
#define RCMM            0x0008		/* IR data follows RC-MM protocol */
#define SPACE_ENC		0x0010		/* IR data is space encoded */
#define SPACE_FIRST     0x0020		/* bits are encoded as space+pulse */
#define GOLDSTAR        0x0040		/* encoding found on Goldstar remote */
#define GRUNDIG         0x0080		/* encoding found on Grundig remote */
#define BO              0x0100		/* encoding found on Bang & Olufsen remote */
#define SERIAL          0x0200		/* serial protocol */
#define XMP             0x0400		/* XMP protocol */

/* additinal flags: can be orred together with protocol flag */
#define REVERSE			0x0800
#define NO_HEAD_REP		0x1000		/* no header for key repeats */
#define NO_FOOT_REP		0x2000		/* no foot for key repeats */
#define CONST_LENGTH    0x4000		/* signal length+gap is always constant */
#define REPEAT_HEADER   0x8000		/* header is also sent before repeat code */

#define COMPAT_REVERSE  0x00010000	/* compatibility mode for REVERSE flag */

#define REPEAT_MAX_DEFAULT 600

#define IR_PARITY_NONE 0
#define IR_PARITY_EVEN 1
#define IR_PARITY_ODD  2
