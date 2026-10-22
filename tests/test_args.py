"""Run after make: python3 -m unittest discover -s tests -v."""

import ctypes
from pathlib import Path
import subprocess
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[1]
VALID = ["5", "2000", "200", "200", "200", "10", "0", "fifo"]
INT_MAX = (1 << (ctypes.sizeof(ctypes.c_int) * 8 - 1)) - 1
LONG_MAX = (1 << (ctypes.sizeof(ctypes.c_long) * 8 - 1)) - 1


class ArgumentTests(unittest.TestCase):
    def check_run(self, arguments, valid):
        result = subprocess.run(
            [str(ROOT / "codexion"), *arguments],
            capture_output=True, text=True, timeout=2,
        )
        self.assertEqual(result.returncode, 0 if valid else 1)
        self.assertEqual(result.stdout, "")
        if valid:
            self.assertEqual(result.stderr, "")
        else:
            self.assertTrue(result.stderr)

    def test_valid_schedulers(self):
        for scheduler in ("fifo", "edf"):
            with self.subTest(scheduler=scheduler):
                self.check_run(VALID[:-1] + [scheduler], True)

    def test_argument_count(self):
        for count in range(8):
            with self.subTest(count=count):
                self.check_run(VALID[:count], False)
        self.check_run(VALID + ["extra"], False)

    def test_invalid_numbers_in_every_position(self):
        for index in range(7):
            for value in ("", "-1", "+1", " 1", "1 ", "1.5", "abc", "1x",
                          "1\n", "9" * 100):
                with self.subTest(index=index, value=value):
                    arguments = VALID.copy()
                    arguments[index] = value
                    self.check_run(arguments, False)

    def test_numeric_boundaries(self):
        for index in range(7):
            limit = INT_MAX if index in (0, 5) else LONG_MAX
            for value, valid in ((str(limit), True), (str(limit + 1), False),
                                 ("0", index != 0)):
                with self.subTest(index=index, value=value):
                    arguments = VALID.copy()
                    arguments[index] = value
                    self.check_run(arguments, valid)
        self.check_run(["1", "0", "0", "0", "0", "0", "0", "edf"], True)

    def test_leading_zeros(self):
        self.check_run(["0005", *VALID[1:]], True)

    def test_invalid_schedulers(self):
        for scheduler in ("", "FIFO", "EDF", "lifo", "fifo ", " edf"):
            with self.subTest(scheduler=scheduler):
                self.check_run(VALID[:-1] + [scheduler], False)

    def test_parsed_fields(self):
        source = '''#include "codexion.h"
int main(int argc, char **argv)
{
    t_args args;
    if (!parse_args(argc, argv, &args))
        return (1);
    printf("%d %ld %ld %ld %ld %d %ld %d\\n", args.num_coders,
        args.time_to_burnout, args.time_to_compile, args.time_to_debug,
        args.time_to_refactor, args.num_compilations, args.dongle_cooldown,
        args.scheduler);
    return (0);
}
'''
        with tempfile.TemporaryDirectory() as directory:
            harness = Path(directory) / "parse_fields.c"
            executable = Path(directory) / "parse_fields"
            harness.write_text(source)
            subprocess.run(
                ["cc", "-Wall", "-Wextra", "-Werror", "-pthread",
                 "-I", str(ROOT), str(harness), str(ROOT / "args.c"),
                 "-o", str(executable)], check=True,
            )
            for scheduler, enum_value in (("fifo", 0), ("edf", 1)):
                with self.subTest(scheduler=scheduler):
                    result = subprocess.run(
                        [str(executable), "3", "901", "102", "203", "304",
                         "7", "405", scheduler],
                        capture_output=True, text=True, check=True, timeout=2,
                    )
                    self.assertEqual(
                        result.stdout, f"3 901 102 203 304 7 405 {enum_value}\n"
                    )


if __name__ == "__main__":
    unittest.main()
