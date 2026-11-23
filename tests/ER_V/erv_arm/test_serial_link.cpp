#include <CppUTest/TestHarness.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <string>
#include "erv_arm/erv.hpp"
#include "erv_arm/serial_link.hpp"

TEST_GROUP(SerialLinkLogic) {};

TEST(SerialLinkLogic, CountsPollsEchoesJointLinesAndFramingErrors) {
  SerialLink link;
  CHECK(link.report(0) == nullptr);
  auto poll = [&](const char* echo) {
    link.onPollSent();
    if (echo) link.onLine(echo, strlen(echo));
  };
  poll("LISTPV POSITION");
  poll("LISTPV POSI");          // split by a framing error
  link.onLine("TION", 4);       // ...its second half isn't counted again
  poll("LISTPV POSITION10");    // run into the next line
  poll(nullptr);                // never arrived...
  poll("LISTPV POSITION");      // ...noticed when the next poll goes out
  link.onJointLine();
  link.onByte('\0', 100);
  link.onByte('X', 100);
  const char* report = link.report(SerialLink::kReportIntervalMs);
  CHECK(report != nullptr);
  STRCMP_CONTAINS("polls=5 damaged=3 joint=1 framing=1 recoveries=0", report);
  // A poll at the end of one window whose echo lands in the next is fine.
  link.onPollSent();
  STRCMP_CONTAINS("polls=1 damaged=0", link.report(2 * SerialLink::kReportIntervalMs));
  link.onLine("LISTPV POSITION", 15);
  STRCMP_CONTAINS("polls=0 damaged=0", link.report(3 * SerialLink::kReportIntervalMs));
}

TEST(SerialLinkLogic, OverduePromptAsksAgainAndAnyOutputRestartsTheClock) {
  SerialLink link;
  CHECK(SerialLink::Recovery::None == link.poll(true, 1000));
  CHECK(SerialLink::Recovery::None == link.poll(true, 3999));
  link.onByte('A', 3500);  // the controller is still talking
  CHECK(SerialLink::Recovery::None == link.poll(true, 6000));
  CHECK(SerialLink::Recovery::SendReturn == link.poll(true, 6500));
  // A CR didn't help: the next attempt toggles manual mode.
  CHECK(SerialLink::Recovery::SendTilde == link.poll(true, 9500));
  CHECK(SerialLink::Recovery::SendReturn == link.poll(true, 12500));
  CHECK(SerialLink::Recovery::SendTilde == link.poll(true, 15500));
  // After the quick attempts, retry slowly.
  CHECK(SerialLink::Recovery::None == link.poll(true, 18500));
  CHECK(SerialLink::Recovery::SendReturn == link.poll(true, 15500 + SerialLink::kSlowRetryMs));
  // A prompt resets everything; not waiting never recovers.
  link.onPrompt();
  CHECK(SerialLink::Recovery::None == link.poll(false, 100000));
  CHECK(SerialLink::Recovery::None == link.poll(true, 100001));
  CHECK(SerialLink::Recovery::SendReturn == link.poll(true, 103001));
}

TEST(SerialLinkLogic, HomeAndMovedGetALongAllowance) {
  SerialLink link;
  link.onCommandSent("HOME\r", 5);
  CHECK(SerialLink::Recovery::None == link.poll(true, 1));
  CHECK(SerialLink::Recovery::None == link.poll(true, 30000));
  CHECK(SerialLink::Recovery::SendReturn == link.poll(true, 1 + SerialLink::kLongCommandTimeoutMs));
  link.onPrompt();
  link.onCommandSent("MOVE DELTA\r", 11);
  CHECK(SerialLink::Recovery::None == link.poll(false, 70000));
  CHECK(SerialLink::Recovery::None == link.poll(true, 70001));
  CHECK(SerialLink::Recovery::SendReturn == link.poll(true, 73001));
}

TEST(SerialLinkLogic, TracksTheControllersModeAnnouncements) {
  SerialLink link;
  CHECK(link.mayTypeCommands(0));
  link.onTildeSent(1000);
  // Until the controller answers a '~', its mode is unknown.
  CHECK_FALSE(link.mayTypeCommands(1100));
  link.onLine("MANUAL MODE!", 12);
  CHECK_FALSE(link.mayTypeCommands(1150));
  CHECK(link.controllerManual(1150));
  link.onTildeSent(2000);
  CHECK_FALSE(link.controllerManual(2100));  // the exit announcement is on its way
  link.onLine("EXIT MANUAL MODE...", 19);
  CHECK(link.mayTypeCommands(2150));
  CHECK_FALSE(link.controllerManual(2150));
  // An announcement that never arrives stops blocking after the settle time.
  link.onTildeSent(3000);
  CHECK_FALSE(link.mayTypeCommands(3100));
  CHECK(link.mayTypeCommands(3000 + SerialLink::kModeSettleMs));
  // An ordinary jog exit doesn't hold up the next jog; a recovery '~' does
  // until the controller says which mode it is in.
  link.onTildeSent(5000);
  CHECK(link.mayStartJog(5010));
  link.onTildeSent(6000, true);
  CHECK_FALSE(link.mayStartJog(6010));
  link.onLine("EXIT MANUAL MODE...", 19);
  CHECK(link.mayStartJog(6020));
  link.onTildeSent(7000, true);
  link.onLine("MANUAL MODE!", 12);
  CHECK_FALSE(link.mayStartJog(7020));
  // "JOINT MODE." alone (its "MANUAL MODE!" line lost) still means manual.
  link.onLine("EXIT MANUAL MODE...", 19);
  CHECK_FALSE(link.controllerManual(7500));
  link.onLine(" JOINT MODE.              ", 26);
  CHECK(link.controllerManual(8000));
  // A recovery while it says it is in manual mode toggles out of it.
  CHECK(SerialLink::Recovery::None == link.poll(true, 8000));
  CHECK(SerialLink::Recovery::SendTilde == link.poll(true, 8000 + SerialLink::kPromptTimeoutMs));
}

