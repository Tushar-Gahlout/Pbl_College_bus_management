#ifndef MAINTENANCERECORD_H
#define MAINTENANCERECORD_H

#include <string>
#include <sstream>
#include <vector>

/**
 * @brief MaintenanceRecord model representing bus servicing and maintenance expenses.
 */
class MaintenanceRecord {
private:
    std::string recordId;
    std::string busId;
    std::string serviceType; // "Oil Change", "Brake Overhaul", "Tire Replacement", "General Servicing", "Engine Repair"
    double cost;
    std::string serviceDate;
    std::string workshop;
    std::string remarks;

public:
    MaintenanceRecord();
    MaintenanceRecord(const std::string& rId, const std::string& bId,
                      const std::string& sType, double c,
                      const std::string& sDate, const std::string& wShop = "College Workshop",
                      const std::string& rem = "Regular maintenance");

    // Getters
    std::string getRecordId() const { return recordId; }
    std::string getBusId() const { return busId; }
    std::string getServiceType() const { return serviceType; }
    double getCost() const { return cost; }
    std::string getServiceDate() const { return serviceDate; }
    std::string getWorkshop() const { return workshop; }
    std::string getRemarks() const { return remarks; }

    // Setters
    void setRecordId(const std::string& id) { recordId = id; }
    void setBusId(const std::string& id) { busId = id; }
    void setServiceType(const std::string& type) { serviceType = type; }
    void setCost(double c) { cost = c; }
    void setServiceDate(const std::string& date) { serviceDate = date; }
    void setWorkshop(const std::string& w) { workshop = w; }
    void setRemarks(const std::string& r) { remarks = r; }

    // Serialization
    std::string serialize() const;
    static MaintenanceRecord deserialize(const std::string& line);
};

#endif // MAINTENANCERECORD_H
