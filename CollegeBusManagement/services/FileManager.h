#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include "../models/Student.h"
#include "../models/Driver.h"
#include "../models/Bus.h"
#include "../models/Route.h"
#include "../models/Seat.h"
#include "../models/Attendance.h"
#include "../models/FeeRecord.h"
#include "../models/MaintenanceRecord.h"
#include "../models/User.h"
#include "Exceptions.h"

#include <vector>
#include <string>
#include <memory>
#include <fstream>

/**
 * @brief FileManager class handling all persistent text-file operations.
 * 
 * Demonstrates:
 * 1. Strict C++ File Handling using std::ifstream, std::ofstream, std::fstream.
 * 2. ios::app for appending new records.
 * 3. Temporary file operations (temp_*.txt) for safe updating and deletion.
 * 4. Function Overloading for varied search queries.
 * 5. Exception Handling on I/O failures.
 */
class FileManager {
private:
    std::string dataDirectory;

    // Helper file paths
    std::string getStudentsFilePath() const;
    std::string getBusesFilePath() const;
    std::string getDriversFilePath() const;
    std::string getRoutesFilePath() const;
    std::string getAttendanceFilePath() const;
    std::string getFeesFilePath() const;
    std::string getSeatsFilePath() const;
    std::string getMaintenanceFilePath() const;
    std::string getUsersFilePath() const;

    // Atomic / Safe replace using temporary file
    bool replaceFileWithTemp(const std::string& origPath, const std::string& tempPath);

public:
    explicit FileManager(const std::string& dataDir = "data");
    ~FileManager() = default;

    // Ensure data directory exists and initialize default seed data if empty
    void initializeDataFiles();

    // ==========================================
    // STUDENT OPERATIONS
    // ==========================================
    std::vector<Student> getAllStudents();
    bool addStudent(const Student& student);
    bool updateStudent(const Student& student);
    bool deleteStudent(const std::string& studentId);
    Student getStudentById(const std::string& studentId);

    // Function Overloading (OOP Concept)
    std::vector<Student> searchStudents(const std::string& query);
    std::vector<Student> searchStudents(const std::string& department, int semester);

    // ==========================================
    // BUS OPERATIONS
    // ==========================================
    std::vector<Bus> getAllBuses();
    bool addBus(const Bus& bus);
    bool updateBus(const Bus& bus);
    bool deleteBus(const std::string& busId);
    Bus getBusById(const std::string& busId);
    std::vector<Bus> searchBuses(const std::string& query);

    // ==========================================
    // DRIVER OPERATIONS
    // ==========================================
    std::vector<Driver> getAllDrivers();
    bool addDriver(const Driver& driver);
    bool updateDriver(const Driver& driver);
    bool deleteDriver(const std::string& driverId);
    Driver getDriverById(const std::string& driverId);
    std::vector<Driver> searchDrivers(const std::string& query);

    // ==========================================
    // ROUTE OPERATIONS
    // ==========================================
    std::vector<Route> getAllRoutes();
    bool addRoute(const Route& route);
    bool updateRoute(const Route& route);
    bool deleteRoute(const std::string& routeId);
    Route getRouteById(const std::string& routeId);
    std::vector<Route> searchRoutes(const std::string& query);

    // ==========================================
    // SEAT OPERATIONS
    // ==========================================
    std::vector<Seat> getAllSeats();
    std::vector<Seat> getSeatsForBus(const std::string& busId);
    std::vector<int> getEmptySeatsForBus(const std::string& busId);
    bool allocateSeat(const std::string& busId, int seatNum, const std::string& studentId, const std::string& studentName);
    bool quickBookEmptySeat(const std::string& busId, int seatNum, const std::string& passengerName, const std::string& contactOrId, const std::string& travelType = "Day Pass");
    bool releaseSeat(const std::string& busId, int seatNum);
    bool releaseSeatForStudent(const std::string& studentId);
    Seat getSeatForStudent(const std::string& studentId);

    // ==========================================
    // ATTENDANCE OPERATIONS
    // ==========================================
    std::vector<Attendance> getAllAttendance();
    std::vector<Attendance> getAttendanceByBusAndDate(const std::string& busId, const std::string& date);
    std::vector<Attendance> getAttendanceByStudent(const std::string& studentId);
    bool markAttendance(const std::vector<Attendance>& records);

    // ==========================================
    // FEE OPERATIONS
    // ==========================================
    std::vector<FeeRecord> getAllFees();
    FeeRecord getFeeByStudentId(const std::string& studentId);
    bool addFeeRecord(const FeeRecord& fee);
    bool updateFeeRecord(const FeeRecord& fee);
    std::vector<FeeRecord> searchFees(const std::string& query);

    // Function Overloading (OOP Concept)
    bool recordPayment(const std::string& studentId, double amount, const std::string& date, const std::string& method, const std::string& receipt);
    bool recordPayment(const std::string& studentId, double amount); // Overloaded with defaults

    // ==========================================
    // MAINTENANCE OPERATIONS
    // ==========================================
    std::vector<MaintenanceRecord> getAllMaintenance();
    std::vector<MaintenanceRecord> getMaintenanceForBus(const std::string& busId);
    bool addMaintenanceRecord(const MaintenanceRecord& record);
    bool updateMaintenanceRecord(const MaintenanceRecord& record);
    bool deleteMaintenanceRecord(const std::string& recordId);

    // ==========================================
    // USER AUTHENTICATION
    // ==========================================
    std::vector<User> getAllUsers();
    bool addUser(const User& user);
    User authenticate(const std::string& username, const std::string& password);

    // ==========================================
    // CSV EXPORTS (C++ FILE HANDLING)
    // ==========================================
    bool exportStudentsToCSV(const std::string& destinationFile);
    bool exportBusesToCSV(const std::string& destinationFile);
    bool exportAttendanceToCSV(const std::string& destinationFile);
    bool exportFeesToCSV(const std::string& destinationFile);
    bool exportMaintenanceToCSV(const std::string& destinationFile);
    bool exportRouteWiseStudentsToCSV(const std::string& routeId, const std::string& destinationFile);
};

#endif // FILEMANAGER_H
