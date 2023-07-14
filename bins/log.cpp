#include "log.hpp"

int main() {
    // a quick way to log
    LOGI("hello ", "world", " this is logi ", 123);

    // create your own logger and log
    auto& log = logger::LoggerMgr::Instance().GetDefault();
    LOGI("info");
    LOGI("trace");

    log.SetLevel(logger::Debug);
    // this will not output
    LOGI("info after set level to debug");
    // this will output
    LOGD("debug after set level to debug");
    LOGW("warning after set level to debug");

    return 0;
}