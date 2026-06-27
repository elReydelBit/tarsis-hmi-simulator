#include <QApplication>
#include "mainwindow.h"
// spdlog core: gives us spdlog::info / warn / critical, etc.
#include <spdlog/spdlog.h>
// Needed specifically for file-based sinks (writing log lines to disk).
// Without this include, basic_file_sink_mt doesn't exist.
#include <spdlog/sinks/basic_file_sink.h>





// Sets up spdlog with two sinks: general.log (everything) and
// alarms.log (warning level and above only). Called once, at the
// very start of main(), before QApplication - it doesn't depend on
// the Qt event loop or any window, it's plain C++.
void setupLogging()
{
    // Two destinations (sinks) for log messages: one keeps everything,
    // the other will later be filtered to warnings/critical only.
    auto general_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/general.log");

    // Second sink: same mechanism as general_sink, but filtered
    // to only accept warning/critical messages - battery alarms and
    // the operator's RTL decision end up here, isolated from
    // routine telemetry logging.
    auto alarm_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/alarms.log");

    // Only warning level and above gets through to this sink -
    // routine telemetry (info, trace) never reaches alarms.log.
    alarm_sink->set_level(spdlog::level::warn);

    // The logger ties both sinks together. Every message sent to this
    // logger gets offered to both sinks - each sink then decides on its
    // own (via set_level) whether to actually write it.
    auto logger = std::make_shared<spdlog::logger>("tarsis", spdlog::sinks_init_list{general_sink, alarm_sink});

    // The logger itself must stay at the most permissive level (trace).
    // If we filtered here instead, messages would get blocked before
    // ever reaching the sinks.
    logger->set_level(spdlog::level::trace);

    // Format applied to every line, regardless of which sink writes it:
    // [date time] [level] message
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S] [%l] %v");

    // Registers this logger as THE default one - from now on, any file
    // in the project can call spdlog::warn(...) / spdlog::info(...) etc.
    // directly, without holding a reference to "logger" at all.
    spdlog::set_default_logger(logger);
}



int main(int argc, char *argv[]){

    //Prepare logging
    setupLogging();

    //Create the application object
    QApplication app(argc, argv);

    //Create the main window
    MainWindow window;

    //Show the window
    window.show();

    // Run the event loop first - this is where the whole program
    // actually lives. Only once it finishes do we flush and release
    // the logger, right before main() itself returns.
    int result = app.exec();
    spdlog::shutdown();
    return result;
}