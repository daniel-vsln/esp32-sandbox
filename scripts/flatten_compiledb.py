# Розгортає у compile_commands.json пару `-iprefix <dir> @<includes>` фреймворка
# в явні -I<dir><path>. clangd, підбираючи прапори для файлів, яких немає в базі
# (тести), губить ці шляхи, і FreeRTOS та решта SDK не знаходяться.
import json
import os
import shlex

from SCons.Script import COMMAND_LINE_TARGETS

Import("env")


def expand_prefixed_includes(args):
    result = []
    prefix = None
    for arg in args:
        if arg == "-iprefix":
            prefix = ""
        elif prefix == "":
            prefix = arg
        elif prefix and arg.startswith("@") and os.path.isfile(arg[1:]):
            with open(arg[1:], encoding="utf-8") as f:
                tokens = shlex.split(f.read())
            for flag, path in zip(tokens[::2], tokens[1::2]):
                if flag == "-iwithprefixbefore":
                    result.append("-I" + os.path.join(prefix, path))
                else:
                    result.extend([flag, path])
        else:
            result.append(arg)
    return result


def flatten(target, source, env):
    path = env.subst("$COMPILATIONDB_PATH")
    with open(path, encoding="utf-8") as f:
        entries = json.load(f)

    for entry in entries:
        entry["command"] = shlex.join(
            expand_prefixed_includes(shlex.split(entry["command"]))
        )

    with open(path, "w", encoding="utf-8") as f:
        json.dump(entries, f, indent=2)


if "compiledb" in COMMAND_LINE_TARGETS:
    env.AddPostAction("compiledb", flatten)
