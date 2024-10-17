/**
 * Subscriber Module; Acts as the interface for the rest of the Operator system to send and receive messages.
 * This includes, logs, return messages, etc.
 * 
 * This interface is intended to be an agnostic interface to whatever messaging implementation is used to communicate with the director.
*/

#pragma once

class SUB_Messenger 
{
private:
public:
    SUB_Messenger() {}

    virtual ~SUB_Messenger() {}

    virtual int Start() = 0;
    virtual int Stop() = 0;
};

#ifdef __cplusplus
extern "C" {
#endif

typedef enum _sub_msg_implementaion
{
    SUB_MSG_AMQ,
} SUB_Concrete;

typedef enum _sub_mode
{
    SUB_MODE_DEAD,
    SUB_MODE_INIT,
    SUB_MODE_RUN,
} SUB_Mode;

/**
 * @brief Configures and initializes Subscriber
 * @param type Specifies the implmentation type to use
 * @param sub_config Pointer to the implementation-specific configuration
*/
int SUB_init(SUB_Concrete type, void *sub_config);

int SUB_start();

int SUB_stop();

int SUB_destroy();

#ifdef __cplusplus
}
#endif