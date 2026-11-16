#include "FileManager.h"
#include <iostream>
#include <filesystem>
#include <algorithm>
#include <chrono>
#include <ctime>
#include <iomanip>

namespace fs = std::filesystem;

FileManager::FileManager(const std::string& dataDir) : dataDirectory(dataDir) {
    initializeDataFiles();
}

std::string FileManager::getStudentsFilePath() const { return dataDirectory + "/students.txt"; }
std::string FileManager::getBusesFilePath() const { return dataDirectory + "/buses.txt"; }
std::string FileManager::getDriversFilePath() const { return dataDirectory + "/drivers.txt"; }
std::string FileManager::getRoutesFilePath() const { return dataDirectory + "/routes.txt"; }
std::string FileManager::getAttendanceFilePath() const { return dataDirectory + "/attendance.txt"; }
std::string FileManager::getFeesFilePath() const { return dataDirectory + "/fees.txt"; }
std::string FileManager::getSeatsFilePath() const { return dataDirectory + "/seats.txt"; }
std::string FileManager::getMaintenanceFilePath() const { return dataDirectory + "/maintenance.txt"; }
std::string FileManager::getUsersFilePath() const { return dataDirectory + "/users.txt"; }

bool FileManager::replaceFileWithTemp(const std::string& origPath, const std::string& tempPath) {
    try {
        if (fs::exists(origPath)) {
            fs::remove(origPath);
        }
        fs::rename(tempPath, origPath);
        return true;
    } catch (const std::exception& e) {
        // Fallback copy if rename across filesystems fails
        std::ifstream src(tempPath, std::ios::binary);
        std::ofstream dst(origPath, std::ios::binary | std::ios::trunc);
        if (src.is_open() && dst.is_open()) {
            dst << src.rdbuf();
            src.close();
            dst.close();
            fs::remove(tempPath);
            return true;
        }
    }
    return false;
}

void FileManager::initializeDataFiles() {
    // 1. Ensure directory exists
    try {
        if (!fs::exists(dataDirectory)) {
            fs::create_directories(dataDirectory);
        }
    } catch (...) {}

    // 2. Initialize users.txt if not exists or empty
    std::string usersPath = getUsersFilePath();
    bool needUsers = false;
    if (!fs::exists(usersPath) || fs::file_size(usersPath) == 0) {
        needUsers = true;
    }
    if (needUsers) {
        std::ofstream uOut(usersPath, std::ios::out | std::ios::trunc);
        if (uOut.is_open()) {
            // username|password|role|associatedId|fullName
            uOut << "admin|admin123|Admin||Administrator\n";
            uOut << "student1|pass123|Student|STU101|Rahul Sharma\n";
            uOut << "student2|pass123|Student|STU102|Priya Patel\n";
            uOut << "driver1|pass123|Driver|DRV01|Rajesh Kumar\n";
            uOut << "driver2|pass123|Driver|DRV02|Vikram Singh\n";
            uOut.close();
        }
    }

    // 3. Initialize buses.txt if empty
    std::string busesPath = getBusesFilePath();
    if (!fs::exists(busesPath) || fs::file_size(busesPath) == 0) {
        std::ofstream bOut(busesPath, std::ios::out | std::ios::trunc);
        if (bOut.is_open()) {
            // busId|regNumber|capacity|occupiedSeats|assignedDriver|routeId|status
            bOut << "BUS01|DL-01-AB-1001|40|2|Rajesh Kumar|R01|Active\n";
            bOut << "BUS02|DL-01-CD-2002|35|1|Vikram Singh|R02|Active\n";
            bOut << "BUS03|DL-01-EF-3003|45|0|None|R03|Active\n";
            bOut << "BUS04|DL-01-GH-4004|30|0|None|None|Maintenance\n";
            bOut.close();
        }
    }

    // 4. Initialize drivers.txt if empty
    std::string driversPath = getDriversFilePath();
    if (!fs::exists(driversPath) || fs::file_size(driversPath) == 0) {
        std::ofstream dOut(driversPath, std::ios::out | std::ios::trunc);
        if (dOut.is_open()) {
            // id|name|phone|licenseNumber|assignedBus|experienceYears|email
            dOut << "DRV01|Rajesh Kumar|9876543210|DL-2015-998877|BUS01|8|rajesh@college.edu\n";
            dOut << "DRV02|Vikram Singh|9811223344|DL-2018-445566|BUS02|5|vikram@college.edu\n";
            dOut << "DRV03|Mahesh Verma|9822334455|DL-2012-112233|None|12|mahesh@college.edu\n";
            dOut.close();
        }
    }

    // 5. Initialize routes.txt if empty
    std::string routesPath = getRoutesFilePath();
    if (!fs::exists(routesPath) || fs::file_size(routesPath) == 0) {
        std::ofstream rOut(routesPath, std::ios::out | std::ios::trunc);
        if (rOut.is_open()) {
            // routeId|routeName|startPoint|endPoint|stops (semicolon)|pickupTime|dropTime|assignedBus
            rOut << "R01|North City Express|City Centre|College Campus|City Centre;Metro Gate 3;Sector 14;Tech Hub;College|07:30 AM|05:00 PM|BUS01\n";
            rOut << "R02|South Suburban Line|South Extension|College Campus|South Extension;Green Park;Ring Road;University Junction;College|07:15 AM|05:15 PM|BUS02\n";
            rOut << "R03|East River Route|East Crossing|College Campus|East Crossing;Civil Lines;River Bridge;College|07:45 AM|04:45 PM|BUS03\n";
            rOut.close();
        }
    }

    // 6. Initialize students.txt if empty
    std::string studentsPath = getStudentsFilePath();
    if (!fs::exists(studentsPath) || fs::file_size(studentsPath) == 0) {
        std::ofstream sOut(studentsPath, std::ios::out | std::ios::trunc);
        if (sOut.is_open()) {
            // id|name|department|semester|phone|address|busNumber|pickupPoint|seatNumber|email
            sOut << "STU101|Rahul Sharma|Computer Science|5|9876500001|12/B Baker Street|BUS01|Sector 14|1|rahul@college.edu\n";
            sOut << "STU102|Priya Patel|Electronics|3|9876500002|45 Lake View Residency|BUS01|Metro Gate 3|2|priya@college.edu\n";
            sOut << "STU103|Amit Verma|Mechanical|7|9876500003|88 Model Town|BUS02|Green Park|1|amit@college.edu\n";
            sOut << "STU104|Sneha Gupta|Information Tech|4|9876500004|202 Sunrise Apartments|None|None|0|sneha@college.edu\n";
            sOut.close();
        }
    }

    // 7. Initialize seats.txt if empty
    std::string seatsPath = getSeatsFilePath();
    if (!fs::exists(seatsPath) || fs::file_size(seatsPath) == 0) {
        std::ofstream stOut(seatsPath, std::ios::out | std::ios::trunc);
        if (stOut.is_open()) {
            // busId|seatNumber|studentId|studentName|allocationDate
            stOut << "BUS01|1|STU101|Rahul Sharma|2026-11-01\n";
            stOut << "BUS01|2|STU102|Priya Patel|2026-11-02\n";
            stOut << "BUS02|1|STU103|Amit Verma|2026-11-03\n";
            stOut.close();
        }
    }

    // 8. Initialize fees.txt if empty
    std::string feesPath = getFeesFilePath();
    if (!fs::exists(feesPath) || fs::file_size(feesPath) == 0) {
        std::ofstream fOut(feesPath, std::ios::out | std::ios::trunc);
        if (fOut.is_open()) {
            // feeId|studentId|studentName|totalAmount|paidAmount|paymentDate|paymentMethod|receiptNumber
            fOut << "FEE001|STU101|Rahul Sharma|15000.00|15000.00|2026-11-01|UPI|REC-2026-101\n";
            fOut << "FEE002|STU102|Priya Patel|15000.00|8000.00|2026-11-02|Card|REC-2026-102\n";
            fOut << "FEE003|STU103|Amit Verma|15000.00|15000.00|2026-11-03|Cash|REC-2026-103\n";
            fOut << "FEE004|STU104|Sneha Gupta|15000.00|0.00||Pending|\n";
            fOut.close();
        }
    }

    // 9. Initialize attendance.txt if empty
    std::string attPath = getAttendanceFilePath();
    if (!fs::exists(attPath) || fs::file_size(attPath) == 0) {
        std::ofstream aOut(attPath, std::ios::out | std::ios::trunc);
        if (aOut.is_open()) {
            // recordId|date|busId|studentId|studentName|status|remarks
            aOut << "ATT001|2026-11-14|BUS01|STU101|Rahul Sharma|Present|Boarded at Sector 14\n";
            aOut << "ATT002|2026-11-14|BUS01|STU102|Priya Patel|Present|Boarded at Metro Gate 3\n";
            aOut << "ATT003|2026-11-15|BUS01|STU101|Rahul Sharma|Present|Boarded on time\n";
            aOut << "ATT004|2026-11-15|BUS01|STU102|Priya Patel|Absent|Notified in advance\n";
            aOut.close();
        }
    }

    // 10. Initialize maintenance.txt if empty
    std::string mPath = getMaintenanceFilePath();
    if (!fs::exists(mPath) || fs::file_size(mPath) == 0) {
        std::ofstream mOut(mPath, std::ios::out | std::ios::trunc);
        if (mOut.is_open()) {
            // recordId|busId|serviceType|cost|serviceDate|workshop|remarks
            mOut << "MNT001|BUS01|Oil Change & Filter|4500.00|2026-10-15|Authorized Service Center|Routine 10k km service\n";
            mOut << "MNT002|BUS02|Brake Pad Replacement|8200.00|2026-10-20|City Auto Garage|Replaced front and rear brake pads\n";
            mOut << "MNT003|BUS04|Engine Diagnostics|12500.00|2026-11-05|College Workshop|Major engine overhaul, bus under repair\n";
            mOut.close();
        }
    }
}

