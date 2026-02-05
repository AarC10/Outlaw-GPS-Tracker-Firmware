#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

class HamCallsign {
public:
    static constexpr std::size_t maxLength = 12;
    static constexpr std::size_t charsPerChunk = 3;
    static constexpr std::size_t chunkCount = 4;
    using ChunkArray = std::array<std::uint16_t, chunkCount>;

    HamCallsign() = default;

    explicit HamCallsign(std::string_view callsign);

    /**
     * Set and encode the callsign
     * @param callsign Callsign to set and encode
     * @return True if the callsign is valid and was encoded successfully, false otherwise
     */
    bool set(std::string_view callsign);

    /**
     * Get the raw callsign string
     * @return Raw callsign string
     */
     [[nodiscard]] std::string_view getRaw() const;


    /**
     * Check if the callsign is valid
     * @return  True if the callsign is valid, false otherwise
     */
    bool isValid() const { return valid; }

    /**
     * Get the normalized callsign string
     * @return Callsign string
     */
    const std::string& callsign() const { return normalized; }

    /**
     * Get the encoded chunks array
     * @return Chunks array representing the encoded callsign
     */
    const ChunkArray& encodedChunks() const { return chunks; }

    /**
     * Get the length of the normalized callsign
     * @return Length of the callsign
     */
    [[nodiscard]] std::size_t length() const { return normalized.size(); }

    /**
     * Decode the encoded callsign back to string
     * @return  Decoded callsign string
     */
    [[nodiscard]] std::string decode() const;

    /**
     * Static method to decode chunks into a callsign string
     * @param chunks Chunk array to decode
     * @param out_callsign Decoded callsign output
     * @return True if decoding was successful, false otherwise
     */
    static bool decodeChunks(const ChunkArray& chunks, std::string& out_callsign);

private:
    std::string raw{};
    std::string normalized {};
    ChunkArray chunks {};
    bool valid {false};

    /**
     * Encode a single character
     * @param c Character to encode
     * @return  Encoded value of the character
     */
    static std::uint8_t encodeChar(char c);

    /**
     * Decode a single encoded value back to character
     * @param code Encoded value
     * @return Decoded character
     */
    static char decodeChar(std::uint8_t code);

    /**
     * Validate and encode the callsign
     * @param callsign Callsign to validate and encode
     * @return True if valid and encoded successfully, false otherwise
     */
    bool validateAndEncode(std::string_view callsign);


};