# Dose simulation for insects using Geant4

This is a Geant4-based dose simulation that uses STL mesh files to define detector geometries and scores the dose
deposited in each mesh volume.
Either a spectrum file or monoenergetic photons can be used as the radiation source.

## Features

- **STL Mesh Loading**: Loads multiple STL files as detector geometries
- **Dose Scoring**: Calculates and reports dose for each mesh volume
- **Mono-energetic and Spectrum Sources**: Supports monoenergetic photons and spectrum-driven sources
- **Low-Energy Physics**: Uses Penelope physics for accurate low-energy photon interactions
- **Multi-threading Support**: Can run with multiple threads for faster simulations

## STL Meshes

The following STL files are loaded (note exact filenames):

- `drosophila.stl` - Drosophila insect (*insect material*, see Materials section)
- `leptopilina.stl` - Leptopilina insect (*insect material*, see Materials section)
- `sitophilus.stl` - Sitophilus insect (*insect material*, see Materials section)
- `100_EtOH.stl` - Ethanol medium (note the underscore and capitalization)
- `tube.stl` - Plastic tube surrounding the insect and ethanol

## Materials

### Insect Material

The insect material is mpdeled as 70% dry biomass and 30% absolute ethanol (C₂H₆O, 100%) by mass.
The dry biomass was modeled with a simplified composition of 50% carbon, 7% hydrogen, 9% nitrogen, 33% oxygen, and 0.5%
each of phosphorus and sulfur, based on whole-body elemental data across
multiple insect taxa (Back & King, 2013; Fagan et al., 2002) and the dominant biochemical components chitin, proteins,
and lipids (Hackman, 1974). Ethanol contributes additional carbon and hydrogen while reducing the overall oxygen
fraction compared to hydrated tissue. The bulk density of the ethanol-preserved insect material was estimated at
approximately 0.95 g/cm³, based on a weighted combination of dry tissue (∼1.3–1.4 g/cm³) and 100% ethanol (0.789 g/cm³).

## Building the Project

```bash
cd /home/yz5083/CLionProjects/insect_dose_sim
mkdir -p build
cd build
cmake ..
make -j4
```

## Running the Simulation

### Interactive Mode (with visualization)

```bash
./insect_dose_sim
```

### Batch Mode (with a macro file)

```bash
./insect_dose_sim macros/run_leptopilina_mono.mac
```

### Quick Test (example macros/test macro not included by default; use one of the provided macros with reduced /run/beamOn)

```bash
# Edit a macro to use fewer events, e.g. /run/beamOn 100
./insect_dose_sim macros/run_drosophila_mono.mac
```

## Available macro settings (commands)

The simulation supports the following macro commands (used in the provided example macros):

- `/detector/selectInsect <name>`
    - Select the insect geometry. Supported values (examples): `drosophila`, `leptopilina`, `sitophilus`.
    - Example: `/detector/selectInsect drosophila`

- `/run/initialize`
    - Initialize the geometry and physics before running events.

- `/generator/setMonochromatic <true|false>`
    - Enable or disable monoenergetic source mode. If `false`, a spectrum file is used.
    - Example: `/generator/setMonochromatic true`

- `/generator/setSpectrumFile <file>`
    - Path to a spectrum file relative to the working directory (example included: `spectra/image_filtered_wb.txt`).
    - Example: `/generator/setSpectrumFile image_filtered_wb.txt`

- `/generator/setPhotonFlux <value>`
    - Photon flux expressed in photons/mm^2/s used by the generator when relevant.
    - Example: `/generator/setPhotonFlux 1.36e13`

- `/generator/setMonoEnergy <value> [keV]`
    - Set the monoenergetic photon energy (units used in macros are keV).
    - Example: `/generator/setMonoEnergy 15.2 keV`

- `/output/setFileNamePrefix <prefix>`
    - Set a prefix for output files produced by the run.
    - Example: `/output/setFileNamePrefix dose_mono_`

- `/random/setSeeds <seed1> <seed2>`
    - Set the RNG seeds for reproducibility.
    - Example: `/random/setSeeds 42 8675309`

- `/run/beamOn <N>`
    - Start the run for N events.
    - Example: `/run/beamOn 10000000`

Visualization-related commands (used in `macros/vis.mac`):

- `/vis/open OGLI`, `/vis/verbose`, `/vis/drawVolume`, `/vis/viewer/set/viewpointThetaPhi`,
  `/vis/scene/add/trajectories`, `/tracking/storeTrajectory` and related `/vis/modeling/...` commands to control
  trajectory drawing and accumulation.

## Included example macros

The repository provides several example macros under `macros/`:

- `macros/run_drosophila_mono.mac` - Monoenergetic run for Drosophila (15.2 keV in the example)
- `macros/run_leptopilina_mono.mac` - Monoenergetic run for Leptopilina
- `macros/run_sitophilus_mono.mac` - Monoenergetic run for Sitophilus
- `macros/run_drosophila_wb.mac` - Spectrum-driven (white beam) run for Drosophila
- `macros/run_leptopilina_wb.mac` - Spectrum-driven run for Leptopilina
- `macros/run_sitophilus_wb.mac` - Spectrum-driven run for Sitophilus
- `macros/vis.mac` - Visualization settings and small example run (draws trajectories)

You can copy and edit any of these macros to change `/run/beamOn`, photon energy, flux, output prefix or RNG seeds.

## Spectrum files

- Example spectrum used by the white-beam macros: `spectra/image_filtered_wb.txt`

## Output

The simulation typically writes output files with the configured prefix into the current folder. Example contents
include per-volume deposited energy and calculated dose values.

## Physics

- **Physics List**: G4EmLivermorePhysics for accurate low-energy electromagnetic interactions
- **Beam Type**: Parallel beam (parameters set in `src/PrimaryGeneratorAction.cpp`)
- **Production Cuts**: Default example uses ~0.1 mm production cuts (see source)

## Customization

- Change number of events: edit a macro and modify `/run/beamOn`.
- Change beam energy: edit `src/PrimaryGeneratorAction.cpp` or set `/generator/setMonoEnergy` in macros.
- Change beam radius or geometry: edit `src/PrimaryGeneratorAction.cpp` and `src/DetectorConstruction.cpp`.
- Add/remove STL meshes: edit `src/DetectorConstruction.cpp` in `ConstructMeshes()`.

## Notes

- The dose calculation assumes a density of approximately 1 g/cm³ for simplified calculations. For more accurate dose
  calculations, adjust densities in the detector/material definitions.
- STL files are expected in millimeter units and are usually binary format.

## License and third-party dependencies

- This code is provided under the GNU Lesser General Public License v3 (LGPL-3.0). See the `LICENSE` file for full text.
- Geant4 is a separate third-party toolkit and is NOT distributed with this repository. You must install Geant4
  separately and accept its license terms. Geant4 has its own license and citation requirements — consult the Geant4
  website and your local installation for details.

SPDX-License-Identifier: LGPL-3.0

Copyright 2025, Copyright Owner: Karlsruhe Institute of Technology (KIT)\
Author: Marcus Zuber (geant4 simulation), Angelica Cecilia (spectra files), Thomas van de Kamp (insect STL files)\
Contact: marcus.zuber@kit.edu, Institute for Photon Science and Synchrotron Radiation
