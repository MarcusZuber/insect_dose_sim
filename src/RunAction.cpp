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

#include "RunAction.h"
#include "RunMessenger.h"
#include "SteppingAction.h"
#include "DetectorConstruction.h"
#include "G4RunManager.hh"
#include "G4Run.hh"
#include "G4SystemOfUnits.hh"
#include <fstream>
#include <iomanip>
#include "parameters.h"
#include <cmath>

#include "PrimaryGeneratorAction.h"

RunAction::RunAction()
{
    messenger = new RunMessenger(this);
}

RunAction::~RunAction() {
    delete messenger;
}

void RunAction::BeginOfRunAction(const G4Run *run) {
    SteppingAction::getDoseMap().clear();
}

void RunAction::EndOfRunAction(const G4Run *run) {

    G4int nEvents = run->GetNumberOfEvent();


    if (nEvents == 0) return;


    G4double photonFlux = PrimaryGeneratorAction::GetPhotonFlux(); // photons/s/mm2

    // Get dose map from SteppingAction
    auto &doseMap = SteppingAction::getDoseMap();
    auto &volumeMap = SteppingAction::getVolumeMap();

    // Calculate and print dose for each volume
    G4cout << "\n========================================" << G4endl;
    G4cout << "Dose Summary (per volume)" << G4endl;
    G4cout << "========================================" << G4endl;

    // Get selected insect name
    const auto *detConstruction = dynamic_cast<const DetectorConstruction *>(
        G4RunManager::GetRunManager()->GetUserDetectorConstruction());
    G4String insectName = detConstruction->GetSelectedInsect();

    // Open output file with insect name
    std::ostringstream fileName;
    // Use configurable prefix from header
    fileName << outputPrefix << insectName << ".txt";
    std::ofstream outFile(fileName.str());

    // Photon flux (photons / s / mm^2) - from notes

    outFile << "Number of events: " << nEvents << "\n";
    outFile << "Photon flux: " << photonFlux << " photons/s/mm2\n";
    outFile << "========================================\n";
    outFile << std::setw(20) << "Volume Name"
            << std::setw(15) << "Volume (mm3)"
            << std::setw(15) << "Density (g/cm3)"
            << std::setw(20) << "Energy Dep (MeV)"
            << std::setw(20) << "Dose (Gy)"
            << std::setw(20) << "Dose per event (Gy)"
            << std::setw(20) << "Dose rate (Gy/s) with 100mA"
            << "\n";
    outFile << "========================================\n";

    // Beam radius used in PrimaryGeneratorAction

    G4double beamArea_mm2 = beamArea; // area in mm^2
    G4double photonsPerSecond = photonFlux * beamArea_mm2;

    // Iterate over all registered volumes so we print zeros too
    for (const auto &[fst, snd]: volumeMap) {
        std::string volName = fst;
        G4double totalEnergyDep = 0.0; // default if no deposition
        if (auto it = doseMap.find(volName); it != doseMap.end()) totalEnergyDep = it->second; // in MeV

        G4double volume = snd; // in mm3
        G4double density = 0.95e-3; // g/cm3, // Approximate mass in g (assuming density ~ 1 g/cm3)
        if (volName == "Tube")
            density = 1.05E-3; // PMMA density ~1.05 g/cm3
        if (volName == "Ethanol")
            density = 0.789E-3;

        G4double mass = volume * density;

        // Calculate dose: Energy (MeV) / mass (g)
        // 1 MeV/g = 1.602e-10 Gy
        G4double dose = 0.0;
        G4double dosePerEvent = 0.0;
        G4double doseRate = 0.0;
        if (mass > 0.0) {
            dose = totalEnergyDep * 1.602e-10 / mass; // in Gy
            dosePerEvent = dose / nEvents;
            doseRate = dosePerEvent * photonsPerSecond; // Gy/s
        }

        G4cout << std::setw(20) << volName
                << std::setw(15) << volume / mm3
                << std::setw(15) << density * 1e3
                << std::setw(20) << totalEnergyDep
                << std::setw(20) << dose
                << std::setw(20) << dosePerEvent
                << std::setw(20) << doseRate
                << G4endl;

        outFile << std::setw(20) << volName
                << std::setw(15) << volume / mm3
                << std::setw(15) << density * 1e3
                << std::setw(20) << totalEnergyDep
                << std::setw(20) << dose
                << std::setw(20) << dosePerEvent
                << std::setw(20) << doseRate
                << "\n";
    }

    G4cout << "========================================\n" << G4endl;
    outFile << "========================================\n";
    outFile.close();

    G4cout << "Results saved to " << fileName.str() << G4endl;
}

void RunAction::SetOutputFilePrefix(const std::string &prefix) { outputPrefix = prefix; }

const std::string &RunAction::GetOutputFilePrefix() const { return outputPrefix; }
