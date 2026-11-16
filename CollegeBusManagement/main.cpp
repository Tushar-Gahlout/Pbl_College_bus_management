/**
 * ============================================================================
 * Project Name: College Bus Management System
 * Technology  : C++17 + Qt 6 Widgets + File Handling
 * Course      : Object Oriented Programming (OOPS with C++) PBL Project
 * ============================================================================
 * 
 * Demonstration of Key OOPS Concepts:
 * 1. Classes and Objects   : Models (Student, Driver, Bus, Route, Seat, FeeRecord)
 * 2. Encapsulation        : Private member fields, getters/setters, state validation
 * 3. Inheritance          : Person base class -> Student and Driver derived classes
 * 4. Polymorphism         : Virtual methods (getRole(), getDetails(), serialize())
 * 5. Abstraction          : Abstract Person base class & clean service interfaces
 * 6. Constructors/Destruct: Parameterized constructors & virtual destructors
 * 7. Function Overloading : searchStudents(), recordPayment()
 * 8. Exception Handling   : Custom exceptions (DuplicateIdException, BusFullException)
 * 
 * Persistent Storage:
 * Pure C++ File Handling (std::ifstream, std::ofstream, std::fstream, std::ios::app)
 * No database used. Data stored in text files inside the 'data/' folder.
 * ============================================================================
 */

#include <QApplication>
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <iostream>

#include "services/FileManager.h"
#include "views/LoginDialog.h"
#include "views/MainWindow.h"

// Helper to locate the 'data' directory relative to execution binary or project root
std::string resolveDataDirectory() {
    QStringList candidates = {
        "data",
        "../data",
        "../../data",
        "CollegeBusManagement/data",
        QDir::currentPath() + "/data",
        QCoreApplication::applicationDirPath() + "/data",
        QCoreApplication::applicationDirPath() + "/../data"
    };

    for (const auto& path : candidates) {
        if (QDir(path).exists() || QFile::exists(path + "/users.txt")) {
            return QDir(path).absolutePath().toStdString();
        }
    }
    // Default fallback to "data" in current working directory
    return "data";
}

int main(int argc, char *argv[]) {
    // 1. Initialize Qt Widgets Application
    QApplication app(argc, argv);
    app.setApplicationName("College Bus Management System");
    app.setOrganizationName("Campus Transport");

    // 2. Load stylesheet (style.qss)
    QFile styleFile(":/style.qss");
    if (!styleFile.open(QFile::ReadOnly)) {
        // Fallback to relative file paths if resource is not embedded
        QStringList styleCandidates = {
            "resources/style.qss",
            "../resources/style.qss",
            "../../resources/style.qss",
            "CollegeBusManagement/resources/style.qss"
        };
        for (const auto& p : styleCandidates) {
            styleFile.setFileName(p);
            if (styleFile.open(QFile::ReadOnly)) break;
        }
    }
    if (styleFile.isOpen()) {
        QString styleSheet = QLatin1String(styleFile.readAll());
        app.setStyleSheet(styleSheet);
        styleFile.close();
    }

    // 3. Initialize File Handling Engine
    std::string dataDir = resolveDataDirectory();
    FileManager fileManager(dataDir);

    // 4. Present Login Authentication Window
    LoginDialog loginDialog(&fileManager);
    if (loginDialog.exec() != QDialog::Accepted) {
        return 0; // User cancelled or exited login
    }

    // 5. Launch Main Window with authenticated role & permissions
    User user = loginDialog.getAuthenticatedUser();
    MainWindow mainWindow(&fileManager, user);
    mainWindow.show();

    // 6. Enter Qt Event Loop
    return app.exec();
}
