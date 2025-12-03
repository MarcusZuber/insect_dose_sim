/*
 * Geant4 based dose simulation for insects
 * Copyright (C) 2025
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

#include "PrimaryGeneratorAction.h"
#include "PrimaryGeneratorMessenger.h"
#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include "G4ParticleDefinition.hh"
#include "parameters.h"
#include "Randomize.hh"
#include <algorithm>
#include <numeric>
#include <fstream>
#include <sstream>
#include <filesystem>

G4double PrimaryGeneratorAction::photonFlux = 1e12; // photons/s/mm2 default

PrimaryGeneratorAction::PrimaryGeneratorAction()
    : fParticleGun(nullptr), maxIntensity(0.), messenger(nullptr) {
    // Create particle gun
    constexpr G4int nParticles = 1;
    fParticleGun = new G4ParticleGun(nParticles);

    // Get particle definition for gamma (photon)
    G4ParticleTable *particleTable = G4ParticleTable::GetParticleTable();
    G4ParticleDefinition *particle = particleTable->FindParticle("gamma");

    // Set particle properties
    fParticleGun->SetParticleDefinition(particle);

    // Create messenger so macros can configure generator
    messenger = new PrimaryGeneratorMessenger(this);

    // Initialize polychromatic spectrum
    InitializeSpectrum();

    // Initial energy will be sampled per event or set as mono
    fParticleGun->SetParticleEnergy(15.2 * CLHEP::keV); // default

    // Set parallel beam along +Z direction
    fParticleGun->SetParticleMomentumDirection(G4ThreeVector(0., 0., -1.));

    // Position at the beginning (before the meshes at Z~400-900mm)
    fParticleGun->SetParticlePosition(G4ThreeVector(0., 0., 5. * CLHEP::mm));
}

PrimaryGeneratorAction::~PrimaryGeneratorAction() {
    delete messenger;
    delete fParticleGun;
}

void PrimaryGeneratorAction::SetSpectrumFilename(const std::string &filename) {
    spectrumFilename = filename;
    // Reinitialize spectrum on change
    spectrumEnergies.clear();
    spectrumIntensities.clear();
    cumulativeDistribution.clear();
    InitializeSpectrum();
}

void PrimaryGeneratorAction::SetPhotonFlux(const G4double flux) {
    photonFlux = flux;
}

void PrimaryGeneratorAction::SetMonochromatic(const bool mono) {
    monochromatic = mono;
}

void PrimaryGeneratorAction::SetMonoEnergy(const G4double e) {
    monoEnergy = e;
}

void PrimaryGeneratorAction::InitializeSpectrum() {
    // If monochromatic mode is set, build a single-energy spectrum
    if (monochromatic) {
        spectrumEnergies.clear();
        spectrumIntensities.clear();
        cumulativeDistribution.clear();

        spectrumEnergies.push_back(monoEnergy);
        spectrumIntensities.push_back(1.0);
        cumulativeDistribution.push_back(0.0);
        cumulativeDistribution.push_back(1.0);
        maxIntensity = 1.0;

        G4cout << "Initialized monochromatic spectrum at " << monoEnergy / CLHEP::keV << " keV" << G4endl;
        return;
    }

    // Try to read a spectrum file from the provided filename or the `spectra/` directory.
    namespace fs = std::filesystem;
    std::vector<std::pair<G4double, G4double> > parsed; // pair<energy_eV, intensity>

    fs::path fileToOpen;
    if (!spectrumFilename.empty()) {
        if (fs::path p(spectrumFilename); p.is_absolute()) {
            if (fs::exists(p)) fileToOpen = p;
        } else {
            // check in project spectra dir first
            if (fs::path candidate = fs::path("spectra") / p; fs::exists(candidate)) fileToOpen = candidate;
            else if (fs::exists(p)) fileToOpen = p; // relative to cwd
        }
    } else {
        // auto-detect first supported file
        std::vector<fs::path> candidates = {"spectra", "./spectra", "../spectra"};
        for (const auto &d: candidates) {
            fs::path dir = d;
            if (!fs::exists(dir) || !fs::is_directory(dir)) continue;
            for (auto &entry: fs::directory_iterator(dir)) {
                if (!entry.is_regular_file()) continue;
                auto ext = entry.path().extension().string();
                std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
                if (ext == ".txt" || ext == ".dat" || ext == ".spc") {
                    fileToOpen = entry.path();
                    break;
                }
            }
            if (!fileToOpen.empty()) break;
        }
    }

    bool fileLoaded = false;
    if (!fileToOpen.empty()) {
        if (std::ifstream ifs(fileToOpen); ifs) {
            G4cout << "Reading spectrum file: " << fileToOpen.string() << G4endl;
            std::string line;
            while (std::getline(ifs, line)) {
                // remove comments after '#'
                if (auto commentPos = line.find('#'); commentPos != std::string::npos) line.resize(commentPos);
                // trim
                std::istringstream iss(line);
                double e_eV = 0.0, inten = 0.0;
                if (!(iss >> e_eV)) continue; // skip empty/invalid lines
                // some files may have only energy + counts per bin; if intensity missing, treat as 1
                if (!(iss >> inten)) inten = 1.0;
                parsed.emplace_back(e_eV, inten);
            }
            if (!parsed.empty()) fileLoaded = true;
        }
    }
    if (!fileLoaded) {
        G4cerr << "ERROR: Could not load spectrum file '" << spectrumFilename << G4endl;
        throw std::runtime_error("Failed to load spectrum file");
    }

    // Convert parsed pairs into the member vectors. Ensure energies are sorted ascending.
    std::sort(parsed.begin(), parsed.end(), [](const auto &a, const auto &b) { return a.first < b.first; });
    spectrumEnergies.clear();
    spectrumIntensities.clear();
    for (auto &[energy, intensity]: parsed) {
        spectrumEnergies.push_back(energy * CLHEP::eV);
        spectrumIntensities.push_back(intensity);
    }


    // Build cumulative distribution and normalize
    if (spectrumIntensities.empty()) {
        spectrumEnergies = {15.2 * CLHEP::keV};
        spectrumIntensities = {1.0};
    }

    G4double sum = std::accumulate(spectrumIntensities.begin(), spectrumIntensities.end(), 0.0);
    if (sum <= 0.0) {
        // avoid division by zero
        std::fill(spectrumIntensities.begin(), spectrumIntensities.end(), 1.0);
        sum = static_cast<G4double>(spectrumIntensities.size());
    }

    maxIntensity = *std::max_element(spectrumIntensities.begin(), spectrumIntensities.end());

    // Normalize and create cumulative distribution
    cumulativeDistribution.clear();
    G4double cumulative = 0.0;
    cumulativeDistribution.push_back(0.0);

    for (double &spectrumIntensity: spectrumIntensities) {
        spectrumIntensity /= sum;
        cumulative += spectrumIntensity;
        cumulativeDistribution.push_back(cumulative);
    }

    G4cout << "Initialized polychromatic spectrum with " << spectrumEnergies.size() << " energy bins" << G4endl;
    if (!spectrumEnergies.empty())
        G4cout << "Energy range: " << spectrumEnergies.front() / CLHEP::keV << " - " << spectrumEnergies.back() /
                CLHEP::keV << " keV" << G4endl;
}

G4double PrimaryGeneratorAction::SampleEnergyFromSpectrum() {
    // Inverse transform sampling
    const G4double random = G4UniformRand();

    // binary search in cumulative distribution
    const auto it = std::lower_bound(cumulativeDistribution.begin(),
                                     cumulativeDistribution.end(),
                                     random);

    size_t index = std::distance(cumulativeDistribution.begin(), it);
    if (index > 0) index--;
    if (index >= spectrumEnergies.size()) index = spectrumEnergies.size() - 1;

    // Linear interpolation between bins
    if (index < spectrumEnergies.size() - 1) {
        const G4double f = (random - cumulativeDistribution[index]) /
                           (cumulativeDistribution[index + 1] - cumulativeDistribution[index]);
        return spectrumEnergies[index] + f * (spectrumEnergies[index + 1] - spectrumEnergies[index]);
    }

    return spectrumEnergies[index];
}

void PrimaryGeneratorAction::GeneratePrimaries(G4Event *event) {
    // Generate parallel beam

    const G4double x = (G4UniformRand() - 0.5) * beamSize;
    const G4double y = (G4UniformRand() - 0.5) * beamSize;
    constexpr G4double z = 5 * CLHEP::mm; // Start position (before scaled meshes at Z ~0.4-0.9mm)

    fParticleGun->SetParticlePosition(G4ThreeVector(x, y, z));
    fParticleGun->SetParticleMomentumDirection(G4ThreeVector(0., 0., -1.));

    // Sample energy from polychromatic spectrum
    const G4double energy = SampleEnergyFromSpectrum();
    fParticleGun->SetParticleEnergy(energy);

    fParticleGun->GeneratePrimaryVertex(event);
}
