#include <future>
#include <iostream>
#include <string>
#include <vector>

#include "Candle.hpp"
#include "DateTimeConverter.hpp"
#include "EventsCollector.hpp"
#include "EventsDumper.hpp"
#include "Quote.hpp"

std::future<void> testQuoteSubscription(const std::string &address, const std::vector<std::string> &symbols,
                                        long timeout) {
    auto dumper = dxfcs::EventsDumper{};

    return dumper.dumpEvents<dxfcs::Quote>(address, symbols, timeout);
}

std::future<std::vector<dxfcs::Candle>> testCandleSnapshot(const std::string &address, const std::string &candleSymbol,
                                                           long long fromTime, long long toTime, long timeout) {
    auto collector = dxfcs::EventsCollector{};

    return collector.collectTimeSeriesSnapshot<dxfcs::Candle>(address, candleSymbol, fromTime, toTime, timeout);
}

int main() {
    auto fromTime = dxfcs::DateTimeConverter::parseISO("2022-07-27T00:00:00Z");
    auto toTime = dxfcs::DateTimeConverter::parseISO("2022-07-28T00:00:00Z");

    auto result = testCandleSnapshot("demo.dxfeed.com:7300", "AAPL&Q{=1m}", fromTime, toTime, 20000).get();

    std::cout << "AAPL&Q{=1m} CANDLE SNAPSHOT RESULT:" << std::endl;

    for (auto const &e : result) {
        std::cout << e.toString() << std::endl;
    }

    std::cout << "AAPL, IBM QUOTES:" << std::endl;

    testQuoteSubscription("demo.dxfeed.com:7300", {"AAPL", "IBM"}, 0).get();
}