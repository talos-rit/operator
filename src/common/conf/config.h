#pragma once

#define CONF_DEFAULT_LOCATION "/media/brooke/Vault/Dev/RIT/Fall2024/SWEN561/operator/src/common/conf/example.conf"
#define CONF_MEMBER_LEN 128
#define CONF_KEY_LEN 32
#define CONF_VAL_LEN 32
#define CONF_PAIR_LIMIT 16
#define CONF_FILE_PERM O_RDONLY

class Config
{
    private:
        char path[CONF_MEMBER_LEN];                     /** File Path */
        char keys[(CONF_VAL_LEN) * CONF_PAIR_LIMIT];    /** 2D Char Array of Keys; Index corresponds to vals array   */
        char vals[(CONF_KEY_LEN) * CONF_PAIR_LIMIT];    /** 2D Char Array of Values; Index corresponds to keys array */
        uint8_t key_count;                              /** Length of key/val table*/

        void ParseYaml(int fd);

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
         * @returns -1 on failure, key index on success
        */
        int AddKey(const char* key);


        /**
         * @brief Copies the associated value of a given key to the destination character pointer
         * @details Uses index to directly access string; Faster than linear search overload
         * @param dst Destination character pointer
         * @param idx Index of value
         * @returns 0 on success, -1 on failure
        */
        int GetVal(char* dst, uint8_t idx);

        /**
         * @brief Copies the associated value of a given key to the destination character pointer
         * @details Searches through key list to find value
         * @param dst Destination character pointer
         * @param key Key associated with value
         * @returns 0 on success, -1 on failure
        */
        int GetVal(char* dst, const char* key);

        /**
         * @brief Clears key-value table
        */
        void ClearKeyVals();
};

