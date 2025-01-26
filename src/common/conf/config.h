/**
 * Generic Base class for parsing basic configuration files.
 * Intended to be very easy to extend into child configuration classes that automatically populate the key list,
 * and implement their own accessors/mutators
*/

#pragma once

#define CONF_DEFAULT_LOCATION "/etc/talos/configs/operator.conf"
#define CONF_MEMBER_LEN 128
#define CONF_KEY_LEN 64
#define CONF_VAL_LEN 64
#define CONF_PAIR_LIMIT 16

#define CONF_FILE_PERM O_RDONLY
#define CONF_ENTRY_FMT "^([^:]*): (.*)$"
#define CONF_REGEX_FLAGS (REG_NEWLINE | REG_EXTENDED)
#define CONF_DEFAULT_BOOL_VAL false

typedef enum _conf_data_type
{
    CONF_DATA_STRING,
    CONF_DATA_BOOL,
    CONF_DATA_INT,
} CONF_Data_Type;

typedef struct _conf_entry
{
    char key[CONF_KEY_LEN];                 /** Holds the key of the config entry*/
    char val[CONF_VAL_LEN];                 /** Holds the value of the config entry*/
    CONF_Data_Type type;                    /** Holds the type of the config entry*/
} CONF_Entry;

class Config
{
    private:
        int         conf_errno;              /** Stores any errors that happen during initialization */
        char        path[CONF_MEMBER_LEN];   /** File Path */
        CONF_Entry  pairs[CONF_PAIR_LIMIT];  /** Stored pairs */
        uint8_t     key_count;               /** Length of key-value table*/

        /**
         * @brief Parses file according to YAML standards (more or less)
         * @param fd File descriptor of config file
         * @returns 0 on success, -1 on failure
        */
        int ParseYaml(int fd);

        /**
         * @brief Linearly searches through keys table and returns the index of the key
         * @details Each entry is offset CONF_KEY_LEN from one another
         * @returns index on success, -1 on failure
        */
        int GetKeyIndex(const char* key);

        /**
         * @brief Helper function or handling an error during GetBool
         * @param idx Index of config entry
         * @returns Default bool value
        */
        bool fail_get_bool(int idx);

    protected:
        /**
         * @brief Overrides a value in the key-value table at the given index
         * @param key_idx index of Key-Value pair to override
         * @param val Value to override pair with
         * @returns 0 on success, -1 on failure
        */
        int OverrideValue(uint8_t key_idx, const char* val);

        /**
         * @brief Overrides a value in the key-value table at the given index
         * @param key_idx index of Key-Value pair to override
         * @param val Value to override pair with
         * @returns 0 on success, -1 on failure
        */
        int OverrideValue(uint8_t key_idx, bool val);

    public:
        Config();
        virtual ~Config();

        /**
         * @brief Returns the path of the active config file
         * @returns const pointer to the path member on success, NULL on failure
        */
        const char* GetFilePath();

        /**
         * @brief Sets the path of the active config file
         * @param path Path to set
         * @returns 0 on success, -1 on failure
        */
        int SetFilePath(const char* file_path);

        /**
         * @brief Parses the selected config file
         * @details opens the predesignated path, parses it, then closes; YAML is used as the format
         * @returns 0 on success, -1 on failure
        */
        int ParseConfig();

        /**
         * @brief Appends a key to the list of keys
         * @param key Key to add
         * @param deflt Default value of key; Will remain the value in the case of missing key/error
         * @param type Data type of the key's value
         * @returns -1 on failure, the index of the new key-value pair on success
        */
        int AddKey(const char* key, const char* deflt, CONF_Data_Type type);

        /**
         * @brief Appends a key to the list of keys
         * @details Assumes value is a string
         * @param key Key to add
         * @param deflt Default value of configuration; (Named to avoid keyword conflict)
         * @returns -1 on failure, key index on success
        */
        int AddKey(const char* key, const char* deflt);

        /**
         * @brief Appends a key to the list of keys
         * @details Assumes value is a boolean
         * @param key Key to add
         * @param deflt Default value of configuration; (Named to avoid keyword conflict)
         * @returns -1 on failure, key index on success
        */
        int AddKey(const char* key, bool deflt);

        /**
         * @brief Appends a key to the list of keys
         * @details Assumes value is a integer
         * @param key Key to add
         * @param deflt Default value of configuration; (Named to avoid keyword conflict)
         * @returns -1 on failure, key index on success
        */
        int AddKey(const char* key, int deflt);


        /**
         * @brief Returns a const char buffer containing the value associated with the given key index
         * @details Uses index to directly access string; Faster than linear search overload
         * @param idx Index of value's key
         * @returns const char pointer to string on success, NULL on failure
        */
        const char* GetVal(uint8_t idx);

        /**
         * @brief Copies the associated value of a given key to the destination character pointer
         * @details Searches through key list to find value
         * @param key Key associated with value
         * @returns const char pointer to string on success, NULL on failure
        */
        const char* GetVal(const char* key);

        /**
         * @brief Returns the value associated with the given index as a bool
         * @details Uses index to directly access value, rather than linear search
         * @param idx Index of value in key-value table
         * @param deflt Default value of configuration
         * @returns Boolean value of configuration
        */
        bool GetBool(int idx);

        /**
         * @brief Returns the value of the given key as a bool
         * @details Uses linear search to find and access the value, as opposed to direct access with an index
         * @param key Key associated with desired value
         * @param deflt Default value of configuration
         * @returns Boolean value of configuration
        */
        bool GetBool(const char* key);

        /**
         * @brief Returns the value associated with the given index as an integer
         * @details Uses index to directly access value, rather than linear search
         * @param idx Index of value in key-value table
         * @param deflt Default value of configuration
         * @returns Integer value of configuration
        */
        int GetInt(int idx);

        /**
         * @brief Returns the value of the given key as an integer
         * @details Uses linear search to find and access the value, as opposed to direct access with an index
         * @param key Key associated with desired value
         * @param deflt Default value of configuration
         * @returns Integer value of configuration
        */
        int GetInt(const char* key);

        /**
         * @brief Clears key-value table
        */
        void ClearKeyVals();

        /**
         * @brief Gives a detailed list of every added key and their value, and dumps it into the logging system
         * @param log_level Determines the log priority level to print at (e.g. LOG_INFO, LOG_VERBOSE + 2)
        */
        virtual void DumpToLog(int log_level);
};

