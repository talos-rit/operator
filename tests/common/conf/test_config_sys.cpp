#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"
#include "conf/config.hpp"

class DerivedConfig : public Config {
 public:
  using Config::OverrideValue;  // Expose protected method for testing
  ~DerivedConfig() = default;
};

// Test group for base Config class
TEST_GROUP(ConfigTest) {
  DerivedConfig* config;
  const char* test_config_path = "test_config.yaml";

  void setup() {
    config = new DerivedConfig();
    // Create a simple test config file
    FILE* f = fopen(test_config_path, "w");
    if (f) {
      fprintf(f, "test_key: test_value\n");
      fprintf(f, "int_key: 42\n");
      fprintf(f, "bool_key: true\n");
      fclose(f);
    }
  }

  void teardown() {
    delete config;
    remove(test_config_path);  // Clean up test file
    mock().clear();
  }
};

/* Test for initialization of a Config object*/
TEST(ConfigTest, Initialization) {
  CHECK_TRUE(config != NULL);

  // Config path should be empty initially
  CHECK_EQUAL(0, strlen(config->GetFilePath()));

  // Initially, no keys should be present, so retrieval should fail for any key
  // and type
  CHECK_TRUE(config->GetVal("nonexistent_str") ==
             NULL);  // Should handle missing keys
  CHECK_EQUAL(-1,
              config->GetInt("nonexistent_int"));  // Should handle missing keys

  // NOTE: There is some interesting behavior here because GetBool is supposed
  // to return a bool The current implementation returns -1 for missing keys,
  // which is not a valid bool value Therefore, the compiler seems to evaluate
  // it as true (non-zero)
  CHECK_EQUAL(
      -1, config->GetBool("nonexistent_bool"));  // Should handle missing keys
}

TEST(ConfigTest, AddAndRetrieveStringKey) {
  int key_idx = config->AddKey("test_string", "default_value");
  CHECK_EQUAL(0, key_idx);
  STRCMP_EQUAL("default_value", config->GetVal("test_string"));
}

TEST(ConfigTest, RetrieveStringKeyDoesNotExist) {
  const char* val = config->GetVal("nonexistent_key");
  CHECK_TRUE(val == NULL);
}

TEST(ConfigTest, FailAddStringKeyWhenMaxKeysReached) {
  // Fill up to CONF_PAIR_LIMIT
  for (uint8_t i = 0; i < CONF_PAIR_LIMIT; i++) {
    char key_name[20];
    snprintf(key_name, sizeof(key_name), "key_%d", i);
    int res = config->AddKey(key_name, "value");
    CHECK_EQUAL(i, res);
  }

  // Now adding one more should fail
  int result = config->AddKey("extra_key", "extra_value");
  CHECK_EQUAL(-1, result);
}

TEST(ConfigTest, AddAndRetrieveIntKey) {
  int key_idx = config->AddKey("test_int", 123);
  CHECK_EQUAL(0, key_idx);
  CHECK_EQUAL(123, config->GetInt("test_int"));
}

TEST(ConfigTest, RetrieveIntKeyDoesNotExist) {
  CHECK_EQUAL(-1, config->GetInt("nonexistent_int_key"));
}

TEST(ConfigTest, AddAndRetrieveBoolKey) {
  int key_idx = config->AddKey("test_bool", true);
  CHECK_EQUAL(0, key_idx);
  CHECK_TRUE(config->GetBool("test_bool"));

  key_idx = config->AddKey("test_bool_false", false);
  CHECK_FALSE(config->GetBool("test_bool_false"));
}

TEST(ConfigTest, RetrieveBoolKeyDoesNotExist) {
  // GetBool returns -1 for non-existent keys, which is not a valid bool value
  // This is a design choice that may need to be revisited
  CHECK_EQUAL(-1, config->GetBool("nonexistent_bool_key"));
}

TEST(ConfigTest, AddWhenKeyAlreadyExists) {
  // TODO: Update the code to handle duplicate keys appropriately
  config->AddKey("duplicate_key", "first_value");
  int result = config->AddKey("duplicate_key", "second_value");
  // Expect -1 since key already exists
  CHECK_EQUAL(-1, result);
}

/* TEST(ConfigTest, KeyIndexManagement)
{
    // Test adding multiple keys
    config->AddKey("key1", "value1");
    config->AddKey("key2", "value2");
    config->AddKey("key3", "value3");

    CHECK_EQUAL(0, config->GetKeyIndex("key1"));
    CHECK_EQUAL(1, config->GetKeyIndex("key2"));
    CHECK_EQUAL(2, config->GetKeyIndex("key3"));
    CHECK_EQUAL(-1, config->GetKeyIndex("nonexistent"));
}*/

TEST(ConfigTest, FilePathManagement) {
  // Test setting valid file path
  CHECK_EQUAL(0, config->SetFilePath(test_config_path));
  STRCMP_EQUAL(test_config_path, config->GetFilePath());

  // Test setting invalid file path
  CHECK_EQUAL(-1, config->SetFilePath("/nonexistent/path/config.yaml"));
}

TEST(ConfigTest, BoolValueParsing) {
  config->AddKey("true_key", "true");
  config->AddKey("false_key", "false");
  config->AddKey("invalid_key", "invalid");

  CHECK_TRUE(config->GetBool("true_key"));
  CHECK_FALSE(config->GetBool("false_key"));
  // Invalid bool values should return default (false)
  CHECK_FALSE(config->GetBool("invalid_key"));
}

/*
 * Test for clearing all key-value pairs.
 * This is essentially a black box test of sorts since we're
 * not directly testing the internal state.
 */
TEST(ConfigTest, ClearKeyVals) {
  // Add multiple key-value pairs
  config->AddKey("test_key", "test_value");
  config->AddKey("int_key", 100);
  config->AddKey("bool_key", true);
  // Check that the key-value pairs were added correctly
  STRCMP_EQUAL("test_value", config->GetVal("test_key"));
  CHECK_EQUAL(100, config->GetInt("int_key"));
  CHECK_TRUE(config->GetBool("bool_key"));

  config->ClearKeyVals();

  // Value retrieval should return NULL after clearing since key no longer
  // exists
  CHECK_TRUE(config->GetVal("test_key") == NULL);
  CHECK_EQUAL(-1, config->GetInt("int_key"));
  // GetBool returns -1 for non-existent keys, which is not a valid bool value
  // This is a design choice that may need to be revisited
  CHECK_EQUAL(
      -1, config->GetBool("bool_key"));  // Default false for non-existent key
}

TEST(ConfigTest, OverrideValues) {
  int key_idx = config->AddKey("test_key", "initial_value");
  CHECK_EQUAL(0, key_idx);

  // Override with string
  CHECK_EQUAL(0, config->OverrideValue(key_idx, "new_value"));
  STRCMP_EQUAL("new_value", config->GetVal(key_idx));

  // Override with bool
  CHECK_EQUAL(0, config->OverrideValue(key_idx, true));
  CHECK_TRUE(config->GetBool(key_idx));
}
