#ifndef SEAT_H
#define SEAT_H

#include <string>
#include <sstream>
#include <vector>

/**
 * @brief Seat model representing seat allocation in buses.
 */
class Seat {
private:
    std::string busId;
    int seatNumber;
    std::string studentId;
    std::string studentName;
    std::string allocationDate;

public:
    Seat();
    Seat(const std::string& bId, int seatNum, const std::string& sId,
         const std::string& sName = "", const std::string& date = "");

    // Getters
    std::string getBusId() const { return busId; }
    int getSeatNumber() const { return seatNumber; }
    std::string getStudentId() const { return studentId; }
    std::string getStudentName() const { return studentName; }
    std::string getAllocationDate() const { return allocationDate; }
    bool isOccupied() const { return !studentId.empty() && studentId != "Available"; }

    // Setters
    void setBusId(const std::string& bId) { busId = bId; }
    void setSeatNumber(int num) { seatNumber = num; }
    void setStudentId(const std::string& sId) { studentId = sId; }
    void setStudentName(const std::string& sName) { studentName = sName; }
    void setAllocationDate(const std::string& date) { allocationDate = date; }

    // Serialization
    std::string serialize() const;
    static Seat deserialize(const std::string& line);
};

#endif // SEAT_H
