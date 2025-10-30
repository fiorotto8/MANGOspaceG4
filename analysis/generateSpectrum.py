#!/usr/bin/env python3
import argparse
import sys
import math

def make_uniform_spectrum(e_min, e_max, nbins, counts_per_bin,
                        pol=0.0, pol_err=0.0):
    """
    Build a list of rows, each a dict with the required fields.
    Energies are in keV.
    """
    if e_max <= e_min:
        raise ValueError("e_max must be > e_min")

    de = (e_max - e_min) / nbins
    rows = []
    for i in range(nbins):
        low  = e_min + i * de
        high = low + de
        center = 0.5 * (low + high)
        row = {
            "bin": i + 1,
            "low": low,
            "center": center,
            "high": high,
            "counts": counts_per_bin,
            "error": 0.0,
            "pol": pol,
            "pol_err": pol_err,
        }
        rows.append(row)
    return rows


def make_monochromatic(energy_keV, counts,
                    pol=0.0, pol_err=0.0):
    """
    One bin, very narrow, centered on energy_keV.
    We still give it a tiny width so Geant4 won't crash on 0 width.
    """
    width = 1e-3  # 1 eV-ish
    low = energy_keV - 0.5 * width
    high = energy_keV + 0.5 * width
    center = energy_keV
    return [{
        "bin": 1,
        "low": low,
        "center": center,
        "high": high,
        "counts": counts,
        "error": 0.0,
        "pol": pol,
        "pol_err": pol_err,
    }]


def write_spectrum(rows, outfile):
    with open(outfile, "w") as f:
        f.write("#bin\tbin_low_keV\tbin_center_keV\tbin_high_keV\tcounts\terror\tpolarization\tpolarization_error\n")
        for r in rows:
            f.write(
                f"{r['bin']}\t"
                f"{r['low']:.6f}\t"
                f"{r['center']:.6f}\t"
                f"{r['high']:.6f}\t"
                f"{r['counts']:.6f}\t"
                f"{r['error']:.6f}\t"
                f"{r['pol']:.6f}\t"
                f"{r['pol_err']:.6f}\n"
            )


def main():
    parser = argparse.ArgumentParser(
        description="Generate a spectrum file for the Geant4 primary generator."
    )

    sub = parser.add_subparsers(dest="mode", required=True)

    # uniform mode
    p_uni = sub.add_parser("uniform", help="uniform spectrum in a range")
    p_uni.add_argument("--emin", type=float, required=True,
                    help="min energy in keV")
    p_uni.add_argument("--emax", type=float, required=True,
                    help="max energy in keV")
    p_uni.add_argument("--nbins", type=int, default=100,
                    help="number of bins (default: 100)")
    p_uni.add_argument("--counts", type=float, default=1.0,
                    help="counts per bin (default: 1.0)")
    p_uni.add_argument("--pol", type=float, default=0.0,
                    help="polarization mean for all bins (0..1)")
    p_uni.add_argument("--polerr", type=float, default=0.0,
                    help="polarization sigma for all bins")
    p_uni.add_argument("--out", type=str, default="spectrum.txt",
                    help="output file")

    # mono mode
    p_mono = sub.add_parser("mono", help="monochromatic line")
    p_mono.add_argument("--energy", type=float, required=True,
                        help="energy in keV")
    p_mono.add_argument("--counts", type=float, default=1.0,
                        help="counts (weight) for this line")
    p_mono.add_argument("--pol", type=float, default=0.0,
                        help="polarization mean (0..1)")
    p_mono.add_argument("--polerr", type=float, default=0.0,
                        help="polarization sigma")
    p_mono.add_argument("--out", type=str, default="spectrum.txt",
                        help="output file")

    args = parser.parse_args()

    if args.mode == "uniform":
        rows = make_uniform_spectrum(
            e_min=args.emin,
            e_max=args.emax,
            nbins=args.nbins,
            counts_per_bin=args.counts,
            pol=args.pol,
            pol_err=args.polerr,
        )
        write_spectrum(rows, args.out)
    elif args.mode == "mono":
        rows = make_monochromatic(
            energy_keV=args.energy,
            counts=args.counts,
            pol=args.pol,
            pol_err=args.polerr,
        )
        write_spectrum(rows, args.out)
    else:
        parser.error("unknown mode")


if __name__ == "__main__":
    main()
