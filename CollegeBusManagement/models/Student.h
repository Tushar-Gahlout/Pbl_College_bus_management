#ifndef STUDENT_H
#define STUDENT_H

#include "Person.h"
#include <sstream>
#include <vector>

/**
 * @brief Student class derived from Person.
 * 
 * Demonstrates OOP Concepts:
 * 1. Inheritance - Inherits id, name, phone, email from Person.
 * 2. Polymorphism - Overrides getRole(), getDetails(), and serialize().
 * 3. Encapsulation - Private member variables with getters/setters.
 */
class Student : public Person {
private:
    std::string department;
    int semester;
    std::string address;
    std::string busNumber;
    std::string pickupPoint;
    int seatNumber;

public:
    // Default Constructor
    Student();

    // Parameterized Constructor
    Student(const std::string& sId, const std::string& sName,
            const std::string& sDept, int sSem,
            const std::string& sPhone, const std::string& sAddress,
            const std::string& sBusNum = "None",
            const std::string& sPickup = "None",
            int sSeat = 0,
            const std::string& sEmail = "");

    // Getters
    std::string getDepartment() const { return department; }
    int getSemester() const { return semester; }
    std::string getAddress() const { return address; }
    std::string getBusNumber() const { return busNumber; }
    std::string getPickupPoint() const { return pickupPoint; }
    int getSeatNumber() const { return seatNumber; }

    // Setters
    void setDepartment(const std::string& dept) { department = dept; }
    void setSemester(int sem) { semester = sem; }
    void setAddress(const std::string& addr) { address = addr; }
    void setBusNumber(const std::string& busNum) { busNumber = busNum; }
    void setPickupPoint(const std::string& pickup) { pickupPoint = pickup; }
    void setSeatNumber(int seat) { seatNumber = seat; }

    // Polymorphic implementations
    std::string getRole() const override { return "Student"; }
    std::string getDetails() const override;
    std::string serialize() const override;

    // Static deserialization method for file handling
    static Student deserialize(const std::string& line);
};

#endif // STUDENT_H
