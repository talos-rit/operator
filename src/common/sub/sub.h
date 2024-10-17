/**
 * Subscriber Module
 * Acts as the interface for the rest of the Operator system to send and receive messages.
 * This includes, logs, return messages, etc.
 * 
 * This interface is intended to be an agnostic interface to whatever messaging implementation is used to communicate with the director.
*/

#pragma once

/**
 * @class Subscriber Messenger Interface
 * @details Serves to separate the rest of the Talos Operator system from the messenger implementation (AMQ, HTTP, etc...)
*/
class SUB_Messenger 
{
private:
public:

    /**
     * @brief Constructor
    */
    SUB_Messenger() {}

    /**
     * @brief Destructor
    */
    virtual ~SUB_Messenger() {}

    /**
     * @brief Starts the Messenger service
     * @return 0 on success, -1 on failure
    */
    virtual int Start() = 0;

    /**
     * @brief Stops the Messenger service
     * @return 0 on success, -1 on failure
    */
    virtual int Stop() = 0;
};

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief List of implemented SUB_Messenger classes
*/
typedef enum _sub_msg_implementaion
{
    SUB_MSG_AMQ,
} SUB_Concrete;

/**
 * @brief List of modes the SUB_Messenger can be in
*/
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


/**
 * @brief Starts the Messenger service (C interface)
 * @return 0 on success, -1 on failure
*/
int SUB_start();


/**
 * @brief Stops the Messenger service (C interface)
 * @return 0 on success, -1 on failure
*/
int SUB_stop();

/**
 * @brief Destorys/Deinitializes the Messenger service
 * @return 0 on success, -1 on failure
*/
int SUB_destroy();

#ifdef __cplusplus
}
#endif