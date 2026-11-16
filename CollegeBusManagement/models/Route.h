#ifndef ROUTE_H
#define ROUTE_H

#include <string>
#include <vector>
#include <sstream>

/**
 * @brief Route model representing bus transit routes with multiple stops.
 */
class Route {
private:
    std::string routeId;
    std::string routeName;
    std::string startPoint;
    std::string endPoint;
    std::vector<std::string> stops;
    std::string pickupTime;
    std::string dropTime;
    std::string assignedBus;

public:
    Route();
    Route(const std::string& id, const std::string& name,
          const std::string& start, const std::string& end,
          const std::vector<std::string>& stpList,
          const std::string& pTime = "07:30 AM",
          const std::string& dTime = "05:00 PM",
          const std::string& bus = "None");

    // Getters
    std::string getRouteId() const { return routeId; }
    std::string getRouteName() const { return routeName; }
    std::string getStartPoint() const { return startPoint; }
    std::string getEndPoint() const { return endPoint; }
    std::vector<std::string> getStops() const { return stops; }
    std::string getStopsAsString() const;
    std::string getPickupTime() const { return pickupTime; }
    std::string getDropTime() const { return dropTime; }
    std::string getAssignedBus() const { return assignedBus; }

    // Setters
    void setRouteId(const std::string& id) { routeId = id; }
    void setRouteName(const std::string& name) { routeName = name; }
    void setStartPoint(const std::string& start) { startPoint = start; }
    void setEndPoint(const std::string& end) { endPoint = end; }
    void setStops(const std::vector<std::string>& stps) { stops = stps; }
    void setStopsFromString(const std::string& stopStr);
    void setPickupTime(const std::string& pTime) { pickupTime = pTime; }
    void setDropTime(const std::string& dTime) { dropTime = dTime; }
    void setAssignedBus(const std::string& bus) { assignedBus = bus; }

    // Helpers
    void addStop(const std::string& stop);

    // Serialization
    std::string serialize() const;
    static Route deserialize(const std::string& line);
};

#endif // ROUTE_H