// ==========================================
// STUDENT OPERATIONS
// ==========================================
std::vector<Student> FileManager::getAllStudents() {
    std::vector<Student> list;
    std::ifstream in(getStudentsFilePath());
    if (!in.is_open()) return list;

    std::string line;
    while (std::getline(in, line)) {
        if (!line.empty()) {
            Student s = Student::deserialize(line);
            if (!s.getId().empty()) {
                list.push_back(s);
            }
        }
    }
    in.close();
    return list;
}

bool FileManager::addStudent(const Student& student) {
    if (student.getId().empty()) {
        throw ValidationException("Student ID", "ID cannot be empty");
    }

    // Check duplicate ID
    std::vector<Student> all = getAllStudents();
    for (const auto& existing : all) {
        if (existing.getId() == student.getId()) {
            throw DuplicateIdException(student.getId(), "Student");
        }
    }

    // Check bus capacity if assigned
    if (student.getBusNumber() != "None" && !student.getBusNumber().empty()) {
        Bus b = getBusById(student.getBusNumber());
        if (!b.getBusId().empty() && b.isFull()) {
            throw BusFullException(b.getBusId(), b.getCapacity());
        }
    }

    // Append to students.txt using ios::app
    std::ofstream out(getStudentsFilePath(), std::ios::app);
    if (!out.is_open()) {
        throw FileIOException(getStudentsFilePath(), "write");
    }
    out << student.serialize() << "\n";
    out.close();

    // If bus is assigned, update bus occupied seats
    if (student.getBusNumber() != "None" && !student.getBusNumber().empty()) {
        Bus b = getBusById(student.getBusNumber());
        if (!b.getBusId().empty()) {
            b.allocateSeat();
            updateBus(b);
        }
        if (student.getSeatNumber() > 0) {
            allocateSeat(student.getBusNumber(), student.getSeatNumber(), student.getId(), student.getName());
        }
    }

    // Automatically ensure a fee record exists for this student
    FeeRecord existingFee = getFeeByStudentId(student.getId());
    if (existingFee.getFeeId().empty()) {
        std::string feeId = "FEE" + student.getId();
        FeeRecord f(feeId, student.getId(), student.getName(), 15000.0, 0.0, "", "Pending", "");
        addFeeRecord(f);
    }

    return true;
}

bool FileManager::updateStudent(const Student& student) {
    if (student.getId().empty()) {
        throw ValidationException("Student ID", "ID cannot be empty");
    }

    std::string origPath = getStudentsFilePath();
    std::string tempPath = dataDirectory + "/temp_students.txt";

    std::ifstream in(origPath);
    std::ofstream out(tempPath, std::ios::out | std::ios::trunc);

    if (!in.is_open() || !out.is_open()) {
        throw FileIOException(origPath, "update");
    }

    Student oldStudent = getStudentById(student.getId());
    bool found = false;
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        Student s = Student::deserialize(line);
        if (s.getId() == student.getId()) {
            out << student.serialize() << "\n";
            found = true;
        } else {
            out << line << "\n";
        }
    }
    in.close();
    out.close();

    if (!found) {
        fs::remove(tempPath);
        throw NotFoundException(student.getId(), "Student");
    }

    replaceFileWithTemp(origPath, tempPath);

    // Handle bus assignment changes
    if (oldStudent.getBusNumber() != student.getBusNumber()) {
        if (oldStudent.getBusNumber() != "None" && !oldStudent.getBusNumber().empty()) {
            Bus oldB = getBusById(oldStudent.getBusNumber());
            if (!oldB.getBusId().empty()) {
                oldB.releaseSeat();
                updateBus(oldB);
            }
            releaseSeatForStudent(student.getId());
        }
        if (student.getBusNumber() != "None" && !student.getBusNumber().empty()) {
            Bus newB = getBusById(student.getBusNumber());
            if (!newB.getBusId().empty()) {
                newB.allocateSeat();
                updateBus(newB);
            }
            if (student.getSeatNumber() > 0) {
                allocateSeat(student.getBusNumber(), student.getSeatNumber(), student.getId(), student.getName());
            }
        }
    } else if (oldStudent.getSeatNumber() != student.getSeatNumber()) {
        if (student.getBusNumber() != "None" && !student.getBusNumber().empty()) {
            releaseSeatForStudent(student.getId());
            if (student.getSeatNumber() > 0) {
                allocateSeat(student.getBusNumber(), student.getSeatNumber(), student.getId(), student.getName());
            }
        }
    }

    return true;
}

