#pragma once

#include <DXFeed.h>
#include <EventData.h>

#include <unordered_set>


namespace dxfcpp {

template <typename E>
struct Subscription {
    dxf_subscription_t subscriptionHandle_ = nullptr;

};

}