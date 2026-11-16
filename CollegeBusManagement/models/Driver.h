#ifndef DRIVER_H
#define DRIVER_H

#include "Person.h"
#include <sstream>
#include <vector>

/**
 * @brief Driver class derived from Person.
 * 
 * Demonstrates OOP Concepts:
 * 1. Inheritance - Derived from Person base class.
 * 2. Polymorphism - Overrides getRole(), getDetails(), and serialize().
 * 3. Encapsulation - Private member variables with getters/setters.
 */
class Driver : public Person {
private:
    std::string licenseNumber;
    std::string assignedBus;
    int experienceYears;

public:
    // Default Constructor
    Driver();

    // Parameterized Constructor
    Driver(const std::string& dId, const std::string& dName,
           const std::string& dPhone, const std::string& dLicense,
           const std::string& dAssignedBus = "None",
           int dExpYears = 0,
           const std::string& dEmail = "");

    // Getters
    std::string getLicenseNumber() const { return licenseNumber; }
    std::string getAssignedBus() const { return assignedBus; }
    int getExperienceYears() const { return experienceYears; }

    // Setters
    void setLicenseNumber(const std::string& license) { licenseNumber = license; }
    void setAssignedBus(const std::string& bus) { assignedBus = bus; }
    void setExperienceYears(int years) { experienceYears = years; }

    // Polymorphic implementations
    std::string getRole() const override { return "Driver"; }
    std::string getDetails() const override;
    std::string serialize() const override;

    // Static deserialization method for file handling
    static Driver deserialize(const std::string& line);
};

#endif // DRIVER_H
