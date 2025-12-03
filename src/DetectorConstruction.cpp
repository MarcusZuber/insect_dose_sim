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

#include "DetectorConstruction.h"
#include "DetectorMessenger.h"
#include "G4NistManager.hh"
#include "G4Box.hh"
#include "G4SubtractionSolid.hh"
#include "G4PVPlacement.hh"
#include "G4SystemOfUnits.hh"
#include "G4VisAttributes.hh"
#include "G4Colour.hh"
#include "G4SDManager.hh"
#include "G4TessellatedSolid.hh"
#include "G4TriangularFacet.hh"
#include "SteppingAction.h"
#include "G4RunManager.hh"
#include <fstream>
#include <iostream>
#include <map>
#include <cfloat>

DetectorConstruction::DetectorConstruction()
    : G4VUserDetectorConstruction(),
      worldPhys(nullptr),
      worldLogical(nullptr),
      selectedInsect("drosophila"),
      messenger(nullptr) {
    // create messenger
    messenger = new DetectorMessenger(this);
}

DetectorConstruction::~DetectorConstruction() {
    delete messenger;
}

G4VPhysicalVolume *DetectorConstruction::Construct() {
    // Get NIST material manager
    G4NistManager *nist = G4NistManager::Instance();

    // World - filled with air (large enough for scaled meshes)
    // Scaled meshes extend to about ±0.63mm in X/Y and 0.4-0.9mm in Z
    // But need more margin for safety
    constexpr G4double worldSize = 100.0 * mm; // 2000 mm to safely accommodate all meshes
    G4Material *worldMat = nist->FindOrBuildMaterial("G4_AIR");

    auto *solidWorld = new G4Box("World", worldSize / 2, worldSize / 2, 10 * mm);
    worldLogical = new G4LogicalVolume(solidWorld, worldMat, "World");
    worldPhys = new G4PVPlacement(nullptr,
                                  G4ThreeVector(),
                                  worldLogical,
                                  "World",
                                  nullptr,
                                  false,
                                  0,
                                  true);

    // Make world invisible
    worldLogical->SetVisAttributes(G4VisAttributes::GetInvisible());

    // Construct meshes
    ConstructMeshes();

    return worldPhys;
}

void DetectorConstruction::ConstructSDandField() {
    // Sensitive detectors are handled via SteppingAction
    // for dose scoring in each mesh volume
}

G4ThreeVector DetectorConstruction::GetSTLMeshCenter(const G4String &filename) {
    G4cout << "Calculating center of STL mesh: " << filename << G4endl;

    std::ifstream file(filename.c_str(), std::ios::binary);
    if (!file.is_open()) {
        G4cerr << "ERROR: Cannot open STL file: " << filename << G4endl;
        return {0, 0, 0};
    }

    // Skip 80-byte header
    file.seekg(80, std::ios::beg);

    // Read number of triangles
    uint32_t numTriangles;
    file.read(reinterpret_cast<char *>(&numTriangles), sizeof(uint32_t));

    if (numTriangles == 0) {
        G4cerr << "ERROR: No triangles in STL file: " << filename << G4endl;
        file.close();
        return {0, 0, 0};
    }

    // Find bounding box
    auto minX = DBL_MAX, minY = DBL_MAX, minZ = DBL_MAX;
    G4double maxX = -DBL_MAX, maxY = -DBL_MAX, maxZ = -DBL_MAX;

    for (uint32_t i = 0; i < numTriangles; i++) {
        file.seekg(12, std::ios::cur); // Skip normal

        // Read 3 vertices (9 floats)
        float vertices[9];
        file.read(reinterpret_cast<char *>(vertices), 9 * sizeof(float));

        file.seekg(2, std::ios::cur); // Skip attribute byte count

        // Check all 3 vertices
        for (int j = 0; j < 3; ++j) {
            float x = vertices[j * 3 + 0];
            float y = vertices[j * 3 + 1];
            float z = vertices[j * 3 + 2];

            if (x < minX) minX = x;
            if (x > maxX) maxX = x;
            if (y < minY) minY = y;
            if (y > maxY) maxY = y;
            if (z < minZ) minZ = z;
            if (z > maxZ) maxZ = z;
        }
    }

    file.close();

    G4ThreeVector center((minX + maxX) / 2.0, (minY + maxY) / 2.0, (minZ + maxZ) / 2.0);
    G4cout << "  -> STL center: (" << center.x() << ", " << center.y() << ", " << center.z() << ") mm" << G4endl;

    return center;
}


