#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

class HamCallsign {
private:
    static constexpr std::size_t maxLength = 12;
    static constexpr std::size_t charsPerChunk = 3;
    static constexpr std::size_t chunkCount = 4;
    using ChunkArray = std::array<std::uint16_t, chunkCount>;

public:
    HamCallsign() = default;
    explicit HamCallsign(std::string_view callsign);

    bool set(std::string_view callsign);
    bool isValid() const { return valid; }
    const std::string& callsign() const { return normalized; }
    const ChunkArray& encodedChunks() const { return chunks; }
    [[nodiscard]] std::size_t length() const { return normalized.size(); }
    [[nodiscard]] std::string decode() const;
    static bool decodeChunks(const ChunkArray& chunks, std::string& out_callsign);

private:
    static std::uint8_t encodeChar(char c);
    static char decodeChar(std::uint8_t code);
    bool validateAndEncode(std::string_view callsign);

    std::string normalized {};
    ChunkArray chunks {};
    bool valid {false};
};