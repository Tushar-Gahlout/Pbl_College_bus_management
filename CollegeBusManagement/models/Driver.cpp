#include "Driver.h"
#include <sstream>
#include <vector>

Driver::Driver()
    : Person("", "", "", ""), licenseNumber(""), assignedBus("None"), experienceYears(0) {}

Driver::Driver(const std::string& dId, const std::string& dName,
               const std::string& dPhone, const std::string& dLicense,
               const std::string& dAssignedBus, int dExpYears,
               const std::string& dEmail)
    : Person(dId, dName, dPhone, dEmail),
      licenseNumber(dLicense),
      assignedBus(dAssignedBus.empty() ? "None" : dAssignedBus),
      experienceYears(dExpYears) {}

std::string Driver::getDetails() const {
    std::ostringstream oss;
    oss << "Driver ID: " << id << "\n"
        << "Name: " << name << "\n"
        << "Phone: " << phone << "\n"
        << "License: " << licenseNumber << "\n"
        << "Assigned Bus: " << assignedBus << "\n"
        << "Experience: " << experienceYears << " years";
    return oss.str();
}

std::string Driver::serialize() const {
    // Pipe-delimited: id|name|phone|licenseNumber|assignedBus|experienceYears|email
    std::ostringstream oss;
    oss << id << "|" << name << "|" << phone << "|" << licenseNumber << "|"
        << assignedBus << "|" << experienceYears << "|" << email;
    return oss.str();
}

Driver Driver::deserialize(const std::string& line) {
    std::vector<std::string> tokens;
    std::stringstream ss(line);
    std::string token;
    while (std::getline(ss, token, '|')) {
        tokens.push_back(token);
    }

    if (tokens.size() < 5) {
        return Driver();
    }

    int exp = 0;
    if (tokens.size() >= 6) {
        try {
            exp = std::stoi(tokens[5]);
        } catch (...) { exp = 0; }
    }

    std::string email = (tokens.size() >= 7) ? tokens[6] : "";

    return Driver(tokens[0], tokens[1], tokens[2], tokens[3],
                  tokens[4], exp, email);
}
