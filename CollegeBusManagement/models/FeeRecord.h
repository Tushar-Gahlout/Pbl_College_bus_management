#ifndef FEERECORD_H
#define FEERECORD_H

#include <string>
#include <sstream>
#include <vector>

/**
 * @brief FeeRecord model representing student bus transportation fees and payments.
 */
class FeeRecord {
private:
    std::string feeId;
    std::string studentId;
    std::string studentName;
    double totalAmount;
    double paidAmount;
    std::string paymentDate;
    std::string paymentMethod; // "Cash", "Card", "UPI", "NetBanking"
    std::string receiptNumber;

public:
    FeeRecord();
    FeeRecord(const std::string& fId, const std::string& sId,
              const std::string& sName, double total, double paid = 0.0,
              const std::string& pDate = "",
              const std::string& pMethod = "UPI",
              const std::string& receipt = "");

    // Getters
    std::string getFeeId() const { return feeId; }
    std::string getStudentId() const { return studentId; }
    std::string getStudentName() const { return studentName; }
    double getTotalAmount() const { return totalAmount; }
    double getPaidAmount() const { return paidAmount; }
    double getPendingAmount() const { return totalAmount - paidAmount; }
    std::string getPaymentDate() const { return paymentDate; }
    std::string getPaymentMethod() const { return paymentMethod; }
    std::string getReceiptNumber() const { return receiptNumber; }
    std::string getStatus() const;

    // Setters
    void setFeeId(const std::string& id) { feeId = id; }
    void setStudentId(const std::string& id) { studentId = id; }
    void setStudentName(const std::string& name) { studentName = name; }
    void setTotalAmount(double total) { totalAmount = total; }
    void setPaidAmount(double paid) { paidAmount = paid; }
    void setPaymentDate(const std::string& date) { paymentDate = date; }
    void setPaymentMethod(const std::string& method) { paymentMethod = method; }
    void setReceiptNumber(const std::string& receipt) { receiptNumber = receipt; }

    // Business Logic
    void addPayment(double amount, const std::string& date, const std::string& method);

    // Serialization
    std::string serialize() const;
    static FeeRecord deserialize(const std::string& line);
};

#endif // FEERECORD_H
