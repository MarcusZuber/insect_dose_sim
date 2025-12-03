/*
 * Dose Simulation for Beetles - GEANT4 Simulation
 * Copyright (C) 2024
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef PrimaryGeneratorAction_h
#define PrimaryGeneratorAction_h

#include "G4VUserPrimaryGeneratorAction.hh"
#include "G4ParticleGun.hh"
#include <vector>
#include <string>
#include <CLHEP/Units/SystemOfUnits.h>

class G4Event;
class PrimaryGeneratorMessenger; // forward

class PrimaryGeneratorAction final : public G4VUserPrimaryGeneratorAction {
public:
    PrimaryGeneratorAction();

    ~PrimaryGeneratorAction() override;

    void GeneratePrimaries(G4Event *event) override;

    [[nodiscard]] const G4ParticleGun *GetParticleGun() const { return fParticleGun; }

    // Configuration accessors for macros (implemented in .cc)
    void SetSpectrumFilename(const std::string &filename);

    [[nodiscard]] const std::string &GetSpectrumFilename() const { return spectrumFilename; }

    static void SetPhotonFlux(G4double flux);

    [[nodiscard]] static G4double GetPhotonFlux() { return photonFlux; }

    void SetMonochromatic(bool mono);

    [[nodiscard]] bool IsMonochromatic() const { return monochromatic; }

    void SetMonoEnergy(G4double e);

    [[nodiscard]] G4double GetMonoEnergy() const { return monoEnergy; }

private:
    G4ParticleGun *fParticleGun;

    // For polychromatic spectrum sampling
    G4double SampleEnergyFromSpectrum();

    void InitializeSpectrum();

    std::vector<G4double> spectrumEnergies; // Energy bins
    std::vector<G4double> spectrumIntensities; // Relative intensities
    std::vector<G4double> cumulativeDistribution; // For inverse transform sampling
    G4double maxIntensity;

    // Configurable parameters (settable from macros)
    std::string spectrumFilename; // if empty -> auto-detect
    static G4double photonFlux; // default photons/s/mm2
    bool monochromatic{false};
    G4double monoEnergy{15.2 * CLHEP::keV};

    // Messenger to receive macro commands
    PrimaryGeneratorMessenger *messenger{nullptr};
};

#endif
