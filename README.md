# README – Using the Geant4 primary generator with external spectra

This Geant4 primary generator is designed so that **physics inputs come from files and macros**, while the C++ code only fixes the basic source geometry and logic.

Hard-coded in C++:

- **Fixed source geometry**: disk of radius **1.25 cm** at `z = -1 cm`, shooting along `+z`.
- **Sphere source geometry**: particles generated on a **sphere of radius R** around the detector, pointing inward (isotropic in 4π).

Configurable at runtime (via macro + files):

- **Particle type** (e.g. `gamma`, `proton`, `e-`, …)
- **Emission mode**:
  - `fixed` → disk beam at `z = -1 cm`
  - `sphere` → isotropic environment from a sphere
- **Sphere radius R** (in `sphere` mode)
- **Energy spectrum and polarization** (from external text / CSV files)

---

## 1. Choose the particle and emission mode (macro)

You do **not** need to edit C++ to change particle or emission. In your `.mac`:

```tcl
/B3/primary/particle gamma             # any Geant4 name: gamma, e-, proton, ...
/B3/primary/spectrumFile ../spectra/55Fe.txt

/B3/primary/emissionMode fixed         # or: sphere
/B3/primary/sphereRadius 100 cm        # only used in 'sphere' mode
```

The constructor sets some defaults, but the macro always overrides them.

---

## 2. Spectrum file formats

The code supports **two** formats and autodetects which one you use.

### (A) 8-column “line spectrum” (keV)

Typical file produced by `make_spectrum.py`:

```text
#bin  bin_low_keV  bin_center_keV  bin_high_keV  counts  error  polarization  polarization_error
1     5.890000     5.895000        5.900000      100.0   0.0    0.0           0.0
...
```

- `counts` = weight of the bin (higher → sampled more often)
- `polarization` = mean probability to be linearly polarized
- `polarization_error` = sigma of that probability

In this format, the **shape and normalization** of the spectrum are entirely controlled by the `counts` column.

### (B) 2-column flux CSV (MeV)

Background components (e.g. CXB) can be given as:

```text
E[MeV], Phi(E) [particles cm^-2 s^-1 sr^-1 MeV^-1]
0.0020068, 2458.3191
0.0022930, 2034.5888
...
```

- The code builds energy bins around each tabulated energy using midpoints in MeV.
- Bin weight is `weight_i = Phi(E_i) * ΔE_i`.
- Energies are converted to keV internally.

This means event sampling is proportional to the **integrated flux** in each bin, consistent with the 8-column “counts per bin” logic.

---

## 3. What happens at runtime

For **every event**, the generator does:

1. **Sample a bin** from the spectrum using `weight` as probability.
2. **Sample energy** uniformly inside that bin → this is the particle energy (in keV).
3. **Read polarization info** from that bin:
   - `μ = polMean`
   - `σ = polSigma`
4. **Draw** a probability `p ~ N(μ, σ)`, clamp to `[0, 1]`.
5. With probability `p` → set polarization to **(0, 1, 0)** (linear along Y);  
   otherwise → unpolarized.
6. **Place and shoot** the particle according to the emission mode:
   - `fixed`  → random point on the disk (R = 1.25 cm, z = -1 cm), direction `+z`
   - `sphere` → random point on a sphere of radius R, direction pointing inward (toward the origin)

So: the **file(s) control energy and polarization**, the **macro controls particle and emission**, and the **C++ controls geometry logic**.

---

## 4. Generate spectra with Python

Example usage of the helper spectrum generator:

```bash
# Uniform in [2, 8] keV (no polarization)
python make_spectrum.py uniform --emin 2 --emax 8 --nbins 100 --counts 1 --out spectra/uniform_2_8keV.txt

# Monochromatic 17.4 keV line (e.g. Mo Kα)
python make_spectrum.py mono --energy 17.4 --counts 1000 --out spectra/line_17p4keV.txt
```

Then in your macro:

```tcl
/B3/primary/spectrumFile ../spectra/line_17p4keV.txt
```

---

## 5. Component weights from flux spectra

If you have multiple background components as **flux CSVs**:

```text
{component}.csv  ->  E[MeV], Phi(E) [cm^-2 s^-1 sr^-1 MeV^-1]
```

you can use the helper script (e.g. `compute_component_weights.py`) to compute:

- **Integrated flux** for each component:  
  `F_j = ∫ Phi_j(E) dE  [particles cm^-2 s^-1 sr^-1]`
- **Normalized per-event weight** assuming you simulate the **same number of events** for each component:  
  `weight_norm_j = F_j / Σ_k F_k`

Typical usage:

```bash
python compute_component_weights.py     --folder spectra/backgrounds     --out component_weights.csv
```

This writes a file like:

```text
#component,integrated_flux[particles cm^-2 s^-1 sr^-1],weight_norm
CXB,1.23e+02,4.5e-01
albedo,8.00e+01,2.9e-01
protons,7.00e+01,2.6e-01
```

### How to use the weights in analysis

If you simulate **the same number of events** for each component:

- Tag events by their component (e.g. run them in separate jobs or encode an integer in the event ID).
- When filling histograms, multiply each event by `weight_norm` for that component:

```python
# pseudo-code
for event in events_of_component_j:
    hist.fill(some_observable, weight=weight_norm[j])
```

This way, the **relative contributions** of all components in your plots reflect the **real flux ratios**, even though you simulated the same number of particles for each one.

Later, if you want to convert to an absolute exposure time `T_target`, you can multiply all weights by a global factor (depending on sphere radius, integrated flux, and number of simulated events), but for most comparisons the **relative weights** are enough.

---

## 6. Utilities in `analysis/`

In `analysis/` you can find some ROOT / Python utilities:

- `checkDigi.py`  
  Quick check of digitized output.

- `ConvertForDigi_withSelection.cpp`  
  Convert simulation output for digitization with selection on containment.  
  - Compile:
    ```bash
    g++ -o convert ConvertForDigi_withSelection.cpp `root-config --cflags --libs` -lm
    ```
  - Use:
    ```bash
    ./convert <input_file.root> <output_basename> <fill_option: 1=check, 0=fill_all>
    ```

- `RecoTrack_faster.C`  
  ROOT macro to reconstruct tracks and extract basic event info.
  - Use:
    ```bash
    root -l 'RecoTrack_faster.C("output_t0.root")'
    ```

- `splitRootFile.C`  
  Split a large ROOT file into smaller chunks.
  - Use:
    ```bash
    root -l 'splitRootFile.C("bigfile.root")'
    ```

You can combine these with the **component weights** above when producing final spectra, rates, or background estimates.

---

Notes:

- `G4EmLivermorePolarizedPhysics` already includes the needed models to handle polarization; `G4LivermorePolarizedPhotoElectricGDModel` is available but not explicitly required here.
