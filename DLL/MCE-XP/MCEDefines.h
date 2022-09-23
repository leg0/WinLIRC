#pragma once

#define MCE_DEVICE_BUFFER_SIZE 100

#define MCE_BLASTER_1		1
#define MCE_BLASTER_2		2
#define MCE_BLASTER_BOTH	3

typedef struct {
    /// Last packet in block?
    int DataEnd;
    /// Number of bytes in block.
    int ByteCount;
    /// Carrier frequency of IR received.
    int CarrierFrequency;
} ReceiveParams, *ReceiveParamsPtr;

typedef struct {
    /// Device protocol version.
    int ProtocolVersion;
    /// Number of transmit ports ? 0-32.
    int TransmitPorts;
    /// Number of receive ports ? 0-32. For beanbag, this is two (one for learning, one for normal receiving).
    int ReceivePorts;
    /// Bitmask identifying which receivers are learning receivers ? low bit is the first receiver, second-low bit is the second receiver, etc ...
    int LearningMask;
    /// Device flags.
    int DetailsFlags;
} MCEDeviceCapabilities, *MCEDeviceCapabilitiesPtr;

typedef struct 
{
  /// Bitmask containing ports to transmit on.
  int TransmitPortMask;
  /// Carrier period.
  int CarrierPeriod;
  /// Transmit Flags.
  int Flags;
  /// Pulse Size.  If Pulse Mode Flag set.
  int PulseSize;
} TransmitParams;

typedef struct 
{
  /// Blaster bit-mask.
  int Blasters;
} AvailableBlasters;


typedef struct {
    /// Index of the receiver to use.
    int Receiver;
    /// Receive timeout, in milliseconds.
    int Timeout;
} StartReceiveParams, *StartReceiveParamsPtr;

typedef struct {

	int offsetToNextChunk;
	int repeatCount;
	int byteCount;

} TransmitChunk;

typedef enum
{
    /// Start receiving IR.
    IoCtrl_StartReceive  = 0x0F608028,
    /// Stop receiving IR.
    IoCtrl_StopReceive   = 0x0F60802C,
    /// Get IR device details.
    IoCtrl_GetDetails    = 0x0F604004,
    /// Get IR blasters
    IoCtrl_GetBlasters   = 0x0F604008,
    /// Receive IR.
    IoCtrl_Receive       = 0x0F604022,
    /// Transmit IR.
    IoCtrl_Transmit      = 0x0F608015,
    /// Reset IR.
    IoCtrl_Reset         = 0x0F608010,
}  IoCtrl;

static int FirstHighBit(int mask)
{
    for (int i = 0; i < 32; i++)
        if ((mask & (1 << i)) != 0)
            return i;

    return -1;
}
static int FirstLowBit(int mask)
{
    for (int i = 0; i < 32; i++)
        if ((mask & (1 << i)) == 0)
            return i;

    return -1;
}

static int GetHighBit(int mask, int bitCount)
{
	int count = 0;
	for (int i = 0; i < 32; i++)
	{
		int bitMask = 1 << i;

		if ((mask & bitMask) != 0)
			if (++count == bitCount)
				return bitMask;
	}

	return 0;
}
