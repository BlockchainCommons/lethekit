# source: https://github.com/diybitcoinhardware/secp256k1-embedded
# build script that creates aliases for libwally files
# properly structured in scr to work with Arduino build system
import os

# where all the files are
ROOT = "libwally-core"
# where aliases will be placed
TARGET = "src"

alias_blacklist = [
    "wally_elements.h",
    "elements.c",
    "scrypt.c",
    "test.c",
    "run.c",
    "bench.c",
]

def create_alias(in_path, out_path, fname):
    # how many ../ we need
    if fname in alias_blacklist:
        return False
    level = len([d for d in out_path.split("/") if len(d)>0 and d!="."])
    prefix = "../"*level
    with open(os.path.join(out_path,fname), "w") as f:
        f.write("#include \"wally_config.h\"\n")
        f.write("#include \"%s\"" % os.path.join(prefix, in_path, fname))
    return True

global_whitelist = [
    "secp256k1.h",
    "secp256k1_preallocated.h",
    "secp256k1_recovery.h",
    "secp256k1_ecdh.h",
]

def global_import(in_path, out_path, fname):
    if fname not in global_whitelist:
        return False
    with open(os.path.join(out_path, fname), "w") as f:
        f.write("#include <%s>" % fname)
    return True

recursive_ignore = [
    "test",
]

rules = [
    # (initial folder, dest folder, file extensions, content rule, recursive?)

    # wally will be importing from here
    ("include", "include", ["h", "hpp"], create_alias, False),
    # for library imports in IDE
    ("include", "",        ["h", "hpp"], create_alias, False),
    ("src/secp256k1/include", "secp256k1/include", ["h"], global_import, False), 
    ("src",     "",        ["c", "h", "inl"], create_alias, False),
    ("src/data/wordlists", "data/wordlists", ["c"], create_alias, False),
    # ("src/ctaes", "ctaes", ["c", "h"], create_alias, False),
    ("src/ccan/ccan", "ccan/ccan", ["c", "h"], create_alias, True),
    ("src/ccan/ccan", "ccan", ["h"], create_alias, True),
]

WALLY_CONFIG = """
#ifndef LIBWALLYCORE_CONFIG_H
#define LIBWALLYCORE_CONFIG_H
#include "ccan_config.h"
#endif /*LIBWALLYCORE_CONFIG_H*/        
"""

def patch():
    # patches where it didn't work

    with open(os.path.join(TARGET, "wally_config.h"), "w") as f:
        f.write(WALLY_CONFIG)

    # encodings and config file
    with open(os.path.join(TARGET, "config.h"), "w") as f:
        f.write("#include \"wally_config.h\"")
    for fname in ["ccan/endian/endian.h", "ccan/ccan/endian/endian.h"]:
        fname = os.path.join(TARGET, fname)
        with open(fname, "r") as f:
            content = f.read()
        with open(fname, "w") as f:
            f.write("#include \"wally_config.h\"\n")
            f.write(content)

def main():
    for in_path, out_path, extensions, fn, recursive in rules:
        in_path = os.path.join(ROOT, in_path)
        out_path = os.path.join(TARGET, out_path)
        try:
            os.makedirs(out_path)
            print("Created:", out_path)
        except:
            print("Path already exists:", out_path)
        if not recursive:
            files = [f for f in os.listdir(in_path) 
                       if  f.split(".")[-1] in extensions]
            for f in files:
                if fn(in_path, out_path, f):
                    print("created", os.path.join(out_path, f))
                else:
                    print("skip", os.path.join(out_path, f))
        else:
            for parent, dirs, files in os.walk(in_path):
                if parent.split("/")[-1] in recursive_ignore:
                    continue
                files = [f for f in files
                           if  f.split(".")[-1] in extensions]
                if len(files) == 0:
                    continue
                out = parent.replace(in_path, out_path, 1)
                try:
                    os.makedirs(out)
                    print("Created:", out)
                except:
                    print("Path already exists:", out)

                for f in files:
                    if fn(parent, out, f):
                        print("created", os.path.join(out, f))
                    else:
                        print("skip", os.path.join(out, f))
    patch()

if __name__ == '__main__':
    main()
