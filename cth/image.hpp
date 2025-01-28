#pragma once
#include "io/log.hpp"

#include <cstdint>
#include <stb_image.h>

namespace cth::img {


struct stb_image {

    stb_image(std::string const& path, int desired_channels) {
        CTH_ERR(!std::filesystem::exists(path), "file not found") throw details->exception();

        int width = 0, height = 0, channels = 0;
        uint8_t* ptr = stbi_load(path.data(), &width, &height, &channels, desired_channels);

        CTH_ERR(width == 0 || height == 0, "failed to load file") throw details->exception();

        CTH_WARN(desired_channels != channels, "desired_channels ({0}) != ({1}) loaded_channels", desired_channels, channels) {}

        _width = width;
        _height = height;
        _channels = channels;
        _data = std::span{ptr, static_cast<size_t>(width * height * channels)};
    }
    stb_image(int width, int height, int channels, uint8_t* ptr) : _width(width), _height(height), _channels(channels),
        _data{ptr, static_cast<size_t>(width * height * channels)} {}
    ~stb_image() { stbi_image_free(_data.data()); }

private:
    size_t _width, _height, _channels;
    std::span<uint8_t> _data{};

public:
    [[nodiscard]] size_t size() const { return _width * _height * _channels; }
    [[nodiscard]] size_t width() const { return _width; }
    [[nodiscard]] size_t height() const { return _height; }
    [[nodiscard]] size_t channels() const { return _channels; }
    [[nodiscard]] size_t pixels() const { return _width * _height; }

    [[nodiscard]] auto& operator [](size_t index) {return _data[index];}
    [[nodiscard]] auto operator [](size_t index) const {return _data[index];}


    [[nodiscard]] uint8_t const* data() const { return _data.data(); }
    [[nodiscard]] uint8_t* data() { return _data.data(); }

    [[nodiscard]] std::span<uint8_t const> view() const { return _data; }
    [[nodiscard]] std::span<uint8_t> view() { return _data; }

    [[nodiscard]] char* raw() { return reinterpret_cast<char*>(_data.data()); }
    [[nodiscard]] char const* raw() const { return reinterpret_cast<char*>(_data.data()); }

    [[nodiscard]] auto begin() const { return view().cbegin(); }
    [[nodiscard]] auto begin() { return view().begin(); }
    [[nodiscard]] auto end() const { return view().cbegin(); }
    [[nodiscard]] auto end() { return view().begin(); }

    stb_image(stb_image const& other) = delete;
    stb_image(stb_image&& other) = default;
    stb_image& operator=(stb_image const& other) = delete;
    stb_image& operator=(stb_image&& other) = default;
};
} // namespace cth::img
