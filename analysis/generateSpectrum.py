#!/usr/bin/env python3
import argparse
import math

HEADER = (
    "#bin\tbin_low_keV\tbin_center_keV\tbin_high_keV\tcounts\terror\tpolarization\tpolarization_error\n"
)

# --- Reference X‑ray line energies (keV) and simple relative intensities ---
# Values are approximate (room‑temperature lab references), easy to tweak.
# Intensities are relative weights used to split a total counts budget.
XRAY_LINES = {
    # Copper
    "Cu": {
        "Kα1": (8.0478, 1.00),
        "Kα2": (8.0278, 0.50),
        "Kβ1": (8.9053, 0.17),
    },
    # Manganese
    "Mn": {
        "Kα1": (5.8988, 1.00),
        "Kα2": (5.8877, 0.50),
        "Kβ1": (6.4904, 0.16),
    },
    # Rubidium
    "Rb": {
        "Kα1": (13.395, 1.00),
        "Kα2": (13.373, 0.50),
        "Kβ1": (14.961, 0.15),
    },
    # Molybdenum
    "Mo": {
        "Kα1": (17.479, 1.00),
        "Kα2": (17.374, 0.50),
        "Kβ1": (19.608, 0.16),
    },
    # Silver
    "Ag": {
        "Kα1": (22.163, 1.00),
        "Kα2": (21.990, 0.50),
        "Kβ1": (24.942, 0.15),
    },
}


def make_uniform_spectrum(e_min, e_max, nbins, counts_per_bin, pol=0.0, pol_err=0.0):
    if e_max <= e_min:
        raise ValueError("e_max must be > e_min")
    de = (e_max - e_min) / nbins
    rows = []
    for i in range(nbins):
        low = e_min + i * de
        high = low + de
        center = 0.5 * (low + high)
        rows.append(
            {
                "bin": i + 1,
                "low": low,
                "center": center,
                "high": high,
                "counts": counts_per_bin,
                "error": 0.0,
                "pol": pol,
                "pol_err": pol_err,
            }
        )
    return rows


def make_monochromatic(energy_keV, counts, pol=0.0, pol_err=0.0):
    width = 1e-3  # ~1 eV
    low = energy_keV - 0.5 * width
    high = energy_keV + 0.5 * width
    center = energy_keV
    return [
        {
            "bin": 1,
            "low": low,
            "center": center,
            "high": high,
            "counts": counts,
            "error": 0.0,
            "pol": pol,
            "pol_err": pol_err,
        }
    ]


def _gaussian_area_per_bin(x, mu, sigma, bin_width):
    # density at bin center times width; good enough for narrow bins
    return (1.0 / (math.sqrt(2.0 * math.pi) * sigma)) * math.exp(-0.5 * ((x - mu) / sigma) ** 2) * bin_width


def make_lines_spectrum(
    elements,  # list of element symbols like ["Cu", "Mn"]
    total_counts=1e5,
    include_kalpha2=True,
    include_kbeta=True,
    fwhm_keV=0.0,  # if >0, discretize each line as a Gaussian with this FWHM
    step_keV=0.001,  # bin width when fwhm_keV > 0
    pol=0.0,
    pol_err=0.0,
):
    rows = []
    bin_idx = 1

    # collect all selected lines with energies and relative weights
    selected = []
    for el in elements:
        if el not in XRAY_LINES:
            raise ValueError(f"Unknown element '{el}'. Known: {sorted(XRAY_LINES.keys())}")
        for name, (E, rel) in XRAY_LINES[el].items():
            if ("α2" in name and not include_kalpha2) or ("β" in name and not include_kbeta):
                continue
            selected.append((f"{el} {name}", E, rel))

    if not selected:
        raise ValueError("No lines selected with the given options.")

    # normalize relative intensities to 1
    rel_sum = sum(rel for _, _, rel in selected)
    norm = [(label, E, rel / rel_sum) for (label, E, rel) in selected]

    if fwhm_keV <= 0:
        # Use very narrow bins (approx. delta functions)
        width = 1e-3
        for label, E, frac in norm:
            counts = total_counts * frac
            low = E - 0.5 * width
            high = E + 0.5 * width
            rows.append(
                {
                    "bin": bin_idx,
                    "low": low,
                    "center": E,
                    "high": high,
                    "counts": counts,
                    "error": 0.0,
                    "pol": pol,
                    "pol_err": pol_err,
                }
            )
            bin_idx += 1
    else:
        # Discretize each line as a Gaussian with area = total_counts * frac
        sigma = fwhm_keV / 2.354820045
        for label, E, frac in norm:
            area = total_counts * frac
            xmin = E - 4.0 * sigma
            xmax = E + 4.0 * sigma
            x = xmin
            while x < xmax:
                low = x
                high = min(x + step_keV, xmax)
                center = 0.5 * (low + high)
                bin_area = _gaussian_area_per_bin(center, E, sigma, high - low)
                rows.append(
                    {
                        "bin": bin_idx,
                        "low": low,
                        "center": center,
                        "high": high,
                        "counts": area * bin_area,  # area fraction
                        "error": 0.0,
                        "pol": pol,
                        "pol_err": pol_err,
                    }
                )
                bin_idx += 1
                x += step_keV

    return rows


