#ifndef ATTENDANCE_H
#define ATTENDANCE_H

#include <string>
#include <sstream>
#include <vector>

/**
 * @brief Attendance model to track daily student bus attendance.
 */
class Attendance {
private:
    std::string recordId;
    std::string date;
    std::string busId;
    std::string studentId;
    std::string studentName;
    std::string status; // "Present", "Absent"
    std::string remarks;

public:
    Attendance();
    Attendance(const std::string& recId, const std::string& attDate,
               const std::string& bId, const std::string& sId,
               const std::string& sName, const std::string& stat = "Present",
               const std::string& rem = "Regular");

    // Getters
    std::string getRecordId() const { return recordId; }
    std::string getDate() const { return date; }
    std::string getBusId() const { return busId; }
    std::string getStudentId() const { return studentId; }
    std::string getStudentName() const { return studentName; }
    std::string getStatus() const { return status; }
    std::string getRemarks() const { return remarks; }

    // Setters
    void setRecordId(const std::string& recId) { recordId = recId; }
    void setDate(const std::string& d) { date = d; }
    void setBusId(const std::string& bId) { busId = bId; }
    void setStudentId(const std::string& sId) { studentId = sId; }
    void setStudentName(const std::string& sName) { studentName = sName; }
    void setStatus(const std::string& stat) { status = stat; }
    void setRemarks(const std::string& rem) { remarks = rem; }

    // Serialization
    std::string serialize() const;
    static Attendance deserialize(const std::string& line);
};

#endif // ATTENDANCE_H
