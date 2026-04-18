const std = @import("std");
const Io = std.Io;

const ZigToolDir = struct {
    path: []u8,
    has_build_script: bool,
};

pub fn build(b: *std.Build) !void {
    var threaded_io: Io.Threaded = .init_single_threaded;
    defer threaded_io.deinit();
    const io = threaded_io.io();

    const optimize = b.standardOptimizeOption(.{});
    const target = b.standardTargetOptions(.{});
    const target_user_input_option = b.user_input_options.get("target");
    const target_str: ?[]const u8 = if (target_user_input_option) |tuio| tuio.value.scalar else null;

    const build_all_step = b.step("build-all", "build all zig tools");

    var cwd = try Io.Dir.cwd().openDir(io, ".", .{ .iterate = true });
    defer cwd.close(io);

    var it = cwd.iterate();

    var zig_tool_dirs: std.ArrayList(ZigToolDir) = .empty;
    defer zig_tool_dirs.deinit(b.allocator);
    defer for (zig_tool_dirs.items) |*tool| b.allocator.free(tool.path);

    while (try it.next(io)) |entry| {
        if (entry.kind != .directory) continue;

        var dir = cwd.openDir(io, entry.name, .{}) catch continue;
        defer dir.close(io);

        dir.access(io, "ZIG", .{}) catch continue;
        // dir contains a zig tool if that didn't error

        var has_build_script = true;
        dir.access(io, "build.zig", .{}) catch {
            has_build_script = false;
        };

        try zig_tool_dirs.append(b.allocator, .{
            .path = try b.allocator.dupe(u8, entry.name),
            .has_build_script = has_build_script,
        });
    }

    const absolute_build_dir = if (Io.Dir.path.isAbsolute(b.install_prefix))
        b.install_prefix
    else
        b.fmt("{s}/{s}", .{
            try cwd.realPathFileAlloc(io, ".", b.allocator),
            try Io.Dir.path.resolve(b.allocator, &[_][]const u8{b.install_prefix}),
        });

    for (zig_tool_dirs.items) |tool_dir| {
        if (tool_dir.has_build_script) {
            const optimize_arg = try std.fmt.allocPrint(b.allocator, "-Doptimize={s}", .{@tagName(optimize)});
            const target_arg = if (target_str) |ts| try std.fmt.allocPrint(b.allocator, "-Dtarget={s}", .{ts}) else null;

            const run_cmd = b.addSystemCommand(if (target_arg) |ta| &.{
                "zig",
                "build",
                "-p",
                absolute_build_dir,
                optimize_arg,
                ta,
            } else &.{
                "zig",
                "build",
                "-p",
                absolute_build_dir,
                optimize_arg,
            });
            run_cmd.setCwd(b.path(tool_dir.path));
            run_cmd.addCheck(.{ .expect_term = .{ .exited = 0 } });
            run_cmd.has_side_effects = true;

            build_all_step.dependOn(&run_cmd.step);
        } else {
            const new_exe = b.addExecutable(.{
                .name = tool_dir.path,
                .root_module = b.createModule(.{
                    .root_source_file = b.path(try std.fmt.allocPrint(b.allocator, "{s}/main.zig", .{tool_dir.path})),
                    .optimize = optimize,
                    .target = target,
                }),
            });
            installArtifactOptions(b, new_exe, .{ .dest_dir = .{ .override = .{ .prefix = {} } } });
        }
    }

    b.getInstallStep().dependOn(build_all_step);
}

pub fn installArtifactOptions(b: *std.Build, artifact: *std.Build.Step.Compile, options: std.Build.Step.InstallArtifact.Options) void {
    b.getInstallStep().dependOn(&b.addInstallArtifact(artifact, options).step);
}