bool FileManager::deleteStudent(const std::string& studentId) {
    std::string origPath = getStudentsFilePath();
    std::string tempPath = dataDirectory + "/temp_students.txt";

    std::ifstream in(origPath);
    std::ofstream out(tempPath, std::ios::out | std::ios::trunc);

    if (!in.is_open() || !out.is_open()) {
        throw FileIOException(origPath, "delete");
    }

    Student toDelete = getStudentById(studentId);
    bool found = false;
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        Student s = Student::deserialize(line);
        if (s.getId() == studentId) {
            found = true;
            // Skip writing to delete
        } else {
            out << line << "\n";
        }
    }
    in.close();
    out.close();

    if (!found) {
        fs::remove(tempPath);
        throw NotFoundException(studentId, "Student");
    }

    replaceFileWithTemp(origPath, tempPath);

    // Release bus seat and decrease bus occupied count
    if (toDelete.getBusNumber() != "None" && !toDelete.getBusNumber().empty()) {
        Bus b = getBusById(toDelete.getBusNumber());
        if (!b.getBusId().empty()) {
            b.releaseSeat();
            updateBus(b);
        }
        releaseSeatForStudent(studentId);
    }

    return true;
}

Student FileManager::getStudentById(const std::string& studentId) {
    std::ifstream in(getStudentsFilePath());
    if (!in.is_open()) return Student();

    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        Student s = Student::deserialize(line);
        if (s.getId() == studentId) {
            return s;
        }
    }
    return Student();
}

// Function Overload 1: Search by text query across ID, Name, Department, Bus
std::vector<Student> FileManager::searchStudents(const std::string& query) {
    std::vector<Student> results;
    if (query.empty()) return getAllStudents();

    std::string qLower = query;
    std::transform(qLower.begin(), qLower.end(), qLower.begin(), ::tolower);

    std::vector<Student> all = getAllStudents();
    for (const auto& s : all) {
        std::string sId = s.getId();
        std::string sName = s.getName();
        std::string sDept = s.getDepartment();
        std::string sBus = s.getBusNumber();
        std::string sPickup = s.getPickupPoint();

        std::transform(sId.begin(), sId.end(), sId.begin(), ::tolower);
        std::transform(sName.begin(), sName.end(), sName.begin(), ::tolower);
        std::transform(sDept.begin(), sDept.end(), sDept.begin(), ::tolower);
        std::transform(sBus.begin(), sBus.end(), sBus.begin(), ::tolower);
        std::transform(sPickup.begin(), sPickup.end(), sPickup.begin(), ::tolower);

        if (sId.find(qLower) != std::string::npos ||
            sName.find(qLower) != std::string::npos ||
            sDept.find(qLower) != std::string::npos ||
            sBus.find(qLower) != std::string::npos ||
            sPickup.find(qLower) != std::string::npos) {
            results.push_back(s);
        }
    }
    return results;
}

// Function Overload 2: Search by department and semester
std::vector<Student> FileManager::searchStudents(const std::string& department, int semester) {
    std::vector<Student> results;
    std::vector<Student> all = getAllStudents();
    for (const auto& s : all) {
        bool deptMatch = department.empty() || s.getDepartment() == department;
        bool semMatch = (semester <= 0) || (s.getSemester() == semester);
        if (deptMatch && semMatch) {
            results.push_back(s);
        }
    }
    return results;
}

// ==========================================
// BUS OPERATIONS
// ==========================================
std::vector<Bus> FileManager::getAllBuses() {
    std::vector<Bus> list;
    std::ifstream in(getBusesFilePath());
    if (!in.is_open()) return list;

    std::string line;
    while (std::getline(in, line)) {
        if (!line.empty()) {
            Bus b = Bus::deserialize(line);
            if (!b.getBusId().empty()) list.push_back(b);
        }
    }
    in.close();
    return list;
}

bool FileManager::addBus(const Bus& bus) {
    if (bus.getBusId().empty()) {
        throw ValidationException("Bus ID", "ID cannot be empty");
    }

    std::vector<Bus> all = getAllBuses();
    for (const auto& b : all) {
        if (b.getBusId() == bus.getBusId()) {
            throw DuplicateIdException(bus.getBusId(), "Bus");
        }
    }

    std::ofstream out(getBusesFilePath(), std::ios::app);
    if (!out.is_open()) {
        throw FileIOException(getBusesFilePath(), "write");
    }
    out << bus.serialize() << "\n";
    out.close();
    return true;
}

bool FileManager::updateBus(const Bus& bus) {
    std::string origPath = getBusesFilePath();
    std::string tempPath = dataDirectory + "/temp_buses.txt";

    std::ifstream in(origPath);
    std::ofstream out(tempPath, std::ios::out | std::ios::trunc);

    if (!in.is_open() || !out.is_open()) {
        throw FileIOException(origPath, "update");
    }

    bool found = false;
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        Bus b = Bus::deserialize(line);
        if (b.getBusId() == bus.getBusId()) {
            out << bus.serialize() << "\n";
            found = true;
        } else {
            out << line << "\n";
        }
    }
    in.close();
    out.close();

    if (!found) {
        fs::remove(tempPath);
        throw NotFoundException(bus.getBusId(), "Bus");
    }

    replaceFileWithTemp(origPath, tempPath);
    return true;
}

bool FileManager::deleteBus(const std::string& busId) {
    std::string origPath = getBusesFilePath();
    std::string tempPath = dataDirectory + "/temp_buses.txt";

    std::ifstream in(origPath);
    std::ofstream out(tempPath, std::ios::out | std::ios::trunc);

    if (!in.is_open() || !out.is_open()) {
        throw FileIOException(origPath, "delete");
    }

    bool found = false;
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        Bus b = Bus::deserialize(line);
        if (b.getBusId() == busId) {
            found = true;
        } else {
            out << line << "\n";
        }
    }
    in.close();
    out.close();

    if (!found) {
        fs::remove(tempPath);
        throw NotFoundException(busId, "Bus");
    }

    replaceFileWithTemp(origPath, tempPath);
    return true;
}

