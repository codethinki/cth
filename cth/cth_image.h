#pragma once
#include "cth_log.hpp"

#include <cstdint>
#include <stb_image.h>

namespace cth::img {
using namespace std;

struct stb_image {

    stb_image(const string_view path, const int desired_channels) {
        CTH_ERR(!filesystem::exists(path), "file not found") throw details->exception();

        int width, height, channels;
        uint8_t* ptr = stbi_load(path.data(), &width, &height, &channels, desired_channels);

        CTH_ERR(width == 0 || height == 0, "failed to load file") throw details->exception();

        CTH_WARN(desired_channels != channels, "desired_chanels ({0}) != ({1}) loaded_channels", desired_channels, channels);

        this->width = width;
        this->height = height;
        this->channels = channels;
        pixels = span{ptr, static_cast<size_t>(width * height * channels)};
    }
    stb_image(const int width, const int height, const int channels, uint8_t* ptr) : width(width), height(height), channels(channels),
        pixels{ptr, static_cast<size_t>(width * height * channels)} {}
    ~stb_image() { stbi_image_free(pixels.data()); }
    uint32_t width, height, channels;
    span<uint8_t> pixels{};

    [[nodiscard]] uint8_t* data() const { return pixels.data(); }
    [[nodiscard]] span<char> raw() const { return span{reinterpret_cast<char*>(pixels.data()), static_cast<size_t>(width * height * channels)}; }

    stb_image(const stb_image& other) = delete;
    stb_image(stb_image&& other) = default;
    stb_image& operator=(const stb_image& other) = delete;
    stb_image& operator=(stb_image&& other) = default;
};
}
