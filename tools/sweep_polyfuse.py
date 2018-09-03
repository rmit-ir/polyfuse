#!/usr/bin/env python3
import argparse
import itertools
import os
import subprocess
import sys
from pathlib import Path
from typing import Any, List, Sequence


def eprint(*args: Any, **kwargs: Any) -> None:
    print(*args, **kwargs, file=sys.stderr, flush=True)  # type: ignore


def run_polyfuse(poly_args: Sequence[str]) -> str:
    eprint(" ".join(poly_args))
    proc = subprocess.run(poly_args, stdout=subprocess.PIPE)
    output: str = proc.stdout.decode("utf-8")
    return output


def write_output(directory: str, filename: str, content: str) -> None:
    path = Path(directory) / Path(filename)
    with open(path, "w") as f:
        f.write(content)


def sweep_rrf(args: argparse.Namespace) -> None:
    for d, k in itertools.product(args.depth, args.rrf_k):
        output = run_polyfuse([args.prog, "rrf", "-d", d, "-k", k] + args.run)
        write_output(
            args.output_dir,
            "{fusion}_depth:{depth}_k:{k}.run".format(fusion="rrf", depth=d, k=k),
            output,
        )


def sweep_rbc(args: argparse.Namespace) -> None:
    for d, p in itertools.product(args.depth, args.rbc_p):
        output = run_polyfuse([args.prog, "rbc", "-d", d, "-p", p] + args.run)
        write_output(
            args.output_dir,
            "{fusion}_depth:{depth}_p:{p}.run".format(fusion="rbc", depth=d, p=p),
            output,
        )


def sweep_comb(fusion: str, args: argparse.Namespace) -> None:
    for d, norm in itertools.product(args.depth, args.score_norm):
        output = run_polyfuse([args.prog, fusion, "-d", d, "-n", norm] + args.run)
        write_output(
            args.output_dir,
            "{fusion}_depth:{depth}_norm:{norm}.run".format(
                fusion=fusion, depth=d, norm=norm
            ),
            output,
        )


def sweep_default(fusion: str, args: argparse.Namespace) -> None:
    for d in args.depth:
        output = run_polyfuse([args.prog, fusion, "-d", d] + args.run)
        write_output(
            args.output_dir,
            "{fusion}_depth:{depth}.run".format(fusion=fusion, depth=d),
            output,
        )


def float_list(input_: str) -> List[str]:
    s = input_.split(",")
    try:
        all([float(e) for e in s])
    except ValueError:
        raise argparse.ArgumentTypeError("Please type floating numbers.")
    return s


def int_list(input_: str) -> List[str]:
    s = input_.split(",")
    try:
        all([int(e) for e in s])
    except ValueError:
        raise argparse.ArgumentTypeError("Please type integers.")
    return s


def fusion_list(input_: str) -> List[str]:
    valid = [
        "borda",
        "combanz",
        "combmax",
        "combmed",
        "combmin",
        "combmnz",
        "combsum",
        "isr",
        "logisr",
        "rbc",
        "rrf",
    ]
    s = input_.split(",")
    for f in s:
        if f not in valid:
            raise argparse.ArgumentTypeError("Invalid fusion method {}.".format(f))
    return s


def norm_list(input_: str) -> List[str]:
    valid = ["minmax", "std", "sum", "minsum"]
    s = input_.split(",")
    for norm in s:
        if norm not in valid:
            raise argparse.ArgumentTypeError("Invalid norm method {}.".format(norm))
    return s


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Sweep polyfuse fusion methods.")

    default_polyfuse = Path(__file__).parent.with_name("polyfuse").resolve()
    parser.add_argument(
        "-g",
        "--prog",
        default=default_polyfuse,
        help="polyfuse executable path, default: {}".format(default_polyfuse),
    )

    parser.add_argument(
        "-d",
        "--depth",
        type=int_list,
        default="100,1000",
        help="Comma separated fusion depths, default: 100,1000",
    )

    parser.add_argument(
        "-f",
        "--fusion",
        type=fusion_list,
        default="borda,combanz,combmax,combmed,"
        "combmin,combmnz,combsum,isr,logisr,rbc,rrf",
        help="Comma separated fusion methods, default: "
        "borda,combanz,combmax,combmed,"
        "combmin,combmnz,combsum,isr,logisr,rbc,rrf",
    )
    parser.add_argument(
        "-n",
        "--score-norm",
        type=norm_list,
        default="minmax,std,sum,minsum",
        help="Comma separated norm methods, valid for score-based methods, "
        "default: minmax,std,sum,minsum",
    )
    parser.add_argument(
        "-p",
        "--rbc-p",
        type=float_list,
        default="0.0,0.1,0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9,1.0",
        help="Comma separated rbc options, default: "
        "0.0,0.1,0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9,1.0",
    )
    parser.add_argument(
        "-k",
        "--rrf-k",
        type=int_list,
        default="10,60,100,600",
        help="Comma separated rrf options, default: 10,60,100,600",
    )

    parser.add_argument(
        "-o",
        "--output-dir",
        default="fusion_output",
        help="Output dir, default: fusion_output/",
    )

    parser.add_argument("run", nargs="+", help="run files")

    args = parser.parse_args()

    args.prog = str(args.prog)

    return args


def print_args(args: argparse.Namespace) -> None:
    max_len = len(max(vars(args).keys(), key=len))
    for (k, v) in vars(args).items():
        eprint("{0:{width}}: {1}".format(k, v, width=max_len + 1))


def main() -> None:
    args = parse_args()
    print_args(args)
    if not os.path.exists(args.output_dir):
        os.makedirs(args.output_dir)

    for fusion in args.fusion:
        if fusion == "rrf":
            sweep_rrf(args)
        elif fusion == "rbc":
            sweep_rbc(args)
        elif fusion.startswith("comb"):
            sweep_comb(fusion, args)
        else:
            sweep_default(fusion, args)

    eprint("Check {} for output files.".format(args.output_dir))


if __name__ == "__main__":
    main()