Bus FileManager::getBusById(const std::string& busId) {
    std::ifstream in(getBusesFilePath());
    if (!in.is_open()) return Bus();

    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        Bus b = Bus::deserialize(line);
        if (b.getBusId() == busId) return b;
    }
    return Bus();
}

std::vector<Bus> FileManager::searchBuses(const std::string& query) {
    std::vector<Bus> results;
    if (query.empty()) return getAllBuses();

    std::string qLower = query;
    std::transform(qLower.begin(), qLower.end(), qLower.begin(), ::tolower);

    std::vector<Bus> all = getAllBuses();
    for (const auto& b : all) {
        std::string bId = b.getBusId();
        std::string reg = b.getRegNumber();
        std::string drv = b.getAssignedDriver();
        std::string stat = b.getStatus();
        std::string rId = b.getRouteId();

        std::transform(bId.begin(), bId.end(), bId.begin(), ::tolower);
        std::transform(reg.begin(), reg.end(), reg.begin(), ::tolower);
        std::transform(drv.begin(), drv.end(), drv.begin(), ::tolower);
        std::transform(stat.begin(), stat.end(), stat.begin(), ::tolower);
        std::transform(rId.begin(), rId.end(), rId.begin(), ::tolower);

        if (bId.find(qLower) != std::string::npos ||
            reg.find(qLower) != std::string::npos ||
            drv.find(qLower) != std::string::npos ||
            stat.find(qLower) != std::string::npos ||
            rId.find(qLower) != std::string::npos) {
            results.push_back(b);
        }
    }
    return results;
}

// ==========================================
// DRIVER OPERATIONS
// ==========================================
std::vector<Driver> FileManager::getAllDrivers() {
    std::vector<Driver> list;
    std::ifstream in(getDriversFilePath());
    if (!in.is_open()) return list;

    std::string line;
    while (std::getline(in, line)) {
        if (!line.empty()) {
            Driver d = Driver::deserialize(line);
            if (!d.getId().empty()) list.push_back(d);
        }
    }
    in.close();
    return list;
}

bool FileManager::addDriver(const Driver& driver) {
    if (driver.getId().empty()) {
        throw ValidationException("Driver ID", "ID cannot be empty");
    }

    std::vector<Driver> all = getAllDrivers();
    for (const auto& d : all) {
        if (d.getId() == driver.getId()) {
            throw DuplicateIdException(driver.getId(), "Driver");
        }
    }

    std::ofstream out(getDriversFilePath(), std::ios::app);
    if (!out.is_open()) {
        throw FileIOException(getDriversFilePath(), "write");
    }
    out << driver.serialize() << "\n";
    out.close();

    // If driver assigned to a bus, update the bus record as well
    if (driver.getAssignedBus() != "None" && !driver.getAssignedBus().empty()) {
        Bus b = getBusById(driver.getAssignedBus());
        if (!b.getBusId().empty()) {
            b.setAssignedDriver(driver.getName());
            updateBus(b);
        }
    }
    return true;
}

bool FileManager::updateDriver(const Driver& driver) {
    std::string origPath = getDriversFilePath();
    std::string tempPath = dataDirectory + "/temp_drivers.txt";

    std::ifstream in(origPath);
    std::ofstream out(tempPath, std::ios::out | std::ios::trunc);

    if (!in.is_open() || !out.is_open()) {
        throw FileIOException(origPath, "update");
    }

    bool found = false;
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        Driver d = Driver::deserialize(line);
        if (d.getId() == driver.getId()) {
            out << driver.serialize() << "\n";
            found = true;
        } else {
            out << line << "\n";
        }
    }
    in.close();
    out.close();

    if (!found) {
        fs::remove(tempPath);
        throw NotFoundException(driver.getId(), "Driver");
    }

    replaceFileWithTemp(origPath, tempPath);

    // Update bus assigned driver if applicable
    if (driver.getAssignedBus() != "None" && !driver.getAssignedBus().empty()) {
        Bus b = getBusById(driver.getAssignedBus());
        if (!b.getBusId().empty()) {
            b.setAssignedDriver(driver.getName());
            updateBus(b);
        }
    }
    return true;
}

bool FileManager::deleteDriver(const std::string& driverId) {
    std::string origPath = getDriversFilePath();
    std::string tempPath = dataDirectory + "/temp_drivers.txt";

    std::ifstream in(origPath);
    std::ofstream out(tempPath, std::ios::out | std::ios::trunc);

    if (!in.is_open() || !out.is_open()) {
        throw FileIOException(origPath, "delete");
    }

    Driver toDelete = getDriverById(driverId);
    bool found = false;
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        Driver d = Driver::deserialize(line);
        if (d.getId() == driverId) {
            found = true;
        } else {
            out << line << "\n";
        }
    }
    in.close();
    out.close();

    if (!found) {
        fs::remove(tempPath);
        throw NotFoundException(driverId, "Driver");
    }

    replaceFileWithTemp(origPath, tempPath);

    // Unassign from bus if was assigned
    if (toDelete.getAssignedBus() != "None" && !toDelete.getAssignedBus().empty()) {
        Bus b = getBusById(toDelete.getAssignedBus());
        if (!b.getBusId().empty() && b.getAssignedDriver() == toDelete.getName()) {
            b.setAssignedDriver("None");
            updateBus(b);
        }
    }
    return true;
}

Driver FileManager::getDriverById(const std::string& driverId) {
    std::ifstream in(getDriversFilePath());
    if (!in.is_open()) return Driver();

    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        Driver d = Driver::deserialize(line);
        if (d.getId() == driverId) return d;
    }
    return Driver();
}

std::vector<Driver> FileManager::searchDrivers(const std::string& query) {
    std::vector<Driver> results;
    if (query.empty()) return getAllDrivers();

    std::string qLower = query;
    std::transform(qLower.begin(), qLower.end(), qLower.begin(), ::tolower);

    std::vector<Driver> all = getAllDrivers();
    for (const auto& d : all) {
        std::string dId = d.getId();
        std::string name = d.getName();
        std::string phone = d.getPhone();
        std::string lic = d.getLicenseNumber();
        std::string bus = d.getAssignedBus();

        std::transform(dId.begin(), dId.end(), dId.begin(), ::tolower);
        std::transform(name.begin(), name.end(), name.begin(), ::tolower);
        std::transform(phone.begin(), phone.end(), phone.begin(), ::tolower);
        std::transform(lic.begin(), lic.end(), lic.begin(), ::tolower);
        std::transform(bus.begin(), bus.end(), bus.begin(), ::tolower);

        if (dId.find(qLower) != std::string::npos ||
            name.find(qLower) != std::string::npos ||
            phone.find(qLower) != std::string::npos ||
            lic.find(qLower) != std::string::npos ||
            bus.find(qLower) != std::string::npos) {
            results.push_back(d);
        }
    }
    return results;
}

