#pragma once
#include "cth_log.hpp"

#include <cstdint>
#include <stb_image.h>

namespace cth::img {


struct stb_image {

    stb_image(const std::string_view path, const int desired_channels) {
        CTH_ERR(!std::filesystem::exists(path), "file not found") throw details->exception();

        int width = 0, height = 0, channels = 0;
        uint8_t* ptr = stbi_load(path.data(), &width, &height, &channels, desired_channels);

        CTH_ERR(width == 0 || height == 0, "failed to load file") throw details->exception();

        CTH_WARN(desired_channels != channels, "desired_chanels ({0}) != ({1}) loaded_channels", desired_channels, channels);

        this->_width = width;
        this->_height = height;
        this->_channels = channels;
        _pixels = std::span{ptr, static_cast<size_t>(width * height * channels)};
    }
    stb_image(const int width, const int height, const int channels, uint8_t* ptr) : _width(width), _height(height), _channels(channels),
        _pixels{ptr, static_cast<size_t>(width * height * channels)} {}
    ~stb_image() { stbi_image_free(_pixels.data()); }





private:
    uint32_t _width, _height, _channels;
    std::span<uint8_t> _pixels{};

public:
    [[nodiscard]] uint8_t* data() const { return _pixels.data(); }
    [[nodiscard]] std::span<char> raw() const {
        return std::span{reinterpret_cast<char*>(_pixels.data()), static_cast<size_t>(_width * _height * _channels)};
    }

    [[nodiscard]] std::span<uint8_t> pixels() const { return _pixels; }

    stb_image(const stb_image& other) = delete;
    stb_image(stb_image&& other) = default;
    stb_image& operator=(const stb_image& other) = delete;
    stb_image& operator=(stb_image&& other) = default;
};
} // namespace cth::img
