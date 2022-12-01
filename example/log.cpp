#include "log.hpp"

int main() {
    // a quick way to log
    LOGI("hello ", "world", " this is logi ", 123);

    // create your own logger and log
    auto& log = logger::LoggerMgr::Instance().GetDefault();
    log.i("info");
    log.t("trace");

    log.SetLevel(logger::Debug);
    // this will not output
    log.i("info after set level to debug");
    // this will output
    log.d("debug after set level to debug");
    log.w("warning after set level to debug");

    return 0;
}