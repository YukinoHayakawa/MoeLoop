﻿#include "JsonAssetDecoder.hpp"

#include <nlohmann/json.hpp>

usagi::negi::json usagi::negi::JsonAssetDecoder::operator()(
    std::istream &in) const
{
    json j;
    in >> j;
    return std::move(j);
}
