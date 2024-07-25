#!/usr/bin/env python

from glob import glob
import io
from rich import print
from ninja_syntax import Writer
import os
import subprocess


def info(message, *args):
    print(f"[blue][INFO][/blue]: {message}", *args)


def ok(message, *args):
    print(f"[green][ OK ][/green]: {message}", *args)


def warn(message, *args):
    print(f"[yellow][WARN][/yellow]: {message}", *args)


def fail(message, *args):
    print(f"[red][FAIL][/red]: {message}", *args)
    exit(1)


DEVKITPRO = os.environ.get("DEVKITPRO")
DEVKITPPC = os.environ.get("DEVKITPPC")

if not DEVKITPRO or not DEVKITPPC:
    fail("DEVKITPRO or DEVKITPPC environment variables are not set.")

libProcess = subprocess.run(
    [
        f"{DEVKITPRO}/portlibs/ppc/bin/powerpc-eabi-pkg-config",
        "--libs",
        "freetype2",
        "libpng",
        "libjpeg",
    ],
    capture_output=True,
    text=True,
)

if libProcess.returncode != 0:
    fail("Could not find required libraries")

CFLAGS = [
    "-g",
    "-O2",
    "-Wall",
    "-DGEKKO",
    "-DHW_RVL",
    "-mrvl",
    "-mcpu=750",
    "-meabi",
    "-mhard-float",
    "-Iinclude",
    f"-I{DEVKITPRO}/libogc/include",
    f"-I{DEVKITPRO}/portlibs/ppc/include",
    f"-I{DEVKITPRO}/portlibs/ppc/include/freetype2",
    f"-I{DEVKITPRO}/portlibs/wii/include",
]
CXXFLAGS = [
    "--std=c++17",
    "-g",
    "-O2",
    "-Wall",
    "-DGEKKO",
    "-DHW_RVL",
    "-mrvl",
    "-mcpu=750",
    "-meabi",
    "-mhard-float",
    "-Iinclude",
    f"-I{DEVKITPRO}/libogc/include",
    f"-I{DEVKITPRO}/portlibs/ppc/include",
    f"-I{DEVKITPRO}/portlibs/ppc/include/freetype2",
    f"-I{DEVKITPRO}/portlibs/wii/include",
]
LDFLAGS = [
    "-g",
    "-DGEKKO",
    "-mrvl",
    "-mcpu=750",
    "-meabi",
    "-mhard-float",
    "-Wl,--section-start,.init=0x80b00000",
    f"-L{DEVKITPRO}/libogc/lib/wii",
    f"-L{DEVKITPRO}/portlibs/wii/lib",
    "-lgrrlib",
    "-lpngu",
    libProcess.stdout.strip(),
    "-lfat",
    "-lwiiuse",
    "-lbte",
    "-logc",
    "-lm",
]


def create_ninja_base(writer: Writer):
    info("Creating Ninja base")

    writer.variable("builddir", "build")
    writer.variable("outdir", "dist")

    writer.newline()

    writer.variable("ccompiler", f"{DEVKITPPC}/bin/powerpc-eabi-gcc")
    writer.variable("cxxcompiler", f"{DEVKITPPC}/bin/powerpc-eabi-g++")
    writer.variable("elf2dol", f"{DEVKITPRO}/tools/bin/elf2dol")

    writer.newline()

    writer.rule("cc", "$ccompiler -MMD -MF $out.d $cflags -c $in -o $out")
    writer.rule("cxx", "$cxxcompiler -MMD -MF $out.d $cxxflags -c $in -o $out")
    writer.rule("link", "$cxxcompiler $in -o $out $ldflags -Wl,-Map=$out.map")
    writer.rule("elftodol", "$elf2dol $in $out")

    writer.newline()

    ok("Created Ninja base")


def create_build_rules(writer: Writer):
    info("Creating build rules")

    output_files = []

    for source_file in glob("src/**/*.c", recursive=True):
        object_file = f"build/{source_file.replace('.c', '.o').replace('src/', '')}"
        output_files.append(object_file)

        writer.build(
            object_file,
            "cc",
            source_file,
            variables={
                "cflags": " ".join(CFLAGS),
            },
        )

    for source_file in glob("src/**/*.cpp", recursive=True):
        object_file = f"$builddir/{source_file.replace('.cpp', '.o')}"
        output_files.append(object_file)

        writer.build(
            object_file,
            "cxx",
            source_file,
            variables={
                "cxxflags": " ".join(CXXFLAGS),
            },
        )

    writer.newline()

    return output_files


def create_link_rules(writer: Writer, output_files):
    info("Creating link rules")

    writer.build(
        "$outdir/boot.elf",
        "link",
        output_files,
        variables={
            "ldflags": " ".join(LDFLAGS),
        },
    )

    writer.build(
        "$outdir/boot.dol",
        "elftodol",
        "$outdir/boot.elf",
    )

    writer.newline()

    ok("Created link rules")


def main():
    outbuf = io.StringIO()
    writer = Writer(outbuf)

    create_ninja_base(writer)
    output_files = create_build_rules(writer)
    create_link_rules(writer, output_files)

    with open("build.ninja", "w") as f:
        f.write(outbuf.getvalue())

    ok("Created build.ninja")

    subprocess.run(["ninja"], check=True)


if __name__ == "__main__":
    main()
