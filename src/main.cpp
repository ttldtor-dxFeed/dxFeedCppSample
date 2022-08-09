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

#include "wrappers/Connection.hpp"
#include "wrappers/EventTraits.hpp"
#include "wrappers/Subscription.hpp"

struct CountingVisitor {
    mutable std::atomic<unsigned long> counter{};

    void operator()(const dxfcpp::Quote &q) const {
        counter++;
        std::cout << std::to_string(counter) + "): " + q.toString() + "\n";
    }

    void operator()(const dxfcpp::Candle &c) const {
        counter++;
    }
};

void testQuoteSubscription(const std::string &address, std::initializer_list<std::string> symbols) {
    CountingVisitor v{};

    for (const auto &s : symbols) {
        std::cout << s << " ";
    }

    std::cout << "QUOTES:" << std::endl;

    auto c = dxfcpp::DXFeed::connect(
        address, []() { std::cout << "Disconnected" << std::endl; },
        [](const dxfcpp::ConnectionStatus &oldStatus, const dxfcpp::ConnectionStatus &newStatus) {
            std::cout << "Status: " << oldStatus << " -> " << newStatus << std::endl;
        });

    auto s = c->createSubscription({dxfcpp::EventType::QUOTE});

    s->onEvent() += [&v](const dxfcpp::Subscription::Event &event) { nonstd::visit(v, event); };
    s->addSymbols(symbols);

    std::this_thread::sleep_for(std::chrono::seconds(10));
}

std::future<std::vector<dxfcpp::Candle>> testCandleSnapshot(const std::string &address, const std::string &candleSymbol,
                                                            long long fromTime, long long toTime, long timeout) {
    auto collector = dxfcpp::EventsCollector{};

    return collector.collectTimeSeriesSnapshot<dxfcpp::Candle>(address, candleSymbol, fromTime, toTime, timeout);
}

int main() {
//    auto fromTimeString = "2022-08-04T00:00:00Z";
//    auto toTimeString = "2022-08-05T00:00:00Z";
//    auto fromTime = dxfcpp::DateTimeConverter::parseISO(fromTimeString);
//    auto toTime = dxfcpp::DateTimeConverter::parseISO(toTimeString);
//
//    auto result = testCandleSnapshot("demo.dxfeed.com:7300", "AAPL&Q{=1m}", fromTime, toTime, 20000).get();
//
//    std::cout << "AAPL&Q{=1m} (" << fromTimeString << " - " << toTimeString << ") CANDLE SNAPSHOT RESULT:" << std::endl;
//
//    for (auto const &e : result) {
//        std::cout << e.toString() << std::endl;
//    }

    testQuoteSubscription("demo.dxfeed.com:7300", {"AAPL", "IBM", "TSLA", "AMZN"});
}