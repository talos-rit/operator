#pragma once

#include "sub/sub.h"

/**
 * @class Subscriber Messenger Interface
 * @details Serves to separate the rest of the Talos Operator system from the messenger implementation (AMQ, HTTP, etc...)
*/
class Inbox
{
protected:
    Subscriber* sub;
    void OnMessage();

public:
    /**
     * @brief Constructor
    */
    Inbox() {}

    /**
     * @brief Destructor
    */
    virtual ~Inbox() {}

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

    virtual int RegisterSubscriber(Subscriber* sub) = 0;
};