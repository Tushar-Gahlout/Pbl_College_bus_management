#include "FileManager.h"
#include <iostream>
#include <filesystem>
#include <algorithm>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

namespace fs = std::filesystem;

// ============================================================================
// Internal Helpers for Clean, Deduplicated File Handling
// ============================================================================
namespace {

template <typename T>
std::vector<T> loadRecordsFromFile(const std::string& path) {
    std::vector<T> list;
    std::ifstream in(path);
    if (!in.is_open()) return list;

    std::string line;
    while (std::getline(in, line)) {
        if (!line.empty()) {
            T item = T::deserialize(line);
            list.push_back(item);
        }
    }
    return list;
}

template <typename T, typename Predicate>
T findRecordInFile(const std::string& path, Predicate predicate) {
    std::ifstream in(path);
    if (!in.is_open()) return T();

    std::string line;
    while (std::getline(in, line)) {
        if (!line.empty()) {
            T item = T::deserialize(line);
            if (predicate(item)) return item;
        }
    }
    return T();
}

template <typename T, typename Matcher>
bool updateRecordInFile(const std::string& origPath, const std::string& tempPath,
                        const T& updatedItem, Matcher matches,
                        const std::string& notFoundId, const std::string& entityName,
                        const std::function<bool(const std::string&, const std::string&)>& replaceFunc) {
    std::ifstream in(origPath);
    std::ofstream out(tempPath, std::ios::out | std::ios::trunc);

    if (!in.is_open() || !out.is_open()) {
        throw FileIOException(origPath, "update");
    }

    bool found = false;
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        T item = T::deserialize(line);
        if (matches(item)) {
            out << updatedItem.serialize() << "\n";
            found = true;
        } else {
            out << line << "\n";
        }
    }
    in.close();
    out.close();

    if (!found) {
        fs::remove(tempPath);
        throw NotFoundException(notFoundId, entityName);
    }

    return replaceFunc(origPath, tempPath);
}

template <typename T, typename Matcher>
bool deleteRecordFromFile(const std::string& origPath, const std::string& tempPath,
                          Matcher matches,
                          const std::string& notFoundId, const std::string& entityName,
                          const std::function<bool(const std::string&, const std::string&)>& replaceFunc) {
    std::ifstream in(origPath);
    std::ofstream out(tempPath, std::ios::out | std::ios::trunc);

    if (!in.is_open() || !out.is_open()) {
        throw FileIOException(origPath, "delete");
    }

    bool found = false;
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        T item = T::deserialize(line);
        if (matches(item)) {
            found = true; // skip writing to delete
        } else {
            out << line << "\n";
        }
    }
    in.close();
    out.close();

    if (!found) {
        fs::remove(tempPath);
        throw NotFoundException(notFoundId, entityName);
    }

    return replaceFunc(origPath, tempPath);
}

bool containsIgnoreCase(const std::string& haystack, const std::string& needleLower) {
    if (needleLower.empty()) return true;
    auto it = std::search(
        haystack.begin(), haystack.end(),
        needleLower.begin(), needleLower.end(),
        [](char h, char n) {
            return std::tolower(static_cast<unsigned char>(h)) == static_cast<unsigned char>(n);
        }
    );
    return it != haystack.end();
}

