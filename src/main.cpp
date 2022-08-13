#include <future>
#include <iostream>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include <DXFeed.hpp>

///
struct QuoteProcessor : dxfcpp::AbstractEventCheckingProcessor<dxfcpp::Quote> {
    explicit QuoteProcessor() noexcept = default;

    void process(dxfcpp::Quote::Ptr e) override { std::cout << e->toString() + "\n"; }

    static AbstractEventProcessor::Ptr create() { return std::make_shared<QuoteProcessor>(); }
};

///
struct CandleProcessor : dxfcpp::AbstractEventCheckingProcessor<dxfcpp::Candle> {
    explicit CandleProcessor() noexcept = default;

    void process(dxfcpp::Candle::Ptr e) override { std::cout << e->toString() + "\n"; }

    static AbstractEventProcessor::Ptr create() { return std::make_shared<CandleProcessor>(); }
};

/**
 *
 * @param c
 * @param symbols
 */
void testQuoteSubscription(dxfcpp::Connection::Ptr c, std::initializer_list<std::string> symbols) {
    dxfcpp::CompositeProcessor processor({QuoteProcessor::create()});

    for (const auto &s : symbols) {
        std::cout << s << " ";
    }

    std::cout << "QUOTES:" << std::endl;

    auto s = c->createSubscription({dxfcpp::EventType::QUOTE});

    s->onEvent() += [&processor](dxfcpp::Event::Ptr event) -> void { processor.process(std::move(event)); };
    s->addSymbols(symbols);

    std::this_thread::sleep_for(std::chrono::seconds(15));
}

/**
 *
 * @param c
 * @param candleSymbol
 * @param fromTime
 * @param toTime
 * @param timeout
 * @return
 */
std::vector<dxfcpp::Candle::Ptr> testCandleSnapshot(dxfcpp::Connection::Ptr c, const std::string &candleSymbol,
                                                    std::uint64_t fromTime, std::uint64_t toTime, long timeout) {
    return c->getTimeSeriesFuture<dxfcpp::Candle>(candleSymbol, fromTime, toTime, timeout).get();
}

int main() {
    std::string address = "demo.dxfeed.com:7300";

    auto c = dxfcpp::DXFeed::connect(
        address, []() {},
        [](const dxfcpp::ConnectionStatus &oldStatus, const dxfcpp::ConnectionStatus &newStatus) {
            std::cout << "Status: " << oldStatus << " -> " << newStatus << std::endl;
        });

    auto fromTimeString = "2022-08-04T00:00:00Z";
    auto toTimeString = "2022-08-05T00:00:00Z";
    auto fromTime = dxfcpp::DateTimeConverter::parseISO(fromTimeString);
    auto toTime = dxfcpp::DateTimeConverter::parseISO(toTimeString);

    auto result = testCandleSnapshot(c, "AAPL&Q{=1m}", fromTime, toTime, 20);

    std::cout << "AAPL&Q{=1m} (" << fromTimeString << " - " << toTimeString << ") CANDLE SNAPSHOT RESULT:" << std::endl;

    for (auto const &e : result) {
        std::cout << e->toString() << std::endl;
    }

    testQuoteSubscription(c, {"AAPL", "IBM"});
}