const std = @import("std");

const FLASH_BEGIN_ADDRESS = 0x10_00_00_00;

// taken from https://microsoft.github.io/uf2/
const UF2Block = extern struct {
    magic_start_0: u32 = 0x0A_32_46_55,
    magic_start_1: u32 = 0x9E_5D_51_57,
    flags: u32 = FAMILY_ID_FLAG, // family ID present
    target_addr: u32 = FLASH_BEGIN_ADDRESS,
    payload_size: u32 = PAYLOAD_SIZE,
    no: u32 = 0,
    num_blocks: u32,
    family_id: u32 = RP2040_FAMILY_ID,
    data: [476]u8 = .{0x00} ** 476,
    magicEnd: u32 = 0x0A_B1_6F_30,  

    const FAMILY_ID_FLAG   = 0x00_00_20_00;
    const RP2040_FAMILY_ID = 0xE4_8B_FF_56;
    
    pub const PAYLOAD_SIZE = 256;
};

fn readBin(bin_file_path: []const u8, buffer: []u8) !usize {
    const bin_file = std.fs.cwd().openFile(
        bin_file_path,
        .{
            .mode = .read_only
        }
    ) catch |err| {
        std.log.err("(bin2uf2) failed to open file {s}", .{bin_file_path});
        return err;
    };
    defer bin_file.close();

    return bin_file.read(buffer) catch |err| {
        std.log.err("(bin2uf2) failed to read data from file {s}", .{bin_file_path});
        return err;
    };
}

const Bin2Uf2Error = error {
    BinFileExceedsFlashCapacity,
};

pub fn main() !void {
    var arg_iter = std.process.args();
    _ = arg_iter.next(); // omit exe name
    const in_file = arg_iter.next();
    const out_option = arg_iter.next();
    const out_file = arg_iter.next();

    if (in_file    == null or std.mem.eql(u8, in_file.?, "-h")     or
        out_option == null or !std.mem.eql(u8, out_option.?, "-o") or
        out_file   == null) {
        std.log.info("(bin2uf2) [usage] bin2uf2 <in_file.bin> -o <out_file.uf2>", .{});
        return;
    }   

    const bin_file_path: []const u8 = in_file.?;
    const uf2_file_path: []const u8 = out_file.?;

    const MAX_FILE_SIZE = 16 * 1024 * 1024; // 16MB

    const allocator = std.heap.page_allocator;
    var data = allocator.alloc(u8, MAX_FILE_SIZE) catch |err| {
        std.log.err("(bin2uf2) failed to alloc {}", .{std.fmt.fmtIntSizeBin(MAX_FILE_SIZE)});
        return err;
    };
    defer allocator.free(data);

    std.mem.set(u8, data, 0xFF);

    var read_size = try readBin(bin_file_path, data);
    if (read_size >= MAX_FILE_SIZE) {
        std.log.err("(bin2uf2) bin file {s} size exceeds {}", .{bin_file_path, std.fmt.fmtIntSizeBin(MAX_FILE_SIZE)});
        return error.BinFileExceedsFlashCapacity;
    }

    // align to payload size
    read_size += UF2Block.PAYLOAD_SIZE - (read_size % UF2Block.PAYLOAD_SIZE);
    
    // compute crc for first block containing flash second stage
    //  check sum parameters rp2040 docs 2.8.1.3.1:
    //      polynomial       = 0x04C11DB7
    //      initial value    = 0xFFFFFFFF
    //      final xor        = 0x00000000
    // set crc at the end of the block (little-endian)

    const Crc32RP2040 = std.hash.crc.Crc(u32, .{
        .polynomial = 0x04_C1_1D_B7,
        .initial = 0xFF_FF_FF_FF,
        .reflect_input = false,
        .reflect_output = false,
        .xor_output = 0x0,
    });
    const crc_value = Crc32RP2040.hash(data[0..(UF2Block.PAYLOAD_SIZE-4)]);
    std.mem.copy(
        u8, 
        data[(UF2Block.PAYLOAD_SIZE-4)..(UF2Block.PAYLOAD_SIZE)], 
        std.mem.asBytes(&crc_value)
    );

    const uf2_file = std.fs.cwd().createFile(
        uf2_file_path,
        .{
            .read = false,
            .truncate = true,
        }
    ) catch |err| {
        std.log.err("(bin2uf2) failed to create file {s}", .{uf2_file_path});
        return err;
    };
    defer uf2_file.close();

    var block = UF2Block{
        .num_blocks = @intCast(u32, read_size / UF2Block.PAYLOAD_SIZE)
    };
    while (block.no < block.num_blocks) : (block.no += 1) {
        std.mem.copy(
            u8,
            block.data[0..UF2Block.PAYLOAD_SIZE],
            data[(block.no*UF2Block.PAYLOAD_SIZE)..((block.no+1)*UF2Block.PAYLOAD_SIZE)]
        );

        _ = uf2_file.write(std.mem.asBytes(&block)) catch |err| {
            std.log.err("(bin2uf2) failed to write {} UF2 block into {s} file",.{
                block.no,
                uf2_file_path,
            });
            return err;
        }; 
        
        block.target_addr += UF2Block.PAYLOAD_SIZE;
    }
}