// ==========================================
// ROUTE OPERATIONS
// ==========================================
std::vector<Route> FileManager::getAllRoutes() {
    std::vector<Route> list;
    std::ifstream in(getRoutesFilePath());
    if (!in.is_open()) return list;

    std::string line;
    while (std::getline(in, line)) {
        if (!line.empty()) {
            Route r = Route::deserialize(line);
            if (!r.getRouteId().empty()) list.push_back(r);
        }
    }
    in.close();
    return list;
}

bool FileManager::addRoute(const Route& route) {
    if (route.getRouteId().empty()) {
        throw ValidationException("Route ID", "ID cannot be empty");
    }

    std::vector<Route> all = getAllRoutes();
    for (const auto& r : all) {
        if (r.getRouteId() == route.getRouteId()) {
            throw DuplicateIdException(route.getRouteId(), "Route");
        }
    }

    std::ofstream out(getRoutesFilePath(), std::ios::app);
    if (!out.is_open()) {
        throw FileIOException(getRoutesFilePath(), "write");
    }
    out << route.serialize() << "\n";
    out.close();

    // Update assigned bus routeId if bus is specified
    if (route.getAssignedBus() != "None" && !route.getAssignedBus().empty()) {
        Bus b = getBusById(route.getAssignedBus());
        if (!b.getBusId().empty()) {
            b.setRouteId(route.getRouteId());
            updateBus(b);
        }
    }
    return true;
}

bool FileManager::updateRoute(const Route& route) {
    std::string origPath = getRoutesFilePath();
    std::string tempPath = dataDirectory + "/temp_routes.txt";

    std::ifstream in(origPath);
    std::ofstream out(tempPath, std::ios::out | std::ios::trunc);

    if (!in.is_open() || !out.is_open()) {
        throw FileIOException(origPath, "update");
    }

    bool found = false;
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        Route r = Route::deserialize(line);
        if (r.getRouteId() == route.getRouteId()) {
            out << route.serialize() << "\n";
            found = true;
        } else {
            out << line << "\n";
        }
    }
    in.close();
    out.close();

    if (!found) {
        fs::remove(tempPath);
        throw NotFoundException(route.getRouteId(), "Route");
    }

    replaceFileWithTemp(origPath, tempPath);

    if (route.getAssignedBus() != "None" && !route.getAssignedBus().empty()) {
        Bus b = getBusById(route.getAssignedBus());
        if (!b.getBusId().empty()) {
            b.setRouteId(route.getRouteId());
            updateBus(b);
        }
    }
    return true;
}

bool FileManager::deleteRoute(const std::string& routeId) {
    std::string origPath = getRoutesFilePath();
    std::string tempPath = dataDirectory + "/temp_routes.txt";

    std::ifstream in(origPath);
    std::ofstream out(tempPath, std::ios::out | std::ios::trunc);

    if (!in.is_open() || !out.is_open()) {
        throw FileIOException(origPath, "delete");
    }

    bool found = false;
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        Route r = Route::deserialize(line);
        if (r.getRouteId() == routeId) {
            found = true;
        } else {
            out << line << "\n";
        }
    }
    in.close();
    out.close();

    if (!found) {
        fs::remove(tempPath);
        throw NotFoundException(routeId, "Route");
    }

    replaceFileWithTemp(origPath, tempPath);
    return true;
}

Route FileManager::getRouteById(const std::string& routeId) {
    std::ifstream in(getRoutesFilePath());
    if (!in.is_open()) return Route();

    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        Route r = Route::deserialize(line);
        if (r.getRouteId() == routeId) return r;
    }
    return Route();
}

std::vector<Route> FileManager::searchRoutes(const std::string& query) {
    std::vector<Route> results;
    if (query.empty()) return getAllRoutes();

    std::string qLower = query;
    std::transform(qLower.begin(), qLower.end(), qLower.begin(), ::tolower);

    std::vector<Route> all = getAllRoutes();
    for (const auto& r : all) {
        std::string rId = r.getRouteId();
        std::string name = r.getRouteName();
        std::string start = r.getStartPoint();
        std::string end = r.getEndPoint();
        std::string stops = r.getStopsAsString();
        std::string bus = r.getAssignedBus();

        std::transform(rId.begin(), rId.end(), rId.begin(), ::tolower);
        std::transform(name.begin(), name.end(), name.begin(), ::tolower);
        std::transform(start.begin(), start.end(), start.begin(), ::tolower);
        std::transform(end.begin(), end.end(), end.begin(), ::tolower);
        std::transform(stops.begin(), stops.end(), stops.begin(), ::tolower);
        std::transform(bus.begin(), bus.end(), bus.begin(), ::tolower);

        if (rId.find(qLower) != std::string::npos ||
            name.find(qLower) != std::string::npos ||
            start.find(qLower) != std::string::npos ||
            end.find(qLower) != std::string::npos ||
            stops.find(qLower) != std::string::npos ||
            bus.find(qLower) != std::string::npos) {
            results.push_back(r);
        }
    }
    return results;
}

// ==========================================
// SEAT OPERATIONS
// ==========================================
std::vector<Seat> FileManager::getAllSeats() {
    std::vector<Seat> list;
    std::ifstream in(getSeatsFilePath());
    if (!in.is_open()) return list;

    std::string line;
    while (std::getline(in, line)) {
        if (!line.empty()) {
            Seat s = Seat::deserialize(line);
            if (!s.getBusId().empty()) list.push_back(s);
        }
    }
    in.close();
    return list;
}

std::vector<Seat> FileManager::getSeatsForBus(const std::string& busId) {
    std::vector<Seat> list;
    std::vector<Seat> all = getAllSeats();
    for (const auto& s : all) {
        if (s.getBusId() == busId) {
            list.push_back(s);
        }
    }
    return list;
}

std::vector<int> FileManager::getEmptySeatsForBus(const std::string& busId) {
    std::vector<int> emptySeats;
    Bus b = getBusById(busId);
    if (b.getBusId().empty()) return emptySeats;

    auto occupied = getSeatsForBus(busId);
    for (int seatNum = 1; seatNum <= b.getCapacity(); ++seatNum) {
        bool isOccupied = false;
        for (const auto& s : occupied) {
            if (s.getSeatNumber() == seatNum) {
                isOccupied = true;
                break;
            }
        }
        if (!isOccupied) {
            emptySeats.push_back(seatNum);
        }
    }
    return emptySeats;
}

