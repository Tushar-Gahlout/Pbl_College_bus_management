#include "Bus.h"
#include <sstream>
#include <vector>

Bus::Bus()
    : busId(""), regNumber(""), capacity(40), occupiedSeats(0),
      assignedDriver("None"), routeId("None"), status("Active"),
      shiftType("Full Day"), currentLocation("College Campus") {}

Bus::Bus(const std::string& id, const std::string& reg, int cap,
         int occupied, const std::string& driver,
         const std::string& route, const std::string& stat,
         const std::string& shift, const std::string& location)
    : busId(id), regNumber(reg), capacity(cap),
      occupiedSeats(occupied),
      assignedDriver(driver.empty() ? "None" : driver),
      routeId(route.empty() ? "None" : route),
      status(stat.empty() ? "Active" : stat),
      shiftType(shift.empty() ? "Full Day" : shift),
      currentLocation(location.empty() ? "College Campus" : location) {}

bool Bus::allocateSeat() {
    if (occupiedSeats < capacity) {
        occupiedSeats++;
        return true;
    }
    return false;
}

bool Bus::releaseSeat() {
    if (occupiedSeats > 0) {
        occupiedSeats--;
        return true;
    }
    return false;
}

std::string Bus::serialize() const {
    // Pipe-delimited: busId|regNumber|capacity|occupiedSeats|assignedDriver|routeId|status|shiftType|currentLocation
    std::ostringstream oss;
    oss << busId << "|" << regNumber << "|" << capacity << "|" << occupiedSeats << "|"
        << assignedDriver << "|" << routeId << "|" << status << "|"
        << shiftType << "|" << currentLocation;
    return oss.str();
}

Bus Bus::deserialize(const std::string& line) {
    std::vector<std::string> tokens;
    std::stringstream ss(line);
    std::string token;
    while (std::getline(ss, token, '|')) {
        tokens.push_back(token);
    }

    if (tokens.size() < 7) {
        return Bus();
    }

    int cap = 40;
    int occ = 0;
    try {
        cap = std::stoi(tokens[2]);
    } catch (...) { cap = 40; }

    try {
        occ = std::stoi(tokens[3]);
    } catch (...) { occ = 0; }

    std::string shift = (tokens.size() >= 8 && !tokens[7].empty()) ? tokens[7] : "Full Day";
    std::string loc = (tokens.size() >= 9 && !tokens[8].empty()) ? tokens[8] : "College Campus";

    return Bus(tokens[0], tokens[1], cap, occ, tokens[4], tokens[5], tokens[6], shift, loc);
}