def write_spectrum(rows, outfile):
    with open(outfile, "w") as f:
        f.write(HEADER)
        for r in rows:
            f.write(
                f"{r['bin']}\t{r['low']:.6f}\t{r['center']:.6f}\t{r['high']:.6f}\t{r['counts']:.6f}\t{r['error']:.6f}\t{r['pol']:.6f}\t{r['pol_err']:.6f}\n"
            )


def parse_args():
    p = argparse.ArgumentParser(description="Generate spectrum files for a Geant4 primary generator.")
    sub = p.add_subparsers(dest="mode", required=True)

    # uniform
    p_uni = sub.add_parser("uniform", help="uniform spectrum in a range")
    p_uni.add_argument("--emin", type=float, required=True, help="min energy in keV")
    p_uni.add_argument("--emax", type=float, required=True, help="max energy in keV")
    p_uni.add_argument("--nbins", type=int, default=100, help="number of bins (default: 100)")
    p_uni.add_argument("--counts", type=float, default=1.0, help="counts per bin (default: 1.0)")
    p_uni.add_argument("--pol", type=float, default=0.0, help="polarization mean for all bins (0..1)")
    p_uni.add_argument("--polerr", type=float, default=0.0, help="polarization sigma for all bins")
    p_uni.add_argument("--out", type=str, default="spectrum.txt", help="output file")

    # mono
    p_mono = sub.add_parser("mono", help="monochromatic line")
    p_mono.add_argument("--energy", type=float, required=True, help="energy in keV")
    p_mono.add_argument("--counts", type=float, default=1.0, help="counts (weight) for this line")
    p_mono.add_argument("--pol", type=float, default=0.0, help="polarization mean (0..1)")
    p_mono.add_argument("--polerr", type=float, default=0.0, help="polarization sigma")
    p_mono.add_argument("--out", type=str, default="spectrum.txt", help="output file")

    # lines (new)
    p_lines = sub.add_parser(
        "lines",
        help=(
            "custom spectrum from known X-ray emission lines. "
            "Supports Cu, Mn, Rb, Mo out of the box."
        ),
    )
    p_lines.add_argument(
        "--elements",
        type=str,
        required=True,
        help="comma-separated list of element symbols (e.g. Cu,Mn,Rb,Mo)",
    )
    p_lines.add_argument(
        "--totalcounts",
        type=float,
        default=1e5,
        help="total counts distributed according to relative line intensities (default: 1e5)",
    )
    p_lines.add_argument(
        "--no-kalpha2",
        action="store_true",
        help="exclude Kα2 lines",
    )
    p_lines.add_argument(
        "--no-kbeta",
        action="store_true",
        help="exclude Kβ lines",
    )
    p_lines.add_argument(
        "--fwhm",
        type=float,
        default=0.0,
        help="Gaussian FWHM (keV). If 0, emit delta-like narrow bins",
    )
    p_lines.add_argument(
        "--step",
        type=float,
        default=0.001,
        help="bin width (keV) used when --fwhm > 0 (default: 0.001 keV)",
    )
    p_lines.add_argument("--pol", type=float, default=0.0, help="polarization mean (0..1)")
    p_lines.add_argument("--polerr", type=float, default=0.0, help="polarization sigma")
    p_lines.add_argument("--out", type=str, default="spectrum.txt", help="output file")

    return p.parse_args()


def main():
    args = parse_args()

    if args.mode == "uniform":
        rows = make_uniform_spectrum(
            e_min=args.emin,
            e_max=args.emax,
            nbins=args.nbins,
            counts_per_bin=args.counts,
            pol=args.pol,
            pol_err=args.polerr,
        )
    elif args.mode == "mono":
        rows = make_monochromatic(
            energy_keV=args.energy, counts=args.counts, pol=args.pol, pol_err=args.polerr
        )
    elif args.mode == "lines":
        elements = [s.strip() for s in args.elements.split(",") if s.strip()]
        rows = make_lines_spectrum(
            elements=elements,
            total_counts=args.totalcounts,
            include_kalpha2=not args.no_kalpha2,
            include_kbeta=not args.no_kbeta,
            fwhm_keV=args.fwhm,
            step_keV=args.step,
            pol=args.pol,
            pol_err=args.polerr,
        )
    else:
        raise SystemExit("unknown mode")

    write_spectrum(rows, args.out)


if __name__ == "__main__":
    main()