void writeSeedIfMissing(const std::string& filePath, const std::string& seedContent) {
    if (!fs::exists(filePath) || fs::file_size(filePath) == 0) {
        std::ofstream out(filePath, std::ios::out | std::ios::trunc);
        if (out.is_open()) {
            out << seedContent;
        }
    }
}

} // namespace

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
    } catch (const std::exception&) {
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
    try {
        if (!fs::exists(dataDirectory)) {
            fs::create_directories(dataDirectory);
        }
    } catch (...) {}

    // 1. users.txt
    writeSeedIfMissing(getUsersFilePath(),
        "admin|admin123|Admin||Administrator\n"
        "student1|pass123|Student|STU101|Rahul Sharma\n"
        "student2|pass123|Student|STU102|Priya Patel\n"
        "student3|pass123|Student|STU103|Amit Verma\n"
        "student4|pass123|Student|STU104|Sneha Gupta\n"
        "student5|pass123|Student|STU105|Rohan Mehra\n"
        "student6|pass123|Student|STU106|Ananya Roy\n"
        "driver1|pass123|Driver|DRV01|Rajesh Kumar\n"
        "driver2|pass123|Driver|DRV02|Vikram Singh\n"
        "driver3|pass123|Driver|DRV03|Mahesh Verma\n"
        "driver4|pass123|Driver|DRV04|Suresh Nair\n"
        "driver5|pass123|Driver|DRV05|Kuldeep Yadav\n"
    );

    // 2. buses.txt
    writeSeedIfMissing(getBusesFilePath(),
        "BUS01|DL-01-AB-1001|40|5|Rajesh Kumar|R01|Active|Full Day|College\n"
        "BUS02|DL-01-CD-2002|35|3|Vikram Singh|R02|Active|Half Day (Morning)|South Extension\n"
        "BUS03|DL-01-EF-3003|45|2|Suresh Nair|R03|Active|Full Day|Civil Lines\n"
        "BUS04|DL-01-GH-4004|30|0|None|None|Maintenance|Half Day (Evening)|Central Workshop\n"
        "BUS05|DL-01-JK-5005|40|3|Mahesh Verma|R04|Active|Full Day|West Terminal\n"
        "BUS06|DL-01-LM-6006|35|2|Kuldeep Yadav|R05|Active|Half Day (Morning)|North Campus\n"
    );

    // 3. drivers.txt
    writeSeedIfMissing(getDriversFilePath(),
        "DRV01|Rajesh Kumar|9876543210|DL-2015-998877|BUS01|8|rajesh@college.edu\n"
        "DRV02|Vikram Singh|9811223344|DL-2018-445566|BUS02|5|vikram@college.edu\n"
        "DRV03|Mahesh Verma|9822334455|DL-2012-112233|BUS05|12|mahesh@college.edu\n"
        "DRV04|Suresh Nair|9833445566|DL-2016-778899|BUS03|6|suresh@college.edu\n"
        "DRV05|Kuldeep Yadav|9844556677|DL-2019-332211|BUS06|4|kuldeep@college.edu\n"
    );

    // 4. routes.txt
    writeSeedIfMissing(getRoutesFilePath(),
        "R01|North City Express|City Centre|College Campus|City Centre;Metro Gate 3;Sector 14;Tech Hub;College|07:30 AM|05:00 PM|BUS01\n"
        "R02|South Suburban Line|South Extension|College Campus|South Extension;Green Park;Ring Road;University Junction;College|07:15 AM|05:15 PM|BUS02\n"
        "R03|East River Route|East Crossing|College Campus|East Crossing;Civil Lines;River Bridge;College|07:45 AM|04:45 PM|BUS03\n"
        "R04|West Highway Corridor|West Terminal|College Campus|West Terminal;Highway Flyover;Industrial Area;College|07:20 AM|05:10 PM|BUS05\n"
        "R05|North Metro Shuttle|North Campus|College Campus|North Campus;Model Town;Vishwa Vidyalaya;Kashmere Gate;College|07:35 AM|05:00 PM|BUS06\n"
    );

    // 5. students.txt
    writeSeedIfMissing(getStudentsFilePath(),
        "STU101|Rahul Sharma|Computer Science|5|9876500001|12/B Baker Street|BUS01|Sector 14|1|rahul@college.edu\n"
        "STU102|Priya Patel|Electronics|3|9876500002|45 Lake View Residency|BUS01|Metro Gate 3|2|priya@college.edu\n"
        "STU103|Amit Verma|Mechanical|7|9876500003|88 Model Town|BUS02|Green Park|1|amit@college.edu\n"
        "STU104|Sneha Gupta|Information Tech|4|9876500004|202 Sunrise Apartments|BUS02|Ring Road|2|sneha@college.edu\n"
        "STU105|Rohan Mehra|Civil|6|9876500005|14 Palm Avenue|BUS03|Civil Lines|1|rohan@college.edu\n"
        "STU106|Ananya Roy|Management|2|9876500006|73 Royal Gardens|BUS01|City Centre|3|ananya@college.edu\n"
        "STU107|Kavita Joshi|Biotechnology|4|9876500007|56 Green Valley|BUS02|University Junction|3|kavita@college.edu\n"
        "STU108|Aditya Kapoor|Computer Science|6|9876500008|19 Cyber Enclave|BUS01|Tech Hub|4|aditya@college.edu\n"
        "STU109|Pooja Hegde|Electrical|5|9876500009|104 Blossom Heights|BUS03|River Bridge|2|pooja@college.edu\n"
        "STU110|Manish Tiwari|Mechanical|3|9876500010|89 Industrial Way|BUS05|Industrial Area|1|manish@college.edu\n"
        "STU111|Simran Kaur|Civil|7|9876500011|22 Airport Road|BUS05|Highway Flyover|2|simran@college.edu\n"
        "STU112|Harsh Vardhan|Information Tech|1|9876500012|404 Lotus Tower|BUS06|Model Town|1|harsh@college.edu\n"
        "STU113|Divya Chawla|Computer Science|3|9876500013|17 Heritage Villa|BUS06|Kashmere Gate|2|divya@college.edu\n"
        "STU114|Aryan Malhotra|Management|4|9876500014|92 Sector 21|BUS05|West Terminal|3|aryan@college.edu\n"
        "STU115|Neha Kulkarni|Electronics|5|9876500015|31 Orchid Gardens|None|None|0|neha@college.edu\n"
    );

    // 6. seats.txt
    writeSeedIfMissing(getSeatsFilePath(),
        "BUS01|1|STU101|Rahul Sharma|2026-11-01\n"
        "BUS01|2|STU102|Priya Patel|2026-11-02\n"
        "BUS01|3|STU106|Ananya Roy|2026-11-05\n"
        "BUS01|4|STU108|Aditya Kapoor|2026-11-06\n"
        "BUS01|5|Traveler (Day Pass)|Tushar Gahlot|2026-11-18\n"
        "BUS02|1|STU103|Amit Verma|2026-11-03\n"
        "BUS02|2|STU104|Sneha Gupta|2026-11-04\n"
        "BUS02|3|STU107|Kavita Joshi|2026-11-07\n"
        "BUS03|1|STU105|Rohan Mehra|2026-11-05\n"
        "BUS03|2|STU109|Pooja Hegde|2026-11-08\n"
        "BUS05|1|STU110|Manish Tiwari|2026-11-09\n"
        "BUS05|2|STU111|Simran Kaur|2026-11-10\n"
        "BUS05|3|STU114|Aryan Malhotra|2026-11-11\n"
        "BUS06|1|STU112|Harsh Vardhan|2026-11-12\n"
        "BUS06|2|STU113|Divya Chawla|2026-11-13\n"
    );

    // 7. attendance.txt
    writeSeedIfMissing(getAttendanceFilePath(),
        "ATT001|2026-11-14|BUS01|STU101|Rahul Sharma|Present|Boarded at Sector 14\n"
        "ATT002|2026-11-14|BUS01|STU102|Priya Patel|Present|Boarded at Metro Gate 3\n"
        "ATT003|2026-11-15|BUS01|STU101|Rahul Sharma|Present|Boarded on time\n"
        "ATT004|2026-11-15|BUS01|STU102|Priya Patel|Absent|Notified in advance\n"
        "ATT005|2026-11-15|BUS02|STU103|Amit Verma|Present|Boarded at Green Park\n"
        "ATT006|2026-11-15|BUS02|STU104|Sneha Gupta|Present|Boarded at Ring Road\n"
        "ATT007|2026-11-17|BUS01|STU101|Rahul Sharma|Present|Boarded on time\n"
        "ATT008|2026-11-17|BUS01|STU102|Priya Patel|Present|Boarded on time\n"
        "ATT009|2026-11-17|BUS01|STU106|Ananya Roy|Present|Boarded at City Centre\n"
        "ATT010|2026-11-17|BUS02|STU103|Amit Verma|Present|Boarded on time\n"
        "ATT011|2026-11-17|BUS02|STU107|Kavita Joshi|Present|Boarded at University Junction\n"
        "ATT012|2026-11-17|BUS03|STU105|Rohan Mehra|Present|Boarded at Civil Lines\n"
        "ATT013|2026-11-17|BUS03|STU109|Pooja Hegde|Absent|Medical leave\n"
        "ATT014|2026-11-17|BUS05|STU110|Manish Tiwari|Present|Boarded at Industrial Area\n"
        "ATT015|2026-11-17|BUS05|STU111|Simran Kaur|Present|Boarded at Highway Flyover\n"
        "ATT016|2026-11-17|BUS06|STU112|Harsh Vardhan|Present|Boarded at Model Town\n"
        "ATT017|2026-11-17|BUS06|STU113|Divya Chawla|Present|Boarded at Kashmere Gate\n"
    );

    // 8. fees.txt
    writeSeedIfMissing(getFeesFilePath(),
        "FEE001|STU101|Rahul Sharma|15000.00|15000.00|2026-11-01|UPI|REC-2026-101\n"
        "FEE002|STU102|Priya Patel|15000.00|10000.00|2026-11-15|UPI|REC-2026-102\n"
        "FEE003|STU103|Amit Verma|15000.00|15000.00|2026-11-03|Cash|REC-2026-103\n"
        "FEE004|STU104|Sneha Gupta|15000.00|10000.00|2026-11-04|UPI|REC-2026-104\n"
        "FEE005|STU105|Rohan Mehra|15000.00|15000.00|2026-11-05|NetBanking|REC-2026-105\n"
        "FEE006|STU106|Ananya Roy|15000.00|15000.00|2026-11-06|UPI|REC-2026-106\n"
        "FEE007|STU107|Kavita Joshi|15000.00|7500.00|2026-11-07|Card|REC-2026-107\n"
        "FEE008|STU108|Aditya Kapoor|15000.00|15000.00|2026-11-08|UPI|REC-2026-108\n"
        "FEE009|STU109|Pooja Hegde|15000.00|15000.00|2026-11-09|NetBanking|REC-2026-109\n"
        "FEE010|STU110|Manish Tiwari|15000.00|15000.00|2026-11-10|UPI|REC-2026-110\n"
        "FEE011|STU111|Simran Kaur|15000.00|10000.00|2026-11-11|UPI|REC-2026-111\n"
        "FEE012|STU112|Harsh Vardhan|15000.00|15000.00|2026-11-12|NetBanking|REC-2026-112\n"
        "FEE013|STU113|Divya Chawla|15000.00|15000.00|2026-11-13|UPI|REC-2026-113\n"
        "FEE014|STU114|Aryan Malhotra|15000.00|5000.00|2026-11-14|Cash|REC-2026-114\n"
        "FEE015|STU115|Neha Kulkarni|15000.00|0.00||Pending|\n"
    );

    // 9. maintenance.txt
    writeSeedIfMissing(getMaintenanceFilePath(),
        "MNT001|BUS01|Oil Change & Filter|4500.00|2026-10-15|Authorized Service Center|Routine 10,000 km periodic servicing\n"
        "MNT002|BUS02|Brake Pad Replacement|8200.00|2026-10-20|City Auto Garage|Replaced front and rear brake pads\n"
        "MNT003|BUS04|Engine Diagnostics & Repair|18500.00|2026-11-05|College Central Workshop|Major transmission and engine checkup, under repair\n"
        "MNT004|BUS03|Tire Alignment & Balancing|3200.00|2026-11-10|Express Radial Tyre Center|Front wheel balance and rotation\n"
        "MNT005|BUS05|AC Compressor Overhaul|11500.00|2026-11-08|CoolAir Systems Workshop|Serviced cabin dual-zone air conditioning\n"
        "MNT006|BUS06|Battery & Alternator Check|4200.00|2026-11-12|National Exide Battery Care|Replaced auxiliary starting battery\n"
    );
}

