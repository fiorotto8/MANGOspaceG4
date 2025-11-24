#!/usr/bin/env python3
import argparse
import glob
import os

import numpy as np


def compute_integrated_flux(energy, flux):
    """
    Compute energy-integrated flux F = ∫ Phi(E) dE using trapezoidal rule.
    energy in MeV, flux in particles cm^-2 s^-1 sr^-1 MeV^-1
    Result units: particles cm^-2 s^-1 sr^-1
    """
    return np.trapz(flux, energy)


def main():
    parser = argparse.ArgumentParser(
        description=(
            "Compute integrated flux and relative weights for all *.csv "
            "spectra in a folder. Each CSV must have: "
            "energy[MeV], flux[particles cm^-2 s^-1 sr^-1 MeV^-1]"
        )
    )
    parser.add_argument(
        "--folder",
        type=str,
        help="Folder containing {component}.csv files",
        default="./"
    )
    parser.add_argument(
        "--out",
        type=str,
        default="component_weights.txt",
        help="Output CSV file with weights (default: component_weights.csv)",
    )
    parser.add_argument(
        "--emin",
        type=float,
        default=None,
        help="Optional minimum energy [MeV] to integrate from",
    )
    parser.add_argument(
        "--emax",
        type=float,
        default=None,
        help="Optional maximum energy [MeV] to integrate to",
    )

    args = parser.parse_args()

    pattern = os.path.join(args.folder, "*.csv")
    files = sorted(glob.glob(pattern))

    if not files:
        raise SystemExit(f"No .csv files found in folder: {args.folder}")

    components = []
    integrated_fluxes = []

    for path in files:
        comp_name = os.path.splitext(os.path.basename(path))[0]

        try:
            data = np.loadtxt(
                path,
                delimiter=",",
                comments="#",
                usecols=(0, 1),
            )
        except Exception as e:
            print(f"[WARNING] Could not read {path}: {e}")
            continue

        if data.ndim == 1:
            # Single line case
            data = data.reshape(1, -1)

        E = data[:, 0]  # MeV
        Phi = data[:, 1]  # particles cm^-2 s^-1 sr^-1 MeV^-1

        # Optional energy range cut
        if args.emin is not None:
            mask = E >= args.emin
            E = E[mask]
            Phi = Phi[mask]
        if args.emax is not None:
            mask = E <= args.emax
            E = E[mask]
            Phi = Phi[mask]

        if len(E) < 2:
            print(f"[WARNING] Not enough points in {path} after cuts; skipping.")
            continue

        F = compute_integrated_flux(E, Phi)

        components.append(comp_name)
        integrated_fluxes.append(F)

        print(f"{comp_name}: F = {F:.6e} [particles cm^-2 s^-1 sr^-1]")

    if not components:
        raise SystemExit("No valid spectra processed; nothing to write.")

    integrated_fluxes = np.array(integrated_fluxes)
    F_total = np.sum(integrated_fluxes)

    if F_total <= 0:
        raise SystemExit("Total integrated flux is non-positive; check spectra.")

    # Normalized weights: if you simulate the SAME number of events for each
    # component, these are the per-event weights to apply when combining.
    weights_norm = integrated_fluxes / F_total

    # Write output CSV
    out_path = args.out
    with open(out_path, "w") as f:
        f.write(
            "#component,integrated_flux[particles cm^-2 s^-1 sr^-1],"
            "weight_norm\n"
        )
        for name, F, w in zip(components, integrated_fluxes, weights_norm):
            f.write(f"{name},{F:.8e},{w:.8e}\n")

    print(f"\nWrote weights to: {out_path}")
    print("Interpretation:")
    print(" - integrated_flux: F_j = ∫ Phi_j(E) dE")
    print(" - weight_norm: per-event weight to use if you simulate the same")
    print("   number of particles for each component (relative contributions "
          "follow F_j).")


if __name__ == "__main__":
    main()
