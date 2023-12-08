//
// Copyright (c) 2023 ZettaScale Technology
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

#include <map>
#include <string>

#include "zenohc.hxx"

using namespace zenohc;

#undef NDEBUG
#include <assert.h>

void writting_through_map_read_by_get() {
    // Writing
    std::map<std::string, std::string> amap;
    amap.insert(std::make_pair("k1", "v1"));
    amap.insert(std::make_pair("k2", "v2"));
    auto attachment = as_attachment(amap);

    // Size check
    assert(attachment.get_len() == 2);

    // Elements check
    assert(attachment.get("k1").as_string_view() == "v1");
    assert(attachment.get("k2").as_string_view() == "v2");
    // TODO(sashacmc): default value or error for unexisted key?
    // do we need also some macro to convert to the map?
    assert(attachment.get("k_non").as_string_view() == "");
}

int8_t _attachment_reader(const BytesView& key, const BytesView& value, void* ctx) {
    assert((size_t)ctx == 42);
    if (key.as_string_view() == "k1") {
        assert(value.as_string_view() == "v1");
        return 0;
    } else if (key.as_string_view() == "k2") {
        assert(value.as_string_view() == "v2");
        return 0;
    }
    assert(!"Unexpected");
}

void writting_through_map_read_by_iter() {
    // Writing
    std::map<std::string, std::string> amap;
    auto attachment = as_attachment(amap);
    // amap passed by reference we can change it after
    amap.insert(std::make_pair("k1", "v1"));
    amap.insert(std::make_pair("k2", "v2"));

    // Size check
    assert(attachment.get_len() == 2);

    // Elements check
    int res = attachment.iterate(_attachment_reader, (void*)42);
}

void writting_no_map_read_by_get() {
    AttachmentVTable vtable(
        [](const void* data, z_attachment_iter_body_t body, void* ctx) -> int8_t {
            int8_t ret = 0;
            ret = body(BytesView("k1"), BytesView("v1"), ctx);
            if (ret) {
                return ret;
            }
            ret = body(BytesView("k2"), BytesView("v2"), ctx);
            return ret;
        },
        [](const void* data) -> size_t { return 2; });

    AttachmentView attachment(nullptr, vtable);

    // Size check
    assert(attachment.get_len() == 2);

    // Elements check
    assert(attachment.get("k1").as_string_view() == "v1");
    assert(attachment.get("k2").as_string_view() == "v2");
    assert(attachment.get("k_non").as_string_view() == "");
}

int main(int argc, char** argv) {
    init_logger();
    writting_through_map_read_by_get();
    writting_through_map_read_by_iter();
    writting_no_map_read_by_get();
}
