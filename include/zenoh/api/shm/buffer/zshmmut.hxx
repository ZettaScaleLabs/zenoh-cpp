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

#pragma once

#include "../../base.hxx"

namespace zenoh {

/// @brief A mutable SHM buffer
class ZShmMut : public Owned<::z_owned_shm_mut_t> {
    friend class ZShm;

   public:
    using Owned::Owned;

    /// @brief Get buffer's const data
    /// @return pointer to the underlying data
    const uint8_t* data() const { return ::z_shm_mut_data(this->loan()); }

    /// @brief Get buffer's data
    /// @return pointer to the underlying data
    uint8_t* data() { return ::z_shm_mut_data_mut(this->loan()); }

    /// @brief Get buffer's data size.
    /// @return underlying data size
    std::size_t len() const { return ::z_shm_mut_len(this->loan()); }
};

}  // end of namespace zenoh
