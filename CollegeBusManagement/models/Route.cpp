#include "Route.h"
#include <algorithm>

Route::Route()
    : routeId(""), routeName(""), startPoint(""), endPoint(""),
      pickupTime("07:30 AM"), dropTime("05:00 PM"), assignedBus("None") {}

Route::Route(const std::string& id, const std::string& name,
             const std::string& start, const std::string& end,
             const std::vector<std::string>& stpList,
             const std::string& pTime, const std::string& dTime,
             const std::string& bus)
    : routeId(id), routeName(name), startPoint(start), endPoint(end),
      stops(stpList), pickupTime(pTime), dropTime(dTime),
      assignedBus(bus.empty() ? "None" : bus) {}

std::string Route::getStopsAsString() const {
    std::ostringstream oss;
    for (size_t i = 0; i < stops.size(); ++i) {
        oss << stops[i];
        if (i + 1 < stops.size()) {
            oss << ", ";
        }
    }
    return oss.str();
}

void Route::setStopsFromString(const std::string& stopStr) {
    stops.clear();
    std::stringstream ss(stopStr);
    std::string item;
    while (std::getline(ss, item, ';')) {
        // Trim leading/trailing whitespace
        size_t first = item.find_first_not_of(" \t\r\n");
        if (first != std::string::npos) {
            size_t last = item.find_last_not_of(" \t\r\n");
            stops.push_back(item.substr(first, last - first + 1));
        }
    }
    if (stops.empty() && !stopStr.empty()) {
        stops.push_back(stopStr);
    }
}

void Route::addStop(const std::string& stop) {
    stops.push_back(stop);
}

std::string Route::serialize() const {
    // Pipe-delimited: routeId|routeName|startPoint|endPoint|stops (semicolon separated)|pickupTime|dropTime|assignedBus
    std::ostringstream stopsStream;
    for (size_t i = 0; i < stops.size(); ++i) {
        stopsStream << stops[i];
        if (i + 1 < stops.size()) stopsStream << ";";
    }

    std::ostringstream oss;
    oss << routeId << "|" << routeName << "|" << startPoint << "|" << endPoint << "|"
        << stopsStream.str() << "|" << pickupTime << "|" << dropTime << "|" << assignedBus;
    return oss.str();
}

Route Route::deserialize(const std::string& line) {
    std::vector<std::string> tokens;
    std::stringstream ss(line);
    std::string token;
    while (std::getline(ss, token, '|')) {
        tokens.push_back(token);
    }

    if (tokens.size() < 7) {
        return Route();
    }

    std::vector<std::string> stopList;
    std::stringstream stopStream(tokens[4]);
    std::string s;
    while (std::getline(stopStream, s, ';')) {
        if (!s.empty()) stopList.push_back(s);
    }

    std::string bus = (tokens.size() >= 8) ? tokens[7] : "None";
    return Route(tokens[0], tokens[1], tokens[2], tokens[3], stopList,
                 tokens[5], tokens[6], bus);
}
