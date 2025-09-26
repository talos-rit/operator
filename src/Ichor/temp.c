typedef int HeartbeatMessageType;
typedef int uint16;
typedef int opaque;
#define HeartbeatMessage.payload_length 0

struct {
      HeartbeatMessageType type;
      uint16 payload_length;
      opaque payload[HeartbeatMessage.payload_length];
      opaque padding[padding_length];
   } HeartbeatMessage;