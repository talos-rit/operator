#include <condition_variable>
#include <atomic>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <mutex>

#include "erv_arm/erv.hpp"
#include "erv_conf/erv_conf.h"
#include "log/log.h"
#include "socket/socket.hpp"
#include "sub/sub.hpp"

#define LOG_FILE_THRESHOLD_THIS LOG_THRESHOLD_MAX
#define LOG_CONSOLE_THRESHOLD_THIS LOG_THRESHOLD_MAX

namespace {

std::atomic<int> quit_sig{0};
std::mutex quit_mutex;
std::condition_variable quit_cv;

void quit_handler(int signum) {
  quit_sig.store(signum);
  quit_cv.notify_one();
}

void register_signals() {
  struct sigaction sa{};
  sa.sa_handler = quit_handler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART; /* Restart functions if
                               interrupted by handler */

  for (int sig : {SIGINT, SIGQUIT, SIGABRT}) {
    if (sigaction(sig, &sa, nullptr) == -1) {
      perror("sigaction");
      std::exit(EXIT_FAILURE);
    }
  }
}

#if VALGRIND
static void dummy_msg(Subscriber *hermes) {
  SUB_Buffer *buf = hermes->DequeueBuffer(SUB_QUEUE_FREE);
  if (!buf) return;

  API_Data_Wrapper *msg = (API_Data_Wrapper *)&buf->body[0];

  msg->header.msg_id = htobe32(0x0);
  msg->header.reserved_1 = htobe16(0);
  msg->header.cmd_id = htobe16(API_CMD_HOME);
  msg->header.len = htobe16(sizeof(API_Data_Home));

  API_Data_Home *cmd = (API_Data_Home *)&msg->payload_head;

  cmd->delay_ms = 0;

  buf->len = sizeof(API_Data_Home) + sizeof(API_Data_Header) + 2;
  hermes->EnqueueBuffer(SUB_QUEUE_COMMAND, buf);
}
#endif

}  // namespace

int main(int argc, char *argv[]) {
  LOG_prep();
  register_signals();

  // Initalize program; Setup Logging
  ERVConfig conf;

  // Setup config priority

  const char *conf_loc[] = {(argc > 1 ? argv[1] : nullptr),
                            CONF_DEFAULT_LOCATION};

  for (const char *loc : conf_loc) {
    if (loc && !conf.SetFilePath(loc))
      break;  // If file is successfully set, break loop
  }

  conf.ParseConfig();

  LOG_init(conf.GetLogLocation());
  LOG_start();

  conf.DumpToLog(LOG_INFO);

  // Init Modules
  Subscriber hermes;
  auto inbox = std::make_unique<Socket>();
  auto bot = std::make_unique<Scorbot>(conf.GetScorbotDevicePath());

  inbox->registerSubscriber(&hermes);
  bot->registerSubscriber(&hermes);

  // Start
  hermes.start();
  bot->start();
  inbox->start();

  // Wait for quit signal
  {
    std::unique_lock<std::mutex> lock(quit_mutex);
    quit_cv.wait(lock, [] { return quit_sig.load() != 0; });
  }

  LOG_VERBOSE(0, "Quit signal: %d", quit_sig.load());
  LOG_INFO("Shutting down...");

  // Cleanup running processes
  inbox->stop();
  hermes.stop();
  bot->stop();

  // End demo
  LOG_INFO("End Program.");
  LOG_stop();
  LOG_destroy();
}
