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

#ifndef DetectorMessenger_h
#define DetectorMessenger_h

#include "G4UImessenger.hh"
#include "G4String.hh"

class G4UIcmdWithAString;
class DetectorConstruction;

class DetectorMessenger final : public G4UImessenger {
public:
    explicit DetectorMessenger(DetectorConstruction *det);

    ~DetectorMessenger() override;

    void SetNewValue(G4UIcommand *command, G4String newValue) override;

private:
    DetectorConstruction *detector;
    G4UIdirectory *detectorDir;
    G4UIcmdWithAString *selectInsectCmd;
};

#endif