// Set selected insect at runtime (called by messenger)
void DetectorConstruction::SetSelectedInsect(const G4String &name) {
    // Validate
    if (name != "drosophila" && name != "leptopilina" && name != "sitophilus") {
        G4cout << "DetectorConstruction::SetSelectedInsect: unknown insect '" << name <<
                "' - allowed: drosophila, leptopilina, sitophilus" << G4endl;
        return;
    }

    if (selectedInsect == name) {
        G4cout << "DetectorConstruction: selected insect is already '" << name << "'" << G4endl;
        return;
    }

    selectedInsect = name;
    G4cout << "DetectorConstruction: selected insect set to '" << selectedInsect << "'" << G4endl;

    // Clear existing volume data before reinitializing
    //SteppingAction::ClearVolumes();

    // Reinitialize geometry so Construct() is called again with new selection
    if (G4RunManager *runManager = G4RunManager::GetRunManager()) {
        runManager->ReinitializeGeometry(true);
    }
}

G4String DetectorConstruction::GetSelectedInsect() const { return selectedInsect; }

void DetectorConstruction::ConstructMeshes() {
    G4NistManager *nist = G4NistManager::Instance();

    // Calculate the reference offset from 100_EtOH.stl
    // All meshes will be shifted relative to this reference
    const G4ThreeVector referenceOffset = GetSTLMeshCenter("meshes/100_EtOH.stl");
    G4cout << "\n=== Using reference offset from 100_EtOH.stl ===" << G4endl;
    G4cout << "All meshes will be shifted by: ("
            << referenceOffset.x() << ", "
            << referenceOffset.y() << ", "
            << referenceOffset.z() << ") mm" << G4endl << G4endl;

    // Define materials
    G4Material *ethanolMat = nist->FindOrBuildMaterial("G4_ETHYL_ALCOHOL");
    G4Material *pmmaMat = nist->FindOrBuildMaterial("G4_PLEXIGLASS");

    constexpr G4double density = 0.95 * g / cm3;
    const auto insectMat = new G4Material(R"(insectMat)", density, 6);
    // 30% ethanol and 70% try mass
    // ethanol = C2H5OH -> C: 2/9, H: 6/9, O: 1/9
    // dry mass of insect = C: 0.5, H: 0.07, N: 0.09,  O: 0.33, S: 0.005, P: 0.005
    insectMat->AddElement(nist->FindOrBuildElement("C"), 0.3 * 2. / 9. + 0.7 * 0.5); // Carbon
    insectMat->AddElement(nist->FindOrBuildElement("N"), 0.3 * 6. / 9. + 0.7 * 0.09); // Nitrogen
    insectMat->AddElement(nist->FindOrBuildElement("O"), 0.3 * 1. / 9. + 0.7 * 0.33); // Oxygen
    insectMat->AddElement(nist->FindOrBuildElement("P"), 0.7 * 0.005); // Phosphorus
    insectMat->AddElement(nist->FindOrBuildElement("S"), 0.7 * 0.005); // Sulfur
    insectMat->AddElement(nist->FindOrBuildElement("H"), 0.7 * 0.07); // Hydrogen

    // SELECT INSECT HERE: use fSelectedInsect (can be changed via UI command)

    // Map insect names to files and their known volumes
    std::map<G4String, G4String> insectFiles = {
        {"drosophila", "meshes/drosophila.stl"},
        {"leptopilina", "meshes/leptopilina.stl"},
        {"sitophilus", "meshes/sitophilus.stl"}
    };

    std::map<G4String, G4Colour> insectColours = {
        {"drosophila", G4Colour(1.0, 0.0, 0.0, 0.7)},
        {"leptopilina", G4Colour(0.0, 1.0, 0.0, 0.7)},
        {"sitophilus", G4Colour(0.0, 0.0, 1.0, 0.7)}
    };

    // Known volumes for insects (in mm^3)
    std::map<G4String, G4double> knownVolumes = {
        {"drosophila", 0.0 * mm3},
        {"leptopilina", 0.0 * mm3},
        {"sitophilus", 0.0 * mm3}
    };

    // 1. Load the selected insect (with reference offset)
    const G4String insectFile = insectFiles[selectedInsect];
    G4VSolid *insectSolid = LoadSTLSolid(insectFile, selectedInsect + "_solid", 10.0, referenceOffset);

    if (!insectSolid) {
        G4cerr << "ERROR: Failed to load insect mesh!" << G4endl;
        return;
    }

    // Override with known volume if available
    if (knownVolumes[selectedInsect] > 0) {
        SteppingAction::setVolume(selectedInsect, knownVolumes[selectedInsect]);
    }

    // Create insect logical volume
    auto *insectLogical = new G4LogicalVolume(insectSolid, insectMat, selectedInsect);
    const auto insectVis = new G4VisAttributes(insectColours[selectedInsect]);
    insectVis->SetVisibility(true);
    insectLogical->SetVisAttributes(insectVis);

    // Place insect
    new G4PVPlacement(nullptr, G4ThreeVector(0, 0, 0), insectLogical, selectedInsect,
                      worldLogical, false, 0, false);
    meshLogicalVolumes[selectedInsect] = insectLogical;

    // 2. Load ethanol and subtract insect from it (with reference offset)
    if (G4VSolid *ethanolSolid = LoadSTLSolid("meshes/100_EtOH.stl", "Ethanol_solid", 10.0, referenceOffset)) {
        // Create subtraction: Ethanol - Insect
        auto *ethanolSubtracted = new G4SubtractionSolid(
            "Ethanol", ethanolSolid, insectSolid, nullptr, G4ThreeVector(0, 0, 0));

        auto *ethanolLogical = new G4LogicalVolume(
            ethanolSubtracted, ethanolMat, "Ethanol");
        const auto ethanolVis = new G4VisAttributes(G4Colour(0.8, 0.8, 1.0, 0.3));
        ethanolVis->SetVisibility(true);
        ethanolLogical->SetVisAttributes(ethanolVis);

        new G4PVPlacement(nullptr, G4ThreeVector(0, 0, 0), ethanolLogical, "Ethanol",
                          worldLogical, false, 1, false);
        meshLogicalVolumes["Ethanol"] = ethanolLogical;

        // Recompute and store the volume of the subtracted solid (Ethanol - Insect)
        const G4double ethanolSubVolume = ethanolSubtracted->GetCubicVolume();

        SteppingAction::setVolume("Ethanol", ethanolSubVolume);
    }

    // 3. Load tube (with reference offset)
    if (G4VSolid *tubeSolid = LoadSTLSolid("meshes/tube.stl", "Tube_solid", 10.0, referenceOffset)) {
        auto *tubeLogical = new G4LogicalVolume(tubeSolid, pmmaMat, "Tube");
        const auto tubeVis = new G4VisAttributes(G4Colour(0.5, 0.5, 0.5, 0.2));
        tubeVis->SetVisibility(true);
        tubeLogical->SetVisAttributes(tubeVis);

        new G4PVPlacement(nullptr, G4ThreeVector(0, 0, 0), tubeLogical, "Tube",
                          worldLogical, false, 2, false);
        meshLogicalVolumes["Tube"] = tubeLogical;
    }

    G4cout << "\n=== Geometry loaded ===" << G4endl;
    G4cout << "Selected insect: " << selectedInsect << G4endl;
    G4cout << "Volumes: Tube, Ethanol (with insect subtracted), " << selectedInsect << G4endl;
}

