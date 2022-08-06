#include <future>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "converters/DateTimeConverter.hpp"
#include "tools/EventsCollector.hpp"
#include "tools/EventsDumper.hpp"
#include "wrappers/Candle.hpp"
#include "wrappers/DXFeed.hpp"
#include "wrappers/Quote.hpp"

std::future<void> testQuoteSubscription(const std::string &address, const std::vector<std::string> &symbols,
                                        long timeout) {
    auto dumper = dxfcpp::EventsDumper{};

    return dumper.dumpEvents<dxfcpp::Quote>(address, symbols, timeout);
}

std::future<std::vector<dxfcpp::Candle>> testCandleSnapshot(const std::string &address, const std::string &candleSymbol,
                                                            long long fromTime, long long toTime, long timeout) {
    auto collector = dxfcpp::EventsCollector{};

    return collector.collectTimeSeriesSnapshot<dxfcpp::Candle>(address, candleSymbol, fromTime, toTime, timeout);
}

int main() {
    auto fromTimeString = "2022-07-27T00:00:00Z";
    auto toTimeString = "2022-07-28T00:00:00Z";
    auto fromTime = dxfcpp::DateTimeConverter::parseISO(fromTimeString);
    auto toTime = dxfcpp::DateTimeConverter::parseISO(toTimeString);

    auto result = testCandleSnapshot("demo.dxfeed.com:7300", "AAPL&Q{=1m}", fromTime, toTime, 20000).get();

    std::cout << "AAPL&Q{=1m} (" << fromTimeString << " - " << toTimeString << ") CANDLE SNAPSHOT RESULT:" << std::endl;

    for (auto const &e : result) {
        std::cout << e.toString() << std::endl;
    }

    std::cout << "AAPL, IBM QUOTES:" << std::endl;

    testQuoteSubscription("demo.dxfeed.com:7300", {"AAPL", "IBM"}, 10000).get();
}