bool FileManager::quickBookEmptySeat(const std::string& busId, int seatNum, const std::string& passengerName, const std::string& contactOrId, const std::string& travelType) {
    if (passengerName.empty() || contactOrId.empty()) {
        throw ValidationException("Booking Details", "Passenger name and contact/ID are required.");
    }

    std::string sId = contactOrId + " (" + travelType + ")";
    bool ok = allocateSeat(busId, seatNum, sId, passengerName);
    if (ok) {
        Bus b = getBusById(busId);
        if (!b.getBusId().empty() && b.getOccupiedSeats() < b.getCapacity()) {
            b.allocateSeat();
            updateBus(b);
        }
    }
    return ok;
}

bool FileManager::allocateSeat(const std::string& busId, int seatNum, const std::string& studentId, const std::string& studentName) {
    // Release any previous seat this student had on any bus
    releaseSeatForStudent(studentId);

    // Get current date
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    std::tm tm_now;
#if defined(_WIN32) || defined(_WIN64)
    localtime_s(&tm_now, &now_time);
#else
    localtime_r(&now_time, &tm_now);
#endif
    std::ostringstream dateStream;
    dateStream << std::put_time(&tm_now, "%Y-%m-%d");
    std::string today = dateStream.str();

    // Check if seat is already occupied by someone else
    std::string origPath = getSeatsFilePath();
    std::string tempPath = dataDirectory + "/temp_seats.txt";

    std::ifstream in(origPath);
    std::ofstream out(tempPath, std::ios::out | std::ios::trunc);

    bool updatedExisting = false;
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        Seat s = Seat::deserialize(line);
        if (s.getBusId() == busId && s.getSeatNumber() == seatNum) {
            Seat newSeat(busId, seatNum, studentId, studentName, today);
            out << newSeat.serialize() << "\n";
            updatedExisting = true;
        } else {
            out << line << "\n";
        }
    }
    in.close();

    if (!updatedExisting) {
        Seat newSeat(busId, seatNum, studentId, studentName, today);
        out << newSeat.serialize() << "\n";
    }
    out.close();

    replaceFileWithTemp(origPath, tempPath);
    return true;
}

bool FileManager::releaseSeat(const std::string& busId, int seatNum) {
    std::string origPath = getSeatsFilePath();
    std::string tempPath = dataDirectory + "/temp_seats.txt";

    std::ifstream in(origPath);
    std::ofstream out(tempPath, std::ios::out | std::ios::trunc);

    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        Seat s = Seat::deserialize(line);
        if (s.getBusId() == busId && s.getSeatNumber() == seatNum) {
            // Remove record
        } else {
            out << line << "\n";
        }
    }
    in.close();
    out.close();

    replaceFileWithTemp(origPath, tempPath);
    return true;
}

bool FileManager::releaseSeatForStudent(const std::string& studentId) {
    if (studentId.empty()) return false;
    std::string origPath = getSeatsFilePath();
    std::string tempPath = dataDirectory + "/temp_seats.txt";

    std::ifstream in(origPath);
    std::ofstream out(tempPath, std::ios::out | std::ios::trunc);

    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        Seat s = Seat::deserialize(line);
        if (s.getStudentId() == studentId) {
            // Skip to remove
        } else {
            out << line << "\n";
        }
    }
    in.close();
    out.close();

    replaceFileWithTemp(origPath, tempPath);
    return true;
}

Seat FileManager::getSeatForStudent(const std::string& studentId) {
    std::vector<Seat> all = getAllSeats();
    for (const auto& s : all) {
        if (s.getStudentId() == studentId) return s;
    }
    return Seat();
}

// ==========================================
// ATTENDANCE OPERATIONS
// ==========================================
std::vector<Attendance> FileManager::getAllAttendance() {
    std::vector<Attendance> list;
    std::ifstream in(getAttendanceFilePath());
    if (!in.is_open()) return list;

    std::string line;
    while (std::getline(in, line)) {
        if (!line.empty()) {
            Attendance a = Attendance::deserialize(line);
            if (!a.getRecordId().empty()) list.push_back(a);
        }
    }
    in.close();
    return list;
}

std::vector<Attendance> FileManager::getAttendanceByBusAndDate(const std::string& busId, const std::string& date) {
    std::vector<Attendance> list;
    std::vector<Attendance> all = getAllAttendance();
    for (const auto& a : all) {
        if (a.getBusId() == busId && (date.empty() || a.getDate() == date)) {
            list.push_back(a);
        }
    }
    return list;
}

std::vector<Attendance> FileManager::getAttendanceByStudent(const std::string& studentId) {
    std::vector<Attendance> list;
    std::vector<Attendance> all = getAllAttendance();
    for (const auto& a : all) {
        if (a.getStudentId() == studentId) {
            list.push_back(a);
        }
    }
    return list;
}

bool FileManager::markAttendance(const std::vector<Attendance>& records) {
    if (records.empty()) return true;

    // Load existing records, replace any matching (busId, date, studentId)
    std::vector<Attendance> existing = getAllAttendance();
    std::string origPath = getAttendanceFilePath();
    std::string tempPath = dataDirectory + "/temp_attendance.txt";

    std::ofstream out(tempPath, std::ios::out | std::ios::trunc);
    if (!out.is_open()) {
        throw FileIOException(origPath, "write");
    }

    // Write existing records that aren't being updated
    for (const auto& oldRec : existing) {
        bool willBeOverwritten = false;
        for (const auto& newRec : records) {
            if (oldRec.getBusId() == newRec.getBusId() &&
                oldRec.getDate() == newRec.getDate() &&
                oldRec.getStudentId() == newRec.getStudentId()) {
                willBeOverwritten = true;
                break;
            }
        }
        if (!willBeOverwritten) {
            out << oldRec.serialize() << "\n";
        }
    }

    // Append new records
    for (const auto& newRec : records) {
        out << newRec.serialize() << "\n";
    }
    out.close();

    replaceFileWithTemp(origPath, tempPath);
    return true;
}

// ==========================================
// FEE OPERATIONS
// ==========================================
std::vector<FeeRecord> FileManager::getAllFees() {
    std::vector<FeeRecord> list;
    std::ifstream in(getFeesFilePath());
    if (!in.is_open()) return list;

    std::string line;
    while (std::getline(in, line)) {
        if (!line.empty()) {
            FeeRecord f = FeeRecord::deserialize(line);
            if (!f.getFeeId().empty()) list.push_back(f);
        }
    }
    in.close();
    return list;
}

FeeRecord FileManager::getFeeByStudentId(const std::string& studentId) {
    std::vector<FeeRecord> all = getAllFees();
    for (const auto& f : all) {
        if (f.getStudentId() == studentId) return f;
    }
    return FeeRecord();
}

bool FileManager::addFeeRecord(const FeeRecord& fee) {
    std::ofstream out(getFeesFilePath(), std::ios::app);
    if (!out.is_open()) {
        throw FileIOException(getFeesFilePath(), "write");
    }
    out << fee.serialize() << "\n";
    out.close();
    return true;
}

