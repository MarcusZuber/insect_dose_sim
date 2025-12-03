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

#include "SteppingAction.h"
#include "G4Step.hh"
#include "G4RunManager.hh"

std::map<std::string, G4double> SteppingAction::doseMap;
std::map<std::string, G4double> SteppingAction::volumeMap;

SteppingAction::SteppingAction()
= default;

SteppingAction::~SteppingAction()
= default;

void SteppingAction::UserSteppingAction(const G4Step *step) {
    // Get energy deposition in this step
    const G4double energyDep = step->GetTotalEnergyDeposit();

    if (energyDep <= 0.) return;

    // Get volume name
    const G4VPhysicalVolume *volume = step->GetPreStepPoint()->GetTouchableHandle()->GetVolume();
    if (!volume) return;

    const G4String volumeName = volume->GetName();

    // Skip world volume
    if (volumeName == "World") return;

    // Accumulate energy deposition for this volume
    const std::string &volNameStr = volumeName;
    doseMap[volNameStr] += energyDep;
}

std::map<std::string, G4double> &SteppingAction::getDoseMap() { return doseMap; }

std::map<std::string, G4double> &SteppingAction::getVolumeMap() { return volumeMap; }