// A PTY standing in for the ACL controller, modelled on Bingo's byte traces:
// it echoes typed text, prompts with '>' and announces manual mode.
namespace {
struct FakeController {
  explicit FakeController(int fd) : master(fd) {}
  int master;
  bool manual = false;
  int drop_prompts = 0;
  std::string line, heard;

  void say(const std::string& text) {
    std::string out;
    for (char c : text) {
      if (c == '>' && drop_prompts > 0) { drop_prompts--; continue; }
      out += c;
    }
    if (!out.empty()) CHECK_EQUAL(static_cast<ssize_t>(out.size()), write(master, out.data(), out.size()));
  }

  void hear(char c) {
    heard += c;
    if (c == '~') {
      manual = !manual;
      line.clear();
      say(manual ? "\r\nMANUAL MODE!\r\n>\r\n JOINT MODE.              \r"
                 : "\r\nEXIT MANUAL MODE...\r\n>");
      return;
    }
    if (manual) return;  // jog keys: no echo, no prompt
    if (c == '\003') { say("\r\n^C\r\n>"); line.clear(); return; }
    if (c != '\r') { line += c; say(std::string(1, c)); return; }
    if (line == "LISTPV POSITION")
      say("\r\n 1:0        2:-1       3:0        4:0        5:0       "
          "\r\n X: 1763    Y:0        Z: 5042    P:-624     R:0       \r\n>");
    else
      say("\r\n>");
    line.clear();
  }

  void serve() {
    char bytes[256];
    ssize_t n;
    while ((n = read(master, bytes, sizeof(bytes))) > 0)
      for (ssize_t i = 0; i < n; i++) hear(bytes[i]);
  }
};

size_t count(const std::string& text, const std::string& token) {
  size_t n = 0;
  for (size_t at = text.find(token); at != std::string::npos; at = text.find(token, at + 1)) n++;
  return n;
}
}  // namespace

TEST_GROUP(SerialLinkRecovery) {
  int master = -1;
  void setup() {
    master = posix_openpt(O_RDWR | O_NOCTTY | O_NONBLOCK);
    CHECK(master >= 0);
    CHECK_EQUAL(0, grantpt(master));
    CHECK_EQUAL(0, unlockpt(master));
  }
  void teardown() { close(master); }
};

TEST(SerialLinkRecovery, LostPromptNoLongerStopsTheArm) {
  FakeController controller(master);
  Scorbot robot(ptsname(master));
  auto pump = [&](int milliseconds) {
    controller.heard.clear();
    for (int t = 0; t < milliseconds; t += 10) {
      static_cast<Arm&>(robot).poll();
      controller.serve();
      usleep(10000);
    }
    return controller.heard;
  };
  CHECK(count(pump(1500), "LISTPV POSITION") > 0);  // home sequence done, polling

  // The next prompt is corrupted on the wire. Operator used to wait forever.
  controller.drop_prompts = 1;
  pump(400);
  CHECK_EQUAL(0, count(pump(1500), "LISTPV POSITION"));

  // Within the prompt timeout it asks for a new prompt and resumes polling.
  std::string recovered = pump(2500);
  CHECK(recovered.find('\r') != std::string::npos);
  CHECK(count(recovered, "LISTPV POSITION") > 0);
}

TEST(SerialLinkRecovery, UnexpectedManualModeIsLeftBeforeAnyTextIsSent) {
  FakeController controller(master);
  Scorbot robot(ptsname(master));
  auto pump = [&](int milliseconds) {
    controller.heard.clear();
    for (int t = 0; t < milliseconds; t += 10) {
      static_cast<Arm&>(robot).poll();
      controller.serve();
      usleep(10000);
    }
    return controller.heard;
  };
  pump(1500);
  // E.g. a recovery '~' that found the controller in direct mode. Its prompt
  // arrives with the announcement, but typing now would jog the arm.
  controller.manual = true;
  controller.say("\r\nMANUAL MODE!\r\n>\r\n JOINT MODE.              \r");
  std::string after = pump(800);
  size_t tilde = after.find('~');
  CHECK(tilde != std::string::npos);
  CHECK_EQUAL(std::string::npos, after.substr(0, tilde).find_first_of("LISTPVOSIN"));
  CHECK_FALSE(controller.manual);
  CHECK(count(pump(1000), "LISTPV POSITION") > 0);
}