bool FileManager::updateFeeRecord(const FeeRecord& fee) {
    std::string origPath = getFeesFilePath();
    std::string tempPath = dataDirectory + "/temp_fees.txt";

    std::ifstream in(origPath);
    std::ofstream out(tempPath, std::ios::out | std::ios::trunc);

    if (!in.is_open() || !out.is_open()) {
        throw FileIOException(origPath, "update");
    }

    bool found = false;
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        FeeRecord f = FeeRecord::deserialize(line);
        if (f.getFeeId() == fee.getFeeId() || f.getStudentId() == fee.getStudentId()) {
            out << fee.serialize() << "\n";
            found = true;
        } else {
            out << line << "\n";
        }
    }
    in.close();
    out.close();

    if (!found) {
        // If not found, add it
        addFeeRecord(fee);
        fs::remove(tempPath);
        return true;
    }

    replaceFileWithTemp(origPath, tempPath);
    return true;
}

std::vector<FeeRecord> FileManager::searchFees(const std::string& query) {
    std::vector<FeeRecord> results;
    if (query.empty()) return getAllFees();

    std::string qLower = query;
    std::transform(qLower.begin(), qLower.end(), qLower.begin(), ::tolower);

    std::vector<FeeRecord> all = getAllFees();
    for (const auto& f : all) {
        std::string sId = f.getStudentId();
        std::string name = f.getStudentName();
        std::string stat = f.getStatus();
        std::string rec = f.getReceiptNumber();

        std::transform(sId.begin(), sId.end(), sId.begin(), ::tolower);
        std::transform(name.begin(), name.end(), name.begin(), ::tolower);
        std::transform(stat.begin(), stat.end(), stat.begin(), ::tolower);
        std::transform(rec.begin(), rec.end(), rec.begin(), ::tolower);

        if (sId.find(qLower) != std::string::npos ||
            name.find(qLower) != std::string::npos ||
            stat.find(qLower) != std::string::npos ||
            rec.find(qLower) != std::string::npos) {
            results.push_back(f);
        }
    }
    return results;
}

// Function Overload 1: recordPayment with detailed params
bool FileManager::recordPayment(const std::string& studentId, double amount,
                                const std::string& date, const std::string& method,
                                const std::string& receipt) {
    FeeRecord f = getFeeByStudentId(studentId);
    if (f.getFeeId().empty()) {
        Student s = getStudentById(studentId);
        f.setFeeId("FEE" + studentId);
        f.setStudentId(studentId);
        f.setStudentName(s.getName().empty() ? studentId : s.getName());
        f.setTotalAmount(15000.0);
        f.setPaidAmount(0.0);
    }
    f.addPayment(amount, date, method);
    f.setReceiptNumber(receipt);
    return updateFeeRecord(f);
}

// Function Overload 2: recordPayment with default date, method and receipt generator
bool FileManager::recordPayment(const std::string& studentId, double amount) {
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    std::tm tm_now;
#if defined(_WIN32) || defined(_WIN64)
    localtime_s(&tm_now, &now_time);
#else
    localtime_r(&now_time, &tm_now);
#endif
    std::ostringstream dateStream;
    dateStream << std::put_time(&tm_now, "%Y-%m-%d");
    std::string today = dateStream.str();
    std::string receipt = "REC-" + today + "-" + studentId;

    return recordPayment(studentId, amount, today, "UPI", receipt);
}

// ==========================================
// MAINTENANCE OPERATIONS
// ==========================================
std::vector<MaintenanceRecord> FileManager::getAllMaintenance() {
    std::vector<MaintenanceRecord> list;
    std::ifstream in(getMaintenanceFilePath());
    if (!in.is_open()) return list;

    std::string line;
    while (std::getline(in, line)) {
        if (!line.empty()) {
            MaintenanceRecord m = MaintenanceRecord::deserialize(line);
            if (!m.getRecordId().empty()) list.push_back(m);
        }
    }
    in.close();
    return list;
}

std::vector<MaintenanceRecord> FileManager::getMaintenanceForBus(const std::string& busId) {
    std::vector<MaintenanceRecord> list;
    std::vector<MaintenanceRecord> all = getAllMaintenance();
    for (const auto& m : all) {
        if (m.getBusId() == busId) list.push_back(m);
    }
    return list;
}

bool FileManager::addMaintenanceRecord(const MaintenanceRecord& record) {
    if (record.getRecordId().empty()) {
        throw ValidationException("Record ID", "ID cannot be empty");
    }

    std::vector<MaintenanceRecord> all = getAllMaintenance();
    for (const auto& m : all) {
        if (m.getRecordId() == record.getRecordId()) {
            throw DuplicateIdException(record.getRecordId(), "Maintenance Record");
        }
    }

    std::ofstream out(getMaintenanceFilePath(), std::ios::app);
    if (!out.is_open()) {
        throw FileIOException(getMaintenanceFilePath(), "write");
    }
    out << record.serialize() << "\n";
    out.close();
    return true;
}

bool FileManager::updateMaintenanceRecord(const MaintenanceRecord& record) {
    std::string origPath = getMaintenanceFilePath();
    std::string tempPath = dataDirectory + "/temp_maintenance.txt";

    std::ifstream in(origPath);
    std::ofstream out(tempPath, std::ios::out | std::ios::trunc);

    if (!in.is_open() || !out.is_open()) {
        throw FileIOException(origPath, "update");
    }

    bool found = false;
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        MaintenanceRecord m = MaintenanceRecord::deserialize(line);
        if (m.getRecordId() == record.getRecordId()) {
            out << record.serialize() << "\n";
            found = true;
        } else {
            out << line << "\n";
        }
    }
    in.close();
    out.close();

    if (!found) {
        fs::remove(tempPath);
        throw NotFoundException(record.getRecordId(), "Maintenance Record");
    }

    replaceFileWithTemp(origPath, tempPath);
    return true;
}

bool FileManager::deleteMaintenanceRecord(const std::string& recordId) {
    std::string origPath = getMaintenanceFilePath();
    std::string tempPath = dataDirectory + "/temp_maintenance.txt";

    std::ifstream in(origPath);
    std::ofstream out(tempPath, std::ios::out | std::ios::trunc);

    if (!in.is_open() || !out.is_open()) {
        throw FileIOException(origPath, "delete");
    }

    bool found = false;
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        MaintenanceRecord m = MaintenanceRecord::deserialize(line);
        if (m.getRecordId() == recordId) {
            found = true;
        } else {
            out << line << "\n";
        }
    }
    in.close();
    out.close();

    if (!found) {
        fs::remove(tempPath);
        throw NotFoundException(recordId, "Maintenance Record");
    }

    replaceFileWithTemp(origPath, tempPath);
    return true;
}

