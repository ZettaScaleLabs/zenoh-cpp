//
// Copyright (c) 2024 ZettaScale Technology
//
// This program and the accompanying materials are made available under the
// terms of the Eclipse Public License 2.0 which is available at
// http://www.eclipse.org/legal/epl-2.0, or the Apache License, Version 2.0
// which is available at https://www.apache.org/licenses/LICENSE-2.0.
//
// SPDX-License-Identifier: EPL-2.0 OR Apache-2.0
//
// Contributors:
//   ZettaScale Zenoh Team, <zenoh@zettascale.tech>
//

#include <chrono>
#include <thread>

#include "zenoh.hxx"

using namespace zenoh;
using namespace std::chrono_literals;

#undef NDEBUG
#include <assert.h>

struct CommonAllocator {
    Bytes alloc_with_data(const char* data) { return Bytes(data); }
};

#if defined Z_FEATURE_SHARED_MEMORY && defined Z_FEATURE_UNSTABLE_API
class SHMAllocator {
    PosixShmProvider provider;

   public:
    SHMAllocator() : provider(MemoryLayout(1024 * 1024, AllocAlignment({2}))) {}

    Bytes alloc_with_data(const char* data) {
        const auto len = strlen(data);
        auto alloc_result = provider.alloc_gc_defrag_blocking(len, AllocAlignment({0}));
        ZShmMut&& buf = std::get<ZShmMut>(std::move(alloc_result));
        memcpy(buf.data(), data, len + 1);
        return Bytes(std::move(buf));
    }
};
#endif

template <typename Talloc>
void pub_sub(Talloc& alloc) {
    KeyExpr ke("zenoh/test");
    auto session1 = Session::open(Config::create_default());
    auto session2 = Session::open(Config::create_default());

    auto publisher = session1.declare_publisher(ke);

    std::this_thread::sleep_for(1s);

    bool subscriber_dropped = false;
    std::vector<std::pair<std::string, std::string>> received_messages;

    auto subscriber = session2.declare_subscriber(
        ke,
        [&received_messages](const Sample& s) {
            received_messages.emplace_back(s.get_keyexpr().as_string_view(), s.get_payload().as_string());
        },
        [&subscriber_dropped]() { subscriber_dropped = true; });

    std::this_thread::sleep_for(1s);

    publisher.put(alloc.alloc_with_data("first"));
    publisher.put(alloc.alloc_with_data("second"));

    std::this_thread::sleep_for(1s);
    std::move(subscriber).undeclare();

    assert(received_messages.size() == 2);
    assert(received_messages[0].first == "zenoh/test");
    assert(received_messages[0].second == "first");
    assert(received_messages[1].first == "zenoh/test");
    assert(received_messages[1].second == "second");
    assert(subscriber_dropped);
}

template <typename Talloc>
void put_sub(Talloc& alloc) {
    KeyExpr ke("zenoh/test");
    auto session1 = Session::open(Config::create_default());
    auto session2 = Session::open(Config::create_default());

    std::this_thread::sleep_for(1s);

    std::vector<std::pair<std::string, std::string>> received_messages;

    auto subscriber = session2.declare_subscriber(
        ke,
        [&received_messages](const Sample& s) {
            received_messages.emplace_back(s.get_keyexpr().as_string_view(), s.get_payload().as_string());
        },
        closures::none);

    std::this_thread::sleep_for(1s);

    session1.put(ke, alloc.alloc_with_data("first"));
    session1.put(ke, alloc.alloc_with_data("second"));

    std::this_thread::sleep_for(1s);

    assert(received_messages.size() == 2);
    assert(received_messages[0].first == "zenoh/test");
    assert(received_messages[0].second == "first");
    assert(received_messages[1].first == "zenoh/test");
    assert(received_messages[1].second == "second");
    std::move(subscriber).undeclare();
}

