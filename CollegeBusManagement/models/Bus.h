#ifndef BUS_H
#define BUS_H

#include <string>
#include <sstream>
#include <vector>

/**
 * @brief Bus model representing college transportation vehicles.
 * 
 * Demonstrates OOP Concepts:
 * 1. Encapsulation - Private data fields, validation logic for capacity and seats.
 * 2. Constructors & Destructor.
 * 3. Schedule tracking: Full Day vs Half Day shifts.
 * 4. GPS / Location tracking for live map integration.
 */
class Bus {
private:
    std::string busId;
    std::string regNumber;
    int capacity;
    int occupiedSeats;
    std::string assignedDriver;
    std::string routeId;
    std::string status;        // "Active", "Maintenance", "Inactive"
    std::string shiftType;     // "Full Day", "Half Day (Morning)", "Half Day (Evening)"
    std::string currentLocation; // Current stop or live location

public:
    Bus();
    Bus(const std::string& id, const std::string& reg, int cap,
        int occupied = 0, const std::string& driver = "None",
        const std::string& route = "None", const std::string& stat = "Active",
        const std::string& shift = "Full Day",
        const std::string& location = "College Campus");

    // Getters
    std::string getBusId() const { return busId; }
    std::string getRegNumber() const { return regNumber; }
    int getCapacity() const { return capacity; }
    int getOccupiedSeats() const { return occupiedSeats; }
    int getAvailableSeats() const { return capacity - occupiedSeats; }
    std::string getAssignedDriver() const { return assignedDriver; }
    std::string getRouteId() const { return routeId; }
    std::string getStatus() const { return status; }
    std::string getShiftType() const { return shiftType; }
    std::string getCurrentLocation() const { return currentLocation; }

    // Setters
    void setBusId(const std::string& id) { busId = id; }
    void setRegNumber(const std::string& reg) { regNumber = reg; }
    void setCapacity(int cap) { capacity = cap; }
    void setOccupiedSeats(int occ) { occupiedSeats = occ; }
    void setAssignedDriver(const std::string& driver) { assignedDriver = driver; }
    void setRouteId(const std::string& route) { routeId = route; }
    void setStatus(const std::string& stat) { status = stat; }
    void setShiftType(const std::string& shift) { shiftType = shift; }
    void setCurrentLocation(const std::string& loc) { currentLocation = loc; }

    // Business Logic
    bool isFull() const { return occupiedSeats >= capacity; }
    bool canAllocate() const { return occupiedSeats < capacity && status == "Active"; }
    bool allocateSeat();
    bool releaseSeat();

    // Serialization for File Handling
    std::string serialize() const;
    static Bus deserialize(const std::string& line);
};

#endif // BUS_H
