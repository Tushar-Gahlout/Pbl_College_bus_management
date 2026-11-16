#include "Seat.h"

Seat::Seat() : busId(""), seatNumber(0), studentId(""), studentName(""), allocationDate("") {}

Seat::Seat(const std::string& bId, int seatNum, const std::string& sId,
           const std::string& sName, const std::string& date)
    : busId(bId), seatNumber(seatNum), studentId(sId),
      studentName(sName), allocationDate(date) {}

std::string Seat::serialize() const {
    // Pipe-delimited: busId|seatNumber|studentId|studentName|allocationDate
    std::ostringstream oss;
    oss << busId << "|" << seatNumber << "|" << studentId << "|"
        << studentName << "|" << allocationDate;
    return oss.str();
}

Seat Seat::deserialize(const std::string& line) {
    std::vector<std::string> tokens;
    std::stringstream ss(line);
    std::string token;
    while (std::getline(ss, token, '|')) {
        tokens.push_back(token);
    }

    if (tokens.size() < 3) {
        return Seat();
    }

    int seatNum = 0;
    try {
        seatNum = std::stoi(tokens[1]);
    } catch (...) { seatNum = 0; }

    std::string name = (tokens.size() >= 4) ? tokens[3] : "";
    std::string date = (tokens.size() >= 5) ? tokens[4] : "";

    return Seat(tokens[0], seatNum, tokens[2], name, date);
}