template <typename Talloc>
void put_sub_fifo_channel(Talloc& alloc) {
    KeyExpr ke("zenoh/test");
    auto session1 = Session::open(Config::create_default());
    auto session2 = Session::open(Config::create_default());

    std::this_thread::sleep_for(1s);

    std::vector<Subscriber<channels::FifoChannel::HandlerType<Sample>>> subscribers;
    subscribers.push_back(session2.declare_subscriber(ke, channels::FifoChannel(16)));
    subscribers.push_back(session2.declare_subscriber(ke, channels::FifoChannel(16)));

    std::this_thread::sleep_for(1s);

    session1.put(ke, alloc.alloc_with_data("first"));
    session1.put(ke, alloc.alloc_with_data("second"));

    std::this_thread::sleep_for(1s);

    for (const auto& subscriber : subscribers) {
        auto res = subscriber.handler().recv();
        assert(std::holds_alternative<Sample>(res));
        assert(std::get<Sample>(res).get_keyexpr() == "zenoh/test");
        assert(std::get<Sample>(res).get_payload().as_string() == "first");
        res = subscriber.handler().try_recv();
        assert(std::holds_alternative<Sample>(res));
        assert(std::get<Sample>(res).get_keyexpr() == "zenoh/test");
        assert(std::get<Sample>(res).get_payload().as_string() == "second");

        res = subscriber.handler().try_recv();
        assert(std::holds_alternative<channels::RecvError>(res));
        assert(std::get<channels::RecvError>(res) == channels::RecvError::Z_NODATA);
    }

    /// after session close subscriber handler should become disconnected
    session2.close();
    for (auto& subscriber : subscribers) {
        auto res = subscriber.handler().recv();
        assert(std::holds_alternative<channels::RecvError>(res));
        assert(std::get<channels::RecvError>(res) == channels::RecvError::Z_DISCONNECTED);
        z_result_t err;
        std::move(subscriber).undeclare(&err);  // a error can be thrown since session2 was closed
    }
}

template <typename Talloc>
void put_sub_ring_channel(Talloc& alloc) {
    KeyExpr ke("zenoh/test");
    auto session1 = Session::open(Config::create_default());
    auto session2 = Session::open(Config::create_default());

    std::this_thread::sleep_for(1s);

    std::vector<Subscriber<channels::RingChannel::HandlerType<Sample>>> subscribers;
    subscribers.push_back(session2.declare_subscriber(ke, channels::RingChannel(1)));
    subscribers.push_back(session2.declare_subscriber(ke, channels::RingChannel(1)));

    std::this_thread::sleep_for(1s);

    session1.put(ke, alloc.alloc_with_data("first"));
    session1.put(ke, alloc.alloc_with_data("second"));

    std::this_thread::sleep_for(1s);

    for (const auto& subscriber : subscribers) {
        auto res = subscriber.handler().recv();
        assert(std::holds_alternative<Sample>(res));
        assert(std::get<Sample>(res).get_keyexpr() == "zenoh/test");
        assert(std::get<Sample>(res).get_payload().as_string() == "second");

        res = subscriber.handler().try_recv();
        assert(std::holds_alternative<channels::RecvError>(res));
        assert(std::get<channels::RecvError>(res) == channels::RecvError::Z_NODATA);
    }

    /// after session close subscriber handler should become disconnected
    session2.close();
    for (auto& subscriber : subscribers) {
        auto res = subscriber.handler().recv();
        assert(std::holds_alternative<channels::RecvError>(res));
        assert(std::get<channels::RecvError>(res) == channels::RecvError::Z_DISCONNECTED);
        z_result_t err;
        std::move(subscriber).undeclare(&err);  // a error can be thrown since session2 was closed
    }
}

template <typename Talloc, bool share_alloc = true>
void test_with_alloc() {
    if constexpr (share_alloc) {
        Talloc alloc;
        pub_sub(alloc);
        put_sub(alloc);
        put_sub_fifo_channel(alloc);
        put_sub_ring_channel(alloc);
    } else {
        {
            Talloc alloc;
            pub_sub(alloc);
        }
        {
            Talloc alloc;
            put_sub(alloc);
        }
        {
            Talloc alloc;
            put_sub_fifo_channel(alloc);
        }
        {
            Talloc alloc;
            put_sub_ring_channel(alloc);
        }
    }
}

void publisher_get_keyexpr() {
    KeyExpr ke("zenoh/test_publisher_keyexpr");
    auto session = Session::open(Config::create_default());
    auto publisher = session.declare_publisher(ke);
    assert(publisher.get_keyexpr().as_string_view() == "zenoh/test_publisher_keyexpr");
}

int main(int argc, char** argv) {
    test_with_alloc<CommonAllocator>();
#if defined Z_FEATURE_SHARED_MEMORY && defined Z_FEATURE_UNSTABLE_API
    test_with_alloc<SHMAllocator>();
    test_with_alloc<SHMAllocator, false>();
#endif
    publisher_get_keyexpr();
    return 0;
}
