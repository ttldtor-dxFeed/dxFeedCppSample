#include <DXFeed.h>
#include <iostream>
#include <vector>
#include <string>

void testQuoteSubscription(const std::string& address, const std::vector<std::string>& symbols, long timeout) {

}

template <typename Candle>
std::vector<Candle> testCandleSnapshot(const std::string& address, const std::string& candleSymbol, long long fromTime, long long toTime, long timeout) {
  return {};
}

int main() {
  auto result = testCandleSnapshot<void*>("demo.dxfeed.com:7300", "AAPL&Q", 0, 0, 0);

  testQuoteSubscription("demo.dxfeed.com:7300", {"AAPL", "IBM"}, 10000);
}