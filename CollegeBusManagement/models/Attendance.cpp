#include "Attendance.h"

Attendance::Attendance()
    : recordId(""), date(""), busId(""), studentId(""),
      studentName(""), status("Present"), remarks("") {}

Attendance::Attendance(const std::string& recId, const std::string& attDate,
                       const std::string& bId, const std::string& sId,
                       const std::string& sName, const std::string& stat,
                       const std::string& rem)
    : recordId(recId), date(attDate), busId(bId), studentId(sId),
      studentName(sName), status(stat), remarks(rem) {}

std::string Attendance::serialize() const {
    // Pipe-delimited: recordId|date|busId|studentId|studentName|status|remarks
    std::ostringstream oss;
    oss << recordId << "|" << date << "|" << busId << "|" << studentId << "|"
        << studentName << "|" << status << "|" << remarks;
    return oss.str();
}

Attendance Attendance::deserialize(const std::string& line) {
    std::vector<std::string> tokens;
    std::stringstream ss(line);
    std::string token;
    while (std::getline(ss, token, '|')) {
        tokens.push_back(token);
    }

    if (tokens.size() < 6) {
        return Attendance();
    }

    std::string rem = (tokens.size() >= 7) ? tokens[6] : "";
    return Attendance(tokens[0], tokens[1], tokens[2], tokens[3],
                      tokens[4], tokens[5], rem);
}
