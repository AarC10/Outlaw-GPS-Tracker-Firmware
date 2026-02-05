#include "core/HamCallsign.h"

#include <array>
#include <cctype>
#include <string>

HamCallsign::HamCallsign(std::string_view callsign) {
    set(callsign);
}

bool HamCallsign::set(std::string_view callsign) {
    valid = validateAndEncode(callsign);
    if (valid) {
        raw = callsign;
    } else {
        raw = {};
    }
    return valid;
}

std::string_view HamCallsign::getRaw() const {
    return raw;
}

std::uint8_t HamCallsign::encodeChar(char character) {
    if (character >= 'A' && character <= 'Z') {
        return static_cast<std::uint8_t>(1 + (character - 'A'));
    }
    if (character >= '0' && character <= '9') {
        return static_cast<std::uint8_t>(27 + (character - '0'));
    }
    switch (character) {
        case '/':
            return 37;
        case '-':
            return 38;
        case '^':
            return 39;
        default:
            return 0xFF;
    }
}

char HamCallsign::decodeChar(std::uint8_t code) {
    if (code == 0) {
        return '\0';
    }
    if (code >= 1 && code <= 26) {
        return static_cast<char>('A' + (code - 1));
    }
    if (code >= 27 && code <= 36) {
        return static_cast<char>('0' + (code - 27));
    }
    switch (code) {
        case 37:
            return '/';
        case 38:
            return '-';
        case 39:
            return '^';
        default:
            return '\0';
    }
}

bool HamCallsign::validateAndEncode(std::string_view callsign) {
    normalized.clear();
    chunks.fill(0);

    if (callsign.empty() || callsign.size() > maxLength) {
        return false;
    }

    std::array<std::uint8_t, maxLength> symbols {};
    std::size_t idx = 0;

    for (char character : callsign) {
        const auto upper = static_cast<char>(std::toupper(static_cast<unsigned char>(character)));
        const auto encoded = encodeChar(upper);
        if (encoded == 0xFF) {
            return false;
        }
        normalized.push_back(upper);
        symbols[idx++] = encoded;
    }

    for (std::size_t chunk = 0; chunk < chunkCount; ++chunk) {
        const std::size_t base = chunk * charsPerChunk;
        const auto c0 = (base < idx) ? symbols[base] : 0;
        const auto c1 = (base + 1 < idx) ? symbols[base + 1] : 0;
        const auto c2 = (base + 2 < idx) ? symbols[base + 2] : 0;
        chunks[chunk] = static_cast<std::uint16_t>(c0 * 1600 + c1 * 40 + c2);
    }

    return true;
}

bool HamCallsign::decodeChunks(const ChunkArray& chunks, std::string& out_callsign) {
    out_callsign.clear();

    if (chunks[0] == 0 || chunks[0] < 0x0640 || chunks[0] > 0xF9FF) {
        return false;
    }

    bool seen_null = false;

    for (std::size_t chunk = 0; chunk < chunkCount; ++chunk) {
        const auto value = chunks[chunk];

        if (value == 0) {
            seen_null = true;
            continue;
        }

        if (value < 0x0640 || value > 0xF9FF) {
            return false;
        }

        const std::uint8_t c0 = static_cast<std::uint8_t>((value / 1600) % 40);
        const std::uint8_t c1 = static_cast<std::uint8_t>((value / 40) % 40);
        const std::uint8_t c2 = static_cast<std::uint8_t>(value % 40);
        const std::uint8_t trio[3] {c0, c1, c2};

        for (std::uint8_t code : trio) {
            if (code == 0) {
                seen_null = true;
                continue;
            }
            if (seen_null) {
                return false;
            }
            const char decoded = decodeChar(code);
            if (decoded == '\0') {
                return false;
            }
            out_callsign.push_back(decoded);
            if (out_callsign.size() > maxLength) {
                return false;
            }
        }
    }

    return !out_callsign.empty();
}

std::string HamCallsign::decode() const {
    std::string out;
    if (!decodeChunks(chunks, out)) {
        return {};
    }
    return out;
}
