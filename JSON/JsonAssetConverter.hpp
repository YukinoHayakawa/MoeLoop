﻿#pragma once

#include <memory>

#include "JSON.hpp"
#include "JsonAssetDecoder.hpp"

namespace usagi
{
struct AssetLoadingContext;
}

namespace usagi::negi
{
template <typename T>
struct JsonAssetConverter
{
    using DefaultDecoder = JsonAssetDecoder;

    template <typename... Args>
    std::shared_ptr<T> operator()(
        AssetLoadingContext *ctx,
        const json &j,
        Args &&...args) const
    {
        auto r = std::make_shared<T>(std::forward<Args>(args)...);
        *r = j;
        return std::move(r);
    }
};
}
