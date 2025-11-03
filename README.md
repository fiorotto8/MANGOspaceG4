# README – Using the Geant4 primary generator with external spectra

This Geant4 primary generator is set up so that **only a few things are hard-coded** in C++:

- the **particle type** (e.g. `gamma`, `proton`, `e-`)
- the **source position/distribution** (disk of radius 3.7 cm at `z = -1 cm`)
- the **direction** (along `+z`)

Everything else — **energy distribution** and **event-by-event polarization** — comes from an external text file that you can generate with the Python script.

---

## 1. Choose the particle

Open:

```text
src/PrimaryGeneratorAction.cc
```

and in the constructor find:

```cpp
auto* table  = G4ParticleTable::GetParticleTable();
fParticleGun->SetParticleDefinition(table->FindParticle("gamma"));
```

Change `"gamma"` to the particle you want:

Rebuild the project.

---

## 2. Prepare the spectrum file

The code expects a file with columns:

```text
#bin  bin_low_keV  bin_center_keV  bin_high_keV  counts  error  polarization  polarization_error
```

Meaning:

- **counts**: weight of the bin (higher → picked more often)
- **polarization**: mean probability to be linearly polarized
- **polarization_error**: sigma of that probability

By default the C++ loads:

```cpp
LoadSpectrum("../h_deconvolved.txt");
```

So either:

- put `h_deconvolved.txt` one level above your run directory, **or**
- change the path in the constructor.

---

## 3. What happens at runtime

For **every event**, Geant4 does:

1. **Pick a bin** at random, using `counts` as weights.
2. **Sample energy** uniformly inside that bin → this is the particle energy.
3. **Read polarization info** from that bin:
   - `μ = polarization`
   - `σ = polarization_error`
4. **Draw** a probability `p ~ N(μ, σ)` and clamp it to `[0, 1]`.
5. **With probability `p`** → set polarization to **(0, 1, 0)** (linear along Y);  
   **else** → unpolarized.
6. **Place** the particle on the disk (radius 3.7 cm, `z = -1 cm`).
7. **Shoot** along `+z`.

So: **the file controls energy *and* polarization**, C++ controls geometry & particle.

---

## 4. Generate the spectrum with Python (optional)

Use the helper script:

```bash
python make_spectrum.py uniform --emin 2 --emax 8 --nbins 100 --counts 1 --out your_spectrum.txt
```

This makes a flat 2–8 keV spectrum, no polarization.

Monochromatic example:

```bash
python make_spectrum.py mono --energy 17.4 --counts 1000 --out your_spectrum.txt
```

Then run your Geant4 app — it will read `your_spectrum.txt` and sample from it.

## Utilities

In analysis/ you can find some ROOT macros:

- checkDigi.py: to check the digitized output
- ConvertForDigi_withSelection.cpp: to convert the output for digitization with selection on containment 
  - compile ```g++ -o convert ConvertForDigi_withSelection.cpp `root-config --cflags --libs` -lm```
  - Usage: ```./convert <input_file.root> <output_basename> <fill_option: 1=check, 0=fill_all>```
- RecoTrack_faster.C: ROOT macro to reconstruct and get some inf fromt he events
  - Usage: ```root -l 'RecoTrack.C("output_t0.root")'```
- splitRootFile.C: ROOT macro to split a big root file into smaller ones
  - Usage: ```root -l 'splitRootFile.C("bigfile.root")'```

Notes:

- G4LivermorePolarizedPhotoElectricGDModel is present but not used since the G4EmLivermorePolarizedPhysics is enough to reproduce polarization.