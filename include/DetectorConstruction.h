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

#ifndef DetectorConstruction_h
#define DetectorConstruction_h

#include "G4VUserDetectorConstruction.hh"
#include "G4LogicalVolume.hh"
#include <map>

class DetectorMessenger; // forward

class DetectorConstruction final : public G4VUserDetectorConstruction {
public:
    DetectorConstruction();

    ~DetectorConstruction() override;

    G4VPhysicalVolume *Construct() override;

    void ConstructSDandField() override;

    // Allow runtime selection of the insect
    void SetSelectedInsect(const G4String &name);

    [[nodiscard]] G4String GetSelectedInsect() const;

private:
    void ConstructMeshes();

    static G4ThreeVector GetSTLMeshCenter(const G4String &filename);

    static G4VSolid *LoadSTLSolid(const G4String &filename, const G4String &name, G4double scaleFactor,
                                  const G4ThreeVector &offset);

    G4VPhysicalVolume *worldPhys;
    G4LogicalVolume *worldLogical;

    std::map<G4String, G4LogicalVolume *> meshLogicalVolumes;

    // currently selected insect (default)
    G4String selectedInsect;

    // messenger for UI command
    DetectorMessenger *messenger;
};

#endif
