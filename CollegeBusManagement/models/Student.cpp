#include "Student.h"
#include <sstream>
#include <vector>

Student::Student()
    : Person("", "", "", ""), department(""), semester(1), address(""),
      busNumber("None"), pickupPoint("None"), seatNumber(0) {}

Student::Student(const std::string& sId, const std::string& sName,
                 const std::string& sDept, int sSem,
                 const std::string& sPhone, const std::string& sAddress,
                 const std::string& sBusNum, const std::string& sPickup,
                 int sSeat, const std::string& sEmail)
    : Person(sId, sName, sPhone, sEmail),
      department(sDept), semester(sSem), address(sAddress),
      busNumber(sBusNum.empty() ? "None" : sBusNum),
      pickupPoint(sPickup.empty() ? "None" : sPickup),
      seatNumber(sSeat) {}

std::string Student::getDetails() const {
    std::ostringstream oss;
    oss << "Student ID: " << id << "\n"
        << "Name: " << name << "\n"
        << "Department: " << department << " (Sem " << semester << ")\n"
        << "Phone: " << phone << "\n"
        << "Bus: " << busNumber << ", Pickup: " << pickupPoint << "\n"
        << "Seat: " << (seatNumber > 0 ? std::to_string(seatNumber) : "Unassigned");
    return oss.str();
}

std::string Student::serialize() const {
    // Pipe-delimited: id|name|department|semester|phone|address|busNumber|pickupPoint|seatNumber|email
    std::ostringstream oss;
    oss << id << "|" << name << "|" << department << "|" << semester << "|"
        << phone << "|" << address << "|" << busNumber << "|" << pickupPoint << "|"
        << seatNumber << "|" << email;
    return oss.str();
}

Student Student::deserialize(const std::string& line) {
    std::vector<std::string> tokens;
    std::stringstream ss(line);
    std::string token;
    while (std::getline(ss, token, '|')) {
        tokens.push_back(token);
    }

    if (tokens.size() < 9) {
        return Student();
    }

    int sem = 1;
    int seat = 0;
    try {
        sem = std::stoi(tokens[3]);
    } catch (...) { sem = 1; }

    try {
        seat = std::stoi(tokens[8]);
    } catch (...) { seat = 0; }

    std::string email = (tokens.size() >= 10) ? tokens[9] : "";

    return Student(tokens[0], tokens[1], tokens[2], sem, tokens[4],
                   tokens[5], tokens[6], tokens[7], seat, email);
}