// ==========================================
// USER AUTHENTICATION
// ==========================================
std::vector<User> FileManager::getAllUsers() {
    std::vector<User> list;
    std::ifstream in(getUsersFilePath());
    if (!in.is_open()) return list;

    std::string line;
    while (std::getline(in, line)) {
        if (!line.empty()) {
            User u = User::deserialize(line);
            if (!u.getUsername().empty()) list.push_back(u);
        }
    }
    in.close();
    return list;
}

bool FileManager::addUser(const User& user) {
    if (user.getUsername().empty() || user.getPassword().empty()) {
        throw ValidationException("Credentials", "Username and password cannot be empty");
    }

    std::vector<User> all = getAllUsers();
    for (const auto& u : all) {
        if (u.getUsername() == user.getUsername()) {
            throw DuplicateIdException(user.getUsername(), "Username");
        }
    }

    std::ofstream out(getUsersFilePath(), std::ios::app);
    if (!out.is_open()) {
        throw FileIOException(getUsersFilePath(), "write");
    }
    out << user.serialize() << "\n";
    out.close();
    return true;
}

User FileManager::authenticate(const std::string& username, const std::string& password) {
    std::vector<User> all = getAllUsers();
    for (const auto& u : all) {
        if (u.getUsername() == username && u.validatePassword(password)) {
            return u;
        }
    }
    return User(); // Returns empty user on failed auth
}

// ==========================================
// CSV EXPORTS (C++ FILE HANDLING)
// ==========================================
bool FileManager::exportStudentsToCSV(const std::string& destinationFile) {
    std::ofstream csv(destinationFile, std::ios::out | std::ios::trunc);
    if (!csv.is_open()) return false;

    csv << "Student ID,Name,Department,Semester,Phone,Address,Bus Number,Pickup Point,Seat Number,Email\n";
    std::vector<Student> list = getAllStudents();
    for (const auto& s : list) {
        csv << "\"" << s.getId() << "\","
            << "\"" << s.getName() << "\","
            << "\"" << s.getDepartment() << "\","
            << s.getSemester() << ","
            << "\"" << s.getPhone() << "\","
            << "\"" << s.getAddress() << "\","
            << "\"" << s.getBusNumber() << "\","
            << "\"" << s.getPickupPoint() << "\","
            << s.getSeatNumber() << ","
            << "\"" << s.getEmail() << "\"\n";
    }
    csv.close();
    return true;
}

bool FileManager::exportBusesToCSV(const std::string& destinationFile) {
    std::ofstream csv(destinationFile, std::ios::out | std::ios::trunc);
    if (!csv.is_open()) return false;

    csv << "Bus ID,Registration Number,Capacity,Occupied Seats,Available Seats,Assigned Driver,Route ID,Status,Shift Type,Current Location\n";
    std::vector<Bus> list = getAllBuses();
    for (const auto& b : list) {
        csv << "\"" << b.getBusId() << "\","
            << "\"" << b.getRegNumber() << "\","
            << b.getCapacity() << ","
            << b.getOccupiedSeats() << ","
            << b.getAvailableSeats() << ","
            << "\"" << b.getAssignedDriver() << "\","
            << "\"" << b.getRouteId() << "\","
            << "\"" << b.getStatus() << "\","
            << "\"" << b.getShiftType() << "\","
            << "\"" << b.getCurrentLocation() << "\"\n";
    }
    csv.close();
    return true;
}

bool FileManager::exportAttendanceToCSV(const std::string& destinationFile) {
    std::ofstream csv(destinationFile, std::ios::out | std::ios::trunc);
    if (!csv.is_open()) return false;

    csv << "Record ID,Date,Bus ID,Student ID,Student Name,Status,Remarks\n";
    std::vector<Attendance> list = getAllAttendance();
    for (const auto& a : list) {
        csv << "\"" << a.getRecordId() << "\","
            << "\"" << a.getDate() << "\","
            << "\"" << a.getBusId() << "\","
            << "\"" << a.getStudentId() << "\","
            << "\"" << a.getStudentName() << "\","
            << "\"" << a.getStatus() << "\","
            << "\"" << a.getRemarks() << "\"\n";
    }
    csv.close();
    return true;
}

bool FileManager::exportFeesToCSV(const std::string& destinationFile) {
    std::ofstream csv(destinationFile, std::ios::out | std::ios::trunc);
    if (!csv.is_open()) return false;

    csv << "Fee ID,Student ID,Student Name,Total Fee,Paid Amount,Pending Amount,Status,Last Payment Date,Receipt No\n";
    std::vector<FeeRecord> list = getAllFees();
    for (const auto& f : list) {
        csv << "\"" << f.getFeeId() << "\","
            << "\"" << f.getStudentId() << "\","
            << "\"" << f.getStudentName() << "\","
            << std::fixed << std::setprecision(2) << f.getTotalAmount() << ","
            << f.getPaidAmount() << ","
            << f.getPendingAmount() << ","
            << "\"" << f.getStatus() << "\","
            << "\"" << f.getPaymentDate() << "\","
            << "\"" << f.getReceiptNumber() << "\"\n";
    }
    csv.close();
    return true;
}

bool FileManager::exportMaintenanceToCSV(const std::string& destinationFile) {
    std::ofstream csv(destinationFile, std::ios::out | std::ios::trunc);
    if (!csv.is_open()) return false;

    csv << "Record ID,Bus ID,Service Type,Cost,Service Date,Workshop,Remarks\n";
    std::vector<MaintenanceRecord> list = getAllMaintenance();
    for (const auto& m : list) {
        csv << "\"" << m.getRecordId() << "\","
            << "\"" << m.getBusId() << "\","
            << "\"" << m.getServiceType() << "\","
            << std::fixed << std::setprecision(2) << m.getCost() << ","
            << "\"" << m.getServiceDate() << "\","
            << "\"" << m.getWorkshop() << "\","
            << "\"" << m.getRemarks() << "\"\n";
    }
    csv.close();
    return true;
}

bool FileManager::exportRouteWiseStudentsToCSV(const std::string& routeId, const std::string& destinationFile) {
    std::ofstream csv(destinationFile, std::ios::out | std::ios::trunc);
    if (!csv.is_open()) return false;

    Route r = getRouteById(routeId);
    csv << "Route ID," << r.getRouteId() << ",Route Name," << r.getRouteName() << ",Assigned Bus," << r.getAssignedBus() << "\n";
    csv << "Student ID,Name,Department,Semester,Phone,Pickup Point,Seat Number\n";

    std::vector<Student> all = getAllStudents();
    for (const auto& s : all) {
        if (s.getBusNumber() == r.getAssignedBus()) {
            csv << "\"" << s.getId() << "\","
                << "\"" << s.getName() << "\","
                << "\"" << s.getDepartment() << "\","
                << s.getSemester() << ","
                << "\"" << s.getPhone() << "\","
                << "\"" << s.getPickupPoint() << "\","
                << s.getSeatNumber() << "\n";
        }
    }
    csv.close();
    return true;
}
