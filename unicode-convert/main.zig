const std = @import("std");
const assert = std.debug.assert;
const Endian = std.builtin.Endian;
const Io = std.Io;

// Sources used:
//   https://en.wikipedia.org/wiki/UTF-8#Description
//   https://en.wikipedia.org/wiki/UTF-16#Examples
pub const Codepoint = struct {
    code: u21,

    pub const Encoding = enum(u4) {
        utf8,
        utf16le,
        utf16be,
        utf32le,
        utf32be,
    };

    // The number of bytes the codepoint would require if encoded
    // in the specified Unicode encoding.
    // Returns `null` if the codepoint does not represent
    // a character (like if it represents a surrogate).
    pub fn lengthEncoded(codepoint: Codepoint, encoding: Encoding) usize {
        const code = codepoint.code;
        return switch (encoding) {
            .utf8 => if (code < 0x0080)
                1
            else if (code < 0x0800)
                2
            else if (code < 0x1_0000)
                3
            else
                4,
            .utf16le, .utf16be => if (code < 0x1_0000) 2 else 4,
            .utf32le, .utf32be => 4,
        };
    }

    pub fn isSurrogate(code_unit: u16) bool {
        return code_unit >= 0xD800 and code_unit < 0xE000;
    }

    pub const EncodeError = error{
        CodepointOutOfRange,
    };

    // The `.lengthEncoded()` method can tell you how many bytes will be written
    // to the writer.
    pub fn encodeOne(codepoint: Codepoint, encoding: Encoding, w: *Io.Writer) (Io.Writer.Error || EncodeError)!usize {
        const code = codepoint.code;

        if (code > 0x10_FFFF or (code < 0x1_0000 and Codepoint.isSurrogate(@intCast(code))))
            return EncodeError.CodepointOutOfRange;

        return switch (encoding) {
            .utf8 => blk: {
                if (code < 0x00_0080) {
                    // U+0000yz = 0000_0000 0000_0000 0yyy_zzzz = 0yyyzzzz
                    try w.writeByte(@intCast(code));
                    break :blk 1;
                } else if (code < 0x00_0800) {
                    // U+000xyz = 0000_0000 0000_0xxx yyyy_zzzz = 110xxxyy 10yyzzzz
                    try w.writeAll(&.{
                        0xC0 | @as(u8, @intCast(code >> 6)),
                        0x80 | @as(u8, @intCast(code & 0x00_003F)),
                    });
                    break :blk 2;
                } else if (code < 0x01_0000) {
                    // U+00wxyz = 0000_0000 wwww_xxxx yyyy_zzzz = 1110wwww 10xxxxyy 10yyzzzz
                    try w.writeAll(&.{
                        0xE0 | @as(u8, @intCast(code >> 12)),
                        0x80 | @as(u8, @intCast((code & 0x00_0FC0) >> 6)),
                        0x80 | @as(u8, @intCast(code & 0x00_003F)),
                    });
                    break :blk 3;
                } else {
                    // U+uvwxyz = 000u_vvvv wwww_xxxx yyyy_zzzz = 11110uvv 10vvwwww 10xxxxyy 10yyzzzz
                    try w.writeAll(&.{
                        0xF0 | @as(u8, @intCast(code >> 18)),
                        0x80 | @as(u8, @intCast((code & 0x03_F000) >> 12)),
                        0x80 | @as(u8, @intCast((code & 0x00_0FC0) >> 6)),
                        0x80 | @as(u8, @intCast(code & 0x00_003F)),
                    });
                    break :blk 4;
                }
            },
            // https://en.wikipedia.org/wiki/UTF-16#Examples
            .utf16le, .utf16be => blk: {
                const endian: Endian = if (encoding == .utf16le) Endian.little else Endian.big;

                if (code < 0x01_0000) {
                    try w.writeInt(u16, @intCast(code), endian);
                    break :blk 2;
                } else {
                    const subbed = code - 0x01_0000;
                    // bitwise OR-ing this because it doesn't matter and I like how it looks
                    const high_surrogate: u16 = 0xD800 | @as(u16, @intCast(subbed >> 10));
                    const low_surrogate: u16 = 0xDC00 | @as(u16, @intCast(subbed & 0x03FF)); // lower 10 bits
                    try w.writeInt(u16, high_surrogate, endian);
                    try w.writeInt(u16, low_surrogate, endian);
                    break :blk 4;
                }
            },
            .utf32le, .utf32be => blk: {
                const endian: Endian = if (encoding == .utf32le) Endian.little else Endian.big;
                try w.writeInt(u32, code, endian);
                break :blk 4;
            },
        };
    }

    pub const DecodeError = error{
        InputTooShort,
    };

    // Returns the decoded codepoint and the length it takes up from the start of the buffer.
    //
    // Some invalid unicode (ex. leading code bytes with >4 leading ones) will not be checked for.
    // It is the callers responsibility to check for these errors, but a few will be detected here.
    //
    // Errors if the code ends before it should.
    pub fn decodeOne(encoding: Encoding, r: *Io.Reader) DecodeError!?struct { Codepoint, usize } {
        return switch (encoding) {
            .utf8 => blk: {
                const first_byte = r.takeByte() catch return null;

                if ((first_byte & 0x80) == 0) {
                    break :blk .{ .{ .code = first_byte }, 1 };
                }
                const code_len: u3 = switch (first_byte) {
                    0b0000_0000...0b0111_1111 => 1,
                    0b1100_0000...0b1101_1111 => 2,
                    0b1110_0000...0b1110_1111 => 3,
                    0b1111_0000...0b1111_0111 => 4,
                    else => break :blk .{ .replacement_character, 3 },
                };
                const byte2 = r.takeByte() catch return DecodeError.InputTooShort;
                const code: Codepoint = inner: switch (code_len) {
                    2 => break :inner .{ .code = (@as(u21, first_byte & 0x1F) << 6) |
                        @as(u21, byte2 & 0x3F) },
                    3 => break :inner .{ .code = (@as(u21, first_byte & 0x0F) << 12) |
                        (@as(u21, byte2 & 0x3F) << 6) |
                        @as(u21, (r.takeByte() catch return DecodeError.InputTooShort) & 0x3F) },
                    4 => {
                        const last_2_bytes = r.take(2) catch return DecodeError.InputTooShort;
                        break :inner .{ .code = (@as(u21, first_byte & 0x03) << 18) |
                            (@as(u21, byte2 & 0x3F) << 12) |
                            (@as(u21, last_2_bytes[0] & 0x3F) << 6) |
                            @as(u21, last_2_bytes[1] & 0x3F) };
                    },
                    else => unreachable, // should this be an error?
                };
                break :blk .{ code, code_len };
            },
            .utf16le, .utf16be => blk: {
                const endian: Endian = if (encoding == .utf16le) .little else .big;

                const first = r.takeInt(u16, endian) catch return null;
                if (!Codepoint.isSurrogate(first)) break :blk .{ .{ .code = first }, 2 };

                const low_surrogate = r.takeInt(u16, endian) catch return DecodeError.InputTooShort;
                assert(Codepoint.isSurrogate(low_surrogate));

                const first_term = (first - 0xD800) * 0x400;
                const second_term = low_surrogate - 0xDC00; // error instead of panic when low_surrogate < 0xDC00?

                break :blk .{ .{ .code = @as(u21, first_term + second_term) + 0x1_0000 }, 4 };
            },
            .utf32le, .utf32be => blk: {
                const endian: Endian = if (encoding == .utf32le) .little else .big;

                break :blk .{ .{ .code = @intCast(r.takeInt(u32, endian) catch return null) }, 4 };
            },
        };
    }

    pub fn fromCodepointStr(str: []const u8) !Codepoint {
        const s = if (str.len >= 2 and std.ascii.eqlIgnoreCase(str[0..2], "U+")) str[2..] else str;
        if (s.len > 6) return error.CodepointStrTooLong;
        return .{ .code = try std.fmt.parseInt(u21, s, 16) };
    }

    pub const replacement_character: Codepoint = .{ .code = 0xFFFD };
};

