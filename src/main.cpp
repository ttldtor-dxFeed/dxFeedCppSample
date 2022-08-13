#include <future>
#include <iostream>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include <DXFeed.hpp>

struct QuoteProcessor : dxfcpp::AbstractEventCheckingProcessor<dxfcpp::Quote> {
    explicit QuoteProcessor() noexcept = default;

    void process(dxfcpp::Quote::Ptr e) override { std::cout << e->toString() + "\n"; }

    static AbstractEventProcessor::Ptr create() { return std::make_shared<QuoteProcessor>(); }
};

void testQuoteSubscription(const std::string &address, std::initializer_list<std::string> symbols) {
    dxfcpp::CompositeProcessor processor({QuoteProcessor::create()});

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

    s->onEvent() += [&processor](dxfcpp::Event::Ptr event) -> void { processor.process(std::move(event)); };
    s->onEvent() += [&processor](dxfcpp::Event::Ptr event) -> void { processor.process(std::move(event)); };
    s->onEvent() += [&processor](dxfcpp::Event::Ptr event) -> void { processor.process(std::move(event)); };
    //    s->onEvent() += [&processor3](dxfcpp::Event::Ptr event) -> void { processor3(std::move(event)); };
    s->addSymbols(symbols);

    std::this_thread::sleep_for(std::chrono::seconds(15));
}

std::future<std::vector<dxfcpp::Candle::Ptr>> testCandleSnapshot(const std::string &address, const std::string &candleSymbol,
                                                            long long fromTime, long long toTime, long timeout) {
    auto collector = dxfcpp::EventsCollector{};

    return collector.collectTimeSeriesSnapshot<dxfcpp::Candle>(address, candleSymbol, fromTime, toTime, timeout);
}

int main() {
    auto fromTimeString = "2022-08-04T00:00:00Z";
    auto toTimeString = "2022-08-05T00:00:00Z";
    auto fromTime = dxfcpp::DateTimeConverter::parseISO(fromTimeString);
    auto toTime = dxfcpp::DateTimeConverter::parseISO(toTimeString);

    auto result = testCandleSnapshot("demo.dxfeed.com:7300", "AAPL&Q{=1m}", fromTime, toTime, 20000).get();

    std::cout << "AAPL&Q{=1m} (" << fromTimeString << " - " << toTimeString << ") CANDLE SNAPSHOT RESULT:" << std::endl;

    for (auto const &e : result) {
        std::cout << e->toString() << std::endl;
    }

    testQuoteSubscription("demo.dxfeed.com:7300", {"AAPL", "IBM"});
}