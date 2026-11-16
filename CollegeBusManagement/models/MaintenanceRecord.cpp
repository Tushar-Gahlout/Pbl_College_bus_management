#include "MaintenanceRecord.h"
#include <iomanip>

MaintenanceRecord::MaintenanceRecord()
    : recordId(""), busId(""), serviceType(""), cost(0.0),
      serviceDate(""), workshop("College Workshop"), remarks("") {}

MaintenanceRecord::MaintenanceRecord(const std::string& rId, const std::string& bId,
                                     const std::string& sType, double c,
                                     const std::string& sDate, const std::string& wShop,
                                     const std::string& rem)
    : recordId(rId), busId(bId), serviceType(sType), cost(c),
      serviceDate(sDate), workshop(wShop), remarks(rem) {}

std::string MaintenanceRecord::serialize() const {
    // Pipe-delimited: recordId|busId|serviceType|cost|serviceDate|workshop|remarks
    std::ostringstream oss;
    oss << recordId << "|" << busId << "|" << serviceType << "|"
        << std::fixed << std::setprecision(2) << cost << "|"
        << serviceDate << "|" << workshop << "|" << remarks;
    return oss.str();
}

MaintenanceRecord MaintenanceRecord::deserialize(const std::string& line) {
    std::vector<std::string> tokens;
    std::stringstream ss(line);
    std::string token;
    while (std::getline(ss, token, '|')) {
        tokens.push_back(token);
    }

    if (tokens.size() < 5) {
        return MaintenanceRecord();
    }

    double c = 0.0;
    try { c = std::stod(tokens[3]); } catch (...) { c = 0.0; }

    std::string shop = (tokens.size() >= 6) ? tokens[5] : "College Workshop";
    std::string rem = (tokens.size() >= 7) ? tokens[6] : "";

    return MaintenanceRecord(tokens[0], tokens[1], tokens[2], c, tokens[4], shop, rem);
}