pub fn main(init: std.process.Init) !void {
    const args = try init.minimal.args.toSlice(init.arena.allocator());

    var write_buffer: [512]u8 = undefined;
    var stdout_writer = Io.File.stdout().writer(init.io, &write_buffer);
    const stdout = &stdout_writer.interface;

    var read_buffer: [512]u8 = undefined;
    var stdin_reader = Io.File.stdin().reader(init.io, &read_buffer);
    const stdin = &stdin_reader.interface;

    var from_encoding: Codepoint.Encoding = .utf8;
    var to_encoding: Codepoint.Encoding = .utf8;
    for (1.., args[1..]) |i, s| {
        if (std.mem.eql(u8, s, "--from")) {
            if (i == args.len - 1) {
                std.log.err("invalid arguments, not setting '--from'", .{});
                continue;
            }

            from_encoding = std.meta.stringToEnum(Codepoint.Encoding, args[i + 1]) orelse {
                std.log.err("invalid arguments, not setting '--from'", .{});
                continue;
            };
        }
        if (std.mem.eql(u8, s, "--to")) {
            if (i == args.len - 1) {
                std.log.err("invalid arguments, not setting '--to'", .{});
                continue;
            }

            to_encoding = std.meta.stringToEnum(Codepoint.Encoding, args[i + 1]) orelse {
                std.log.err("invalid arguments, not setting '--to'", .{});
                continue;
            };
        }
    }

    switch (if (args.len > 1) args[1][0] else 0) {
        // decode the string of bytes from stdin into a list of space-terminated
        // codepoints with U+ prefixes
        'd' => while (true) {
            const codepoint: Codepoint, _ = Codepoint.decodeOne(from_encoding, stdin) catch {
                std.log.err("input ended in the middle of a UTF-8 byte sequence", .{});
                break;
            } orelse break;

            try stdout.print("U+{X} ", .{codepoint.code});
        },
        // read from stdin and encode each codepoint string to stdout
        'e' => while (true) {
            const codepoint_str = stdin.takeDelimiter(' ') catch break;
            if (codepoint_str == null) break;
            _ = try Codepoint.encodeOne(try .fromCodepointStr(codepoint_str.?), to_encoding, stdout);
        },
        // decode from stdin, encode to stdout
        'c' => if (to_encoding == from_encoding) {
            _ = try stdin.streamRemaining(stdout);
        } else while (true) {
            const codepoint: Codepoint, _ = Codepoint.decodeOne(from_encoding, stdin) catch {
                std.log.err("input ended in the middle of a UTF-8 byte sequence", .{});
                break;
            } orelse break;

            _ = try codepoint.encodeOne(to_encoding, stdout);
        },
        else => {
            std.log.err("only 'd' (decode), 'e' (encode), and 'c' (convert) actions are allowed", .{});
            std.process.exit(1);
        },
    }

    try stdout.flush();
}
