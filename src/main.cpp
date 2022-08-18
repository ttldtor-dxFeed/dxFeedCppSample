#include <future>
#include <iostream>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include <DXFeed.hpp>

/// The class that represents Quote processing entity with checking events by type Quote
struct QuoteProcessor : dxfcpp::AbstractEventCheckingProcessor<dxfcpp::Quote> {
    explicit QuoteProcessor() noexcept = default;

    void process(dxfcpp::Quote::Ptr e) override { std::cout << e->toString() + "\n"; }

    static AbstractEventProcessor::Ptr create() { return std::make_shared<QuoteProcessor>(); }
};

/// The class that represents Candle processing entity with checking events by type Candle
struct CandleProcessor : dxfcpp::AbstractEventCheckingProcessor<dxfcpp::Candle> {
    explicit CandleProcessor() noexcept = default;

    void process(dxfcpp::Candle::Ptr e) override { std::cout << e->toString() + "\n"; }

    static AbstractEventProcessor::Ptr create() { return std::make_shared<CandleProcessor>(); }
};

/**
 * Tests a subscription to Quote events
 *
 * @param c The parent connection pointer
 * @param symbols The quote symbols
 */
void testQuoteSubscription(dxfcpp::Connection::Ptr c, std::initializer_list<std::string> symbols) {
    dxfcpp::CompositeProcessor processor({QuoteProcessor::create()});

    for (const auto &s : symbols) {
        std::cout << s << " ";
    }

    std::cout << "QUOTES:" << std::endl;

    auto s = c->createSubscription({dxfcpp::EventType::QUOTE});

    s->onEvent() += [&processor](dxfcpp::Event::Ptr event) -> void { processor.process(event); };
    s->addSymbols(symbols);

    // Prevent automatic unsubscribing from events.
    std::this_thread::sleep_for(std::chrono::seconds(15));
}

/**
 * Tests getting a snapshot of candles. Returns the future for a snapshot of candles from some time to some time.
 * The vector of candles will be sorted in reverse order.
 * If the timeout occurs before the last candle has been received, then a future will be returned for an incomplete
 * snapshot of the candles.
 *
 * @param c The parent connection
 * @param candleSymbol The symbol of candle
 * @param fromTime Time from which events will be added to the snapshot (historical event buffer)
 * @param toTime The time until which events will be added to the snapshot (historical event buffer)
 * @param timeout The timeout after which the work completes.
 * @return A Future with a vector of smart pointers to Candle events
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
            std::cout << std::string("Status: ") + oldStatus.toString() + " -> " + newStatus.toString() + "\n";
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

    std::cout << "AAPL TRADES\n";

    auto s = c->createSubscription({dxfcpp::EventType::TRADE});
    s->onEvent() += [](dxfcpp::Event::Ptr e) { std::cout << e->toString() + "\n"; };

    s->addSymbol("AAPL");

    std::this_thread::sleep_for(std::chrono::seconds(15));
}