G4VSolid *DetectorConstruction::LoadSTLSolid(const G4String &filename, const G4String &name,
                                             G4double scaleFactor, const G4ThreeVector &offset) {
    std::ifstream file(filename.c_str(), std::ios::binary);
    if (!file.is_open()) {
        G4cerr << "ERROR: Cannot open STL file: " << filename << G4endl;
        return nullptr;
    }

    // Skip 80-byte header
    char header[80];
    file.read(header, 80);

    // Read number of triangles
    uint32_t numTriangles;
    file.read(reinterpret_cast<char *>(&numTriangles), sizeof(uint32_t));

    G4cout << "Number of triangles: " << numTriangles << G4endl;

    // Create tessellated solid
    auto *solid = new G4TessellatedSolid(name);

    // Read triangles
    for (uint32_t i = 0; i < numTriangles; i++) {
        // Read normal (skip for now)
        float normal[3];
        file.read(reinterpret_cast<char *>(normal), 3 * sizeof(float));

        // Read vertices
        float v1[3], v2[3], v3[3];
        file.read(reinterpret_cast<char *>(v1), 3 * sizeof(float));
        file.read(reinterpret_cast<char *>(v2), 3 * sizeof(float));
        file.read(reinterpret_cast<char *>(v3), 3 * sizeof(float));

        // Skip attribute byte count
        uint16_t attributeByteCount;
        file.read(reinterpret_cast<char *>(&attributeByteCount), sizeof(uint16_t));

        // Create Geant4 vertices with offset applied
        G4ThreeVector vertex1((v1[0] - offset.x()) * mm * scaleFactor,
                              (v1[1] - offset.y()) * mm * scaleFactor,
                              (v1[2] - offset.z()) * mm * scaleFactor);
        G4ThreeVector vertex2((v2[0] - offset.x()) * mm * scaleFactor,
                              (v2[1] - offset.y()) * mm * scaleFactor,
                              (v2[2] - offset.z()) * mm * scaleFactor);
        G4ThreeVector vertex3((v3[0] - offset.x()) * mm * scaleFactor,
                              (v3[1] - offset.y()) * mm * scaleFactor,
                              (v3[2] - offset.z()) * mm * scaleFactor);

        // Add facet to solid
        auto *facet = new G4TriangularFacet(vertex1, vertex2, vertex3, ABSOLUTE);
        solid->AddFacet(facet);
    }

    file.close();

    // Close the solid
    solid->SetSolidClosed(true);

    // Calculate and store volume
    G4double volume = solid->GetCubicVolume();

    // Store volume (remove "_solid" suffix if present)
    G4String volumeName = name;
    if (size_t pos = volumeName.find("_solid"); pos != std::string::npos) {
        volumeName = volumeName.substr(0, pos);
    }
    SteppingAction::setVolume(volumeName, volume);

    return solid;
}
