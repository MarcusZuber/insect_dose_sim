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

#include "DetectorMessenger.h"
#include "DetectorConstruction.h"
#include "G4UIdirectory.hh"
#include "G4UIcmdWithAString.hh"

DetectorMessenger::DetectorMessenger(DetectorConstruction *det)
    : G4UImessenger(), detector(det) {
    detectorDir = new G4UIdirectory("/detector/");
    detectorDir->SetGuidance("Detector control commands");

    selectInsectCmd = new G4UIcmdWithAString("/detector/selectInsect", this);
    selectInsectCmd->SetGuidance("Select insect to place: drosophila | leptopilina | sitophilus");
    selectInsectCmd->SetParameterName("insect", false);
    selectInsectCmd->AvailableForStates(G4State_PreInit, G4State_Idle);
}

DetectorMessenger::~DetectorMessenger() {
    delete selectInsectCmd;
    delete detectorDir;
}

void DetectorMessenger::SetNewValue(G4UIcommand *command, const G4String newValue) {
    if (command == selectInsectCmd) {
        detector->SetSelectedInsect(newValue);
    }
}
