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

#include "G4RunManager.hh"
#include "G4MTRunManager.hh"
#include "G4UImanager.hh"
#include "G4VisExecutive.hh"
#include "G4UIExecutive.hh"

#include "DetectorConstruction.h"
#include "PhysicsList.h"
#include "ActionInitialization.h"

#include "QBBC.hh"


int main(const int argc, char **argv) {
    // Detect interactive mode
    G4UIExecutive *ui = nullptr;
    if (argc == 1) {
        ui = new G4UIExecutive(argc, argv);
    }

    // Construct the run manager
#ifdef G4MULTITHREADED
    auto *runManager = new G4MTRunManager;
    if (ui) {
        G4cout << "WARNING: Visualization is not supported in multi-threaded mode." << G4endl;
        G4cout << "Running with 1 thread." << G4endl;
        runManager->SetNumberOfThreads(1);
    } else {
        runManager->SetNumberOfThreads(10);
    }
#else
    G4RunManager *runManager = new G4RunManager;
#endif

    // Set mandatory initialization classes
    runManager->SetUserInitialization(new DetectorConstruction());
    runManager->SetUserInitialization(new PhysicsList());
    runManager->SetUserInitialization(new ActionInitialization());

    // Initialize visualization manager
    G4VisManager *visManager = new G4VisExecutive;
    visManager->Initialize();

    // Get the pointer to the User Interface manager
    G4UImanager *ui_manager = G4UImanager::GetUIpointer();

    if (ui) {
        // Interactive mode
        ui_manager->ApplyCommand("/control/execute macros/vis.mac");
        ui->SessionStart();
        delete ui;
    } else {
        // Batch mode
        const G4String command = "/control/execute ";
        const G4String fileName = argv[1];
        ui_manager->ApplyCommand(command + fileName);
    }

    // Job termination
    delete visManager;
    delete runManager;

    return 0;
}