// ==========================================
// STUDENT OPERATIONS
// ==========================================
std::vector<Student> FileManager::getAllStudents() {
    return loadRecordsFromFile<Student>(getStudentsFilePath());
}

bool FileManager::addStudent(const Student& student) {
    if (student.getId().empty()) {
        throw ValidationException("Student ID", "ID cannot be empty");
    }

    // Check duplicate ID
    for (const auto& existing : getAllStudents()) {
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

    Student oldStudent = getStudentById(student.getId());
    std::string tempPath = dataDirectory + "/temp_students.txt";

    updateRecordInFile<Student>(getStudentsFilePath(), tempPath,
        student, [&](const Student& s) { return s.getId() == student.getId(); },
        student.getId(), "Student",
        [this](const std::string& o, const std::string& t) { return replaceFileWithTemp(o, t); });

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
    Student toDelete = getStudentById(studentId);
    std::string tempPath = dataDirectory + "/temp_students.txt";

    deleteRecordFromFile<Student>(getStudentsFilePath(), tempPath,
        [&](const Student& s) { return s.getId() == studentId; },
        studentId, "Student",
        [this](const std::string& o, const std::string& t) { return replaceFileWithTemp(o, t); });

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
    return findRecordInFile<Student>(getStudentsFilePath(), [&](const Student& s) { return s.getId() == studentId; });
}

std::vector<Student> FileManager::searchStudents(const std::string& query) {
    if (query.empty()) return getAllStudents();
    std::string qLower = query;
    std::transform(qLower.begin(), qLower.end(), qLower.begin(), ::tolower);

    std::vector<Student> results;
    for (const auto& s : getAllStudents()) {
        if (containsIgnoreCase(s.getId(), qLower) ||
            containsIgnoreCase(s.getName(), qLower) ||
            containsIgnoreCase(s.getDepartment(), qLower) ||
            containsIgnoreCase(s.getBusNumber(), qLower) ||
            containsIgnoreCase(s.getPickupPoint(), qLower)) {
            results.push_back(s);
        }
    }
    return results;
}

std::vector<Student> FileManager::searchStudents(const std::string& department, int semester) {
    std::vector<Student> results;
    for (const auto& s : getAllStudents()) {
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
    return loadRecordsFromFile<Bus>(getBusesFilePath());
}

bool FileManager::addBus(const Bus& bus) {
    if (bus.getBusId().empty()) {
        throw ValidationException("Bus ID", "ID cannot be empty");
    }

    for (const auto& b : getAllBuses()) {
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
    std::string tempPath = dataDirectory + "/temp_buses.txt";
    return updateRecordInFile<Bus>(getBusesFilePath(), tempPath,
        bus, [&](const Bus& b) { return b.getBusId() == bus.getBusId(); },
        bus.getBusId(), "Bus",
        [this](const std::string& o, const std::string& t) { return replaceFileWithTemp(o, t); });
}

bool FileManager::deleteBus(const std::string& busId) {
    std::string tempPath = dataDirectory + "/temp_buses.txt";
    return deleteRecordFromFile<Bus>(getBusesFilePath(), tempPath,
        [&](const Bus& b) { return b.getBusId() == busId; },
        busId, "Bus",
        [this](const std::string& o, const std::string& t) { return replaceFileWithTemp(o, t); });
}

Bus FileManager::getBusById(const std::string& busId) {
    return findRecordInFile<Bus>(getBusesFilePath(), [&](const Bus& b) { return b.getBusId() == busId; });
}

std::vector<Bus> FileManager::searchBuses(const std::string& query) {
    if (query.empty()) return getAllBuses();
    std::string qLower = query;
    std::transform(qLower.begin(), qLower.end(), qLower.begin(), ::tolower);

    std::vector<Bus> results;
    for (const auto& b : getAllBuses()) {
        if (containsIgnoreCase(b.getBusId(), qLower) ||
            containsIgnoreCase(b.getRegNumber(), qLower) ||
            containsIgnoreCase(b.getAssignedDriver(), qLower) ||
            containsIgnoreCase(b.getStatus(), qLower) ||
            containsIgnoreCase(b.getRouteId(), qLower) ||
            containsIgnoreCase(b.getShiftType(), qLower) ||
            containsIgnoreCase(b.getCurrentLocation(), qLower)) {
            results.push_back(b);
        }
    }
    return results;
}

// ==========================================
// DRIVER OPERATIONS
// ==========================================
std::vector<Driver> FileManager::getAllDrivers() {
    return loadRecordsFromFile<Driver>(getDriversFilePath());
}

bool FileManager::addDriver(const Driver& driver) {
    if (driver.getId().empty()) {
        throw ValidationException("Driver ID", "ID cannot be empty");
    }

    for (const auto& d : getAllDrivers()) {
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

    // If driver assigned to a bus, update the bus record
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
    std::string tempPath = dataDirectory + "/temp_drivers.txt";
    bool ok = updateRecordInFile<Driver>(getDriversFilePath(), tempPath,
        driver, [&](const Driver& d) { return d.getId() == driver.getId(); },
        driver.getId(), "Driver",
        [this](const std::string& o, const std::string& t) { return replaceFileWithTemp(o, t); });

    // Update bus assigned driver if applicable
    if (driver.getAssignedBus() != "None" && !driver.getAssignedBus().empty()) {
        Bus b = getBusById(driver.getAssignedBus());
        if (!b.getBusId().empty()) {
            b.setAssignedDriver(driver.getName());
            updateBus(b);
        }
    }
    return ok;
}

bool FileManager::deleteDriver(const std::string& driverId) {
    Driver toDelete = getDriverById(driverId);
    std::string tempPath = dataDirectory + "/temp_drivers.txt";

    bool ok = deleteRecordFromFile<Driver>(getDriversFilePath(), tempPath,
        [&](const Driver& d) { return d.getId() == driverId; },
        driverId, "Driver",
        [this](const std::string& o, const std::string& t) { return replaceFileWithTemp(o, t); });

    // Unassign from bus if was assigned
    if (toDelete.getAssignedBus() != "None" && !toDelete.getAssignedBus().empty()) {
        Bus b = getBusById(toDelete.getAssignedBus());
        if (!b.getBusId().empty() && b.getAssignedDriver() == toDelete.getName()) {
            b.setAssignedDriver("None");
            updateBus(b);
        }
    }
    return ok;
}

Driver FileManager::getDriverById(const std::string& driverId) {
    return findRecordInFile<Driver>(getDriversFilePath(), [&](const Driver& d) { return d.getId() == driverId; });
}

std::vector<Driver> FileManager::searchDrivers(const std::string& query) {
    if (query.empty()) return getAllDrivers();
    std::string qLower = query;
    std::transform(qLower.begin(), qLower.end(), qLower.begin(), ::tolower);

    std::vector<Driver> results;
    for (const auto& d : getAllDrivers()) {
        if (containsIgnoreCase(d.getId(), qLower) ||
            containsIgnoreCase(d.getName(), qLower) ||
            containsIgnoreCase(d.getPhone(), qLower) ||
            containsIgnoreCase(d.getLicenseNumber(), qLower) ||
            containsIgnoreCase(d.getAssignedBus(), qLower)) {
            results.push_back(d);
        }
    }
    return results;
}

// ==========================================
// ROUTE OPERATIONS
// ==========================================
std::vector<Route> FileManager::getAllRoutes() {
    return loadRecordsFromFile<Route>(getRoutesFilePath());
}

bool FileManager::addRoute(const Route& route) {
    if (route.getRouteId().empty()) {
        throw ValidationException("Route ID", "ID cannot be empty");
    }

    for (const auto& r : getAllRoutes()) {
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
    std::string tempPath = dataDirectory + "/temp_routes.txt";
    bool ok = updateRecordInFile<Route>(getRoutesFilePath(), tempPath,
        route, [&](const Route& r) { return r.getRouteId() == route.getRouteId(); },
        route.getRouteId(), "Route",
        [this](const std::string& o, const std::string& t) { return replaceFileWithTemp(o, t); });

    if (route.getAssignedBus() != "None" && !route.getAssignedBus().empty()) {
        Bus b = getBusById(route.getAssignedBus());
        if (!b.getBusId().empty()) {
            b.setRouteId(route.getRouteId());
            updateBus(b);
        }
    }
    return ok;
}

bool FileManager::deleteRoute(const std::string& routeId) {
    std::string tempPath = dataDirectory + "/temp_routes.txt";
    return deleteRecordFromFile<Route>(getRoutesFilePath(), tempPath,
        [&](const Route& r) { return r.getRouteId() == routeId; },
        routeId, "Route",
        [this](const std::string& o, const std::string& t) { return replaceFileWithTemp(o, t); });
}

Route FileManager::getRouteById(const std::string& routeId) {
    return findRecordInFile<Route>(getRoutesFilePath(), [&](const Route& r) { return r.getRouteId() == routeId; });
}

std::vector<Route> FileManager::searchRoutes(const std::string& query) {
    if (query.empty()) return getAllRoutes();
    std::string qLower = query;
    std::transform(qLower.begin(), qLower.end(), qLower.begin(), ::tolower);

    std::vector<Route> results;
    for (const auto& r : getAllRoutes()) {
        if (containsIgnoreCase(r.getRouteId(), qLower) ||
            containsIgnoreCase(r.getRouteName(), qLower) ||
            containsIgnoreCase(r.getStartPoint(), qLower) ||
            containsIgnoreCase(r.getEndPoint(), qLower) ||
            containsIgnoreCase(r.getStopsAsString(), qLower) ||
            containsIgnoreCase(r.getAssignedBus(), qLower)) {
            results.push_back(r);
        }
    }
    return results;
}

// ==========================================
// SEAT OPERATIONS
// ==========================================
std::vector<Seat> FileManager::getAllSeats() {
    return loadRecordsFromFile<Seat>(getSeatsFilePath());
}

std::vector<Seat> FileManager::getSeatsForBus(const std::string& busId) {
    std::vector<Seat> list;
    for (const auto& s : getAllSeats()) {
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
        if (!(s.getBusId() == busId && s.getSeatNumber() == seatNum)) {
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
        if (s.getStudentId() != studentId) {
            out << line << "\n";
        }
    }
    in.close();
    out.close();

    replaceFileWithTemp(origPath, tempPath);
    return true;
}

Seat FileManager::getSeatForStudent(const std::string& studentId) {
    for (const auto& s : getAllSeats()) {
        if (s.getStudentId() == studentId) return s;
    }
    return Seat();
}

// ==========================================
// ATTENDANCE OPERATIONS
// ==========================================
std::vector<Attendance> FileManager::getAllAttendance() {
    return loadRecordsFromFile<Attendance>(getAttendanceFilePath());
}

std::vector<Attendance> FileManager::getAttendanceByBusAndDate(const std::string& busId, const std::string& date) {
    std::vector<Attendance> list;
    for (const auto& a : getAllAttendance()) {
        if (a.getBusId() == busId && (date.empty() || a.getDate() == date)) {
            list.push_back(a);
        }
    }
    return list;
}

std::vector<Attendance> FileManager::getAttendanceByStudent(const std::string& studentId) {
    std::vector<Attendance> list;
    for (const auto& a : getAllAttendance()) {
        if (a.getStudentId() == studentId) {
            list.push_back(a);
        }
    }
    return list;
}

bool FileManager::markAttendance(const std::vector<Attendance>& records) {
    if (records.empty()) return true;

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
    return loadRecordsFromFile<FeeRecord>(getFeesFilePath());
}

FeeRecord FileManager::getFeeByStudentId(const std::string& studentId) {
    for (const auto& f : getAllFees()) {
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
        addFeeRecord(fee);
        fs::remove(tempPath);
        return true;
    }

    replaceFileWithTemp(origPath, tempPath);
    return true;
}

std::vector<FeeRecord> FileManager::searchFees(const std::string& query) {
    if (query.empty()) return getAllFees();
    std::string qLower = query;
    std::transform(qLower.begin(), qLower.end(), qLower.begin(), ::tolower);

    std::vector<FeeRecord> results;
    for (const auto& f : getAllFees()) {
        if (containsIgnoreCase(f.getStudentId(), qLower) ||
            containsIgnoreCase(f.getStudentName(), qLower) ||
            containsIgnoreCase(f.getStatus(), qLower) ||
            containsIgnoreCase(f.getReceiptNumber(), qLower)) {
            results.push_back(f);
        }
    }
    return results;
}

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
    return loadRecordsFromFile<MaintenanceRecord>(getMaintenanceFilePath());
}

std::vector<MaintenanceRecord> FileManager::getMaintenanceForBus(const std::string& busId) {
    std::vector<MaintenanceRecord> list;
    for (const auto& m : getAllMaintenance()) {
        if (m.getBusId() == busId) list.push_back(m);
    }
    return list;
}

bool FileManager::addMaintenanceRecord(const MaintenanceRecord& record) {
    if (record.getRecordId().empty()) {
        throw ValidationException("Record ID", "ID cannot be empty");
    }

    for (const auto& m : getAllMaintenance()) {
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
    std::string tempPath = dataDirectory + "/temp_maintenance.txt";
    return updateRecordInFile<MaintenanceRecord>(getMaintenanceFilePath(), tempPath,
        record, [&](const MaintenanceRecord& m) { return m.getRecordId() == record.getRecordId(); },
        record.getRecordId(), "Maintenance Record",
        [this](const std::string& o, const std::string& t) { return replaceFileWithTemp(o, t); });
}

bool FileManager::deleteMaintenanceRecord(const std::string& recordId) {
    std::string tempPath = dataDirectory + "/temp_maintenance.txt";
    return deleteRecordFromFile<MaintenanceRecord>(getMaintenanceFilePath(), tempPath,
        [&](const MaintenanceRecord& m) { return m.getRecordId() == recordId; },
        recordId, "Maintenance Record",
        [this](const std::string& o, const std::string& t) { return replaceFileWithTemp(o, t); });
}

// ==========================================
// USER AUTHENTICATION
// ==========================================
std::vector<User> FileManager::getAllUsers() {
    return loadRecordsFromFile<User>(getUsersFilePath());
}

bool FileManager::addUser(const User& user) {
    if (user.getUsername().empty() || user.getPassword().empty()) {
        throw ValidationException("Credentials", "Username and password cannot be empty");
    }

    for (const auto& u : getAllUsers()) {
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
    for (const auto& u : getAllUsers()) {
        if (u.getUsername() == username && u.validatePassword(password)) {
            return u;
        }
    }
    return User();
}

// ==========================================
// CSV EXPORTS (C++ FILE HANDLING)
// ==========================================
bool FileManager::exportStudentsToCSV(const std::string& destinationFile) {
    std::ofstream csv(destinationFile, std::ios::out | std::ios::trunc);
    if (!csv.is_open()) return false;

    csv << "Student ID,Name,Department,Semester,Phone,Address,Bus Number,Pickup Point,Seat Number,Email\n";
    for (const auto& s : getAllStudents()) {
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
    for (const auto& b : getAllBuses()) {
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
    for (const auto& a : getAllAttendance()) {
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
    for (const auto& f : getAllFees()) {
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
    for (const auto& m : getAllMaintenance()) {
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

    for (const auto& s : getAllStudents()) {
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
