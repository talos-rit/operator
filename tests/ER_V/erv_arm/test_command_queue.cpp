#include <CppUTest/TestHarness.h>
#include <unistd.h>
#include "ER_V/acl/acl.hpp"

bool poll_cmd_buffer(int fd, S_List* cmd_buffer);

TEST_GROUP(CommandQueue) {};

TEST(CommandQueue, DelayDoesNotConsumePromptAndNextCommandStillTransmits) {
  int descriptors[2];
  CHECK_EQUAL(0, pipe(descriptors));
  S_List queue;
  DATA_S_List_init(&queue);
  ACL_Command delay{};
  ACL_Command command{};
  const char payload[] = "HERE DELTA\r";
  command.len = sizeof(payload) - 1;
  memcpy(command.payload, payload, command.len);
  DATA_S_List_append(&queue, &delay.node);
  DATA_S_List_append(&queue, &command.node);

  CHECK_FALSE(poll_cmd_buffer(descriptors[1], &queue));
  CHECK_EQUAL(1, queue.len);
  CHECK_TRUE(poll_cmd_buffer(descriptors[1], &queue));
  CHECK_EQUAL(0, queue.len);
  char received[sizeof(payload)]{};
  CHECK_EQUAL(sizeof(payload) - 1, read(descriptors[0], received, sizeof(received)));
  STRCMP_EQUAL(payload, received);
  close(descriptors[0]);
  close(descriptors[1]);
}
