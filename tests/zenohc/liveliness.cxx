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

#include "zenoh.hxx"

#include <thread>
#include <unordered_set>

using namespace zenoh;
using namespace std::chrono_literals;

#undef NDEBUG
#include <assert.h>

void test_liveliness_get() {
    KeyExpr ke("zenoh/liveliness/test/*");
    KeyExpr token_ke("zenoh/liveliness/test/1");
    auto session1 = Session::open(Config());
    auto session2 = Session::open(Config());
    auto token = session1.liveliness_declare_token(token_ke);
    std::this_thread::sleep_for(1s);

    auto replies = session2.liveliness_get(ke, channels::FifoChannel(3));
    auto reply = replies.recv().first;
    assert(static_cast<bool>(reply));
    assert(reply.is_ok());
    assert(reply.get_ok().get_keyexpr() == "zenoh/liveliness/test/1");
    reply = replies.recv().first;
    assert(!replies.recv().first);

    std::move(token).undeclare();
    std::this_thread::sleep_for(1s);

    replies = session2.liveliness_get(ke, channels::FifoChannel(3));
    assert(!replies.recv().first);
}

void test_liveliness_subscriber() {
    KeyExpr ke("zenoh/liveliness/test/*");
    KeyExpr token_ke1("zenoh/liveliness/test/1");
    KeyExpr token_ke2("zenoh/liveliness/test/2");
    auto session1 = Session::open(Config());
    auto session2 = Session::open(Config());

    std::unordered_set<std::string> put_tokens;
    std::unordered_set<std::string> delete_tokens;

    auto subscriber = session1.liveliness_declare_subscriber(
        ke, 
        [&put_tokens, &delete_tokens](const Sample& s) { 
            if (s.get_kind() == Z_SAMPLE_KIND_PUT) {
                put_tokens.insert(std::string(s.get_keyexpr().as_string_view()));
            } else if (s.get_kind() == Z_SAMPLE_KIND_DELETE) {
                delete_tokens.insert(std::string(s.get_keyexpr().as_string_view()));
            }
        },
        closures::none
    );
    std::this_thread::sleep_for(1s);

    auto token1 = session2.liveliness_declare_token(token_ke1);
    auto token2 = session2.liveliness_declare_token(token_ke2);
    std::this_thread::sleep_for(1s);
    assert(put_tokens.size() == 2);
    assert(put_tokens.count("zenoh/liveliness/test/1") == 1);
    assert(put_tokens.count("zenoh/liveliness/test/2") == 1);


    std::move(token1).undeclare();
    std::this_thread::sleep_for(1s);

    assert(delete_tokens.size() == 1);
    assert(delete_tokens.count("zenoh/liveliness/test/1") == 1);

    std::move(token2).undeclare();
    std::this_thread::sleep_for(1s);
    assert(delete_tokens.size() == 2);
    assert(delete_tokens.count("zenoh/liveliness/test/2") == 1);
}


int main(int argc, char** argv) {
    test_liveliness_get();
    test_liveliness_subscriber();
};