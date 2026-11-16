#include "FeeRecord.h"
#include <iomanip>

FeeRecord::FeeRecord()
    : feeId(""), studentId(""), studentName(""), totalAmount(0.0),
      paidAmount(0.0), paymentDate(""), paymentMethod("UPI"), receiptNumber("") {}

FeeRecord::FeeRecord(const std::string& fId, const std::string& sId,
                     const std::string& sName, double total, double paid,
                     const std::string& pDate, const std::string& pMethod,
                     const std::string& receipt)
    : feeId(fId), studentId(sId), studentName(sName),
      totalAmount(total), paidAmount(paid),
      paymentDate(pDate), paymentMethod(pMethod),
      receiptNumber(receipt) {}

std::string FeeRecord::getStatus() const {
    if (paidAmount >= totalAmount && totalAmount > 0) return "Paid";
    if (paidAmount > 0) return "Partial";
    return "Pending";
}

void FeeRecord::addPayment(double amount, const std::string& date, const std::string& method) {
    paidAmount += amount;
    if (paidAmount > totalAmount) paidAmount = totalAmount;
    paymentDate = date;
    paymentMethod = method;
}

std::string FeeRecord::serialize() const {
    // Pipe-delimited: feeId|studentId|studentName|totalAmount|paidAmount|paymentDate|paymentMethod|receiptNumber
    std::ostringstream oss;
    oss << feeId << "|" << studentId << "|" << studentName << "|"
        << std::fixed << std::setprecision(2) << totalAmount << "|"
        << paidAmount << "|" << paymentDate << "|"
        << paymentMethod << "|" << receiptNumber;
    return oss.str();
}

FeeRecord FeeRecord::deserialize(const std::string& line) {
    std::vector<std::string> tokens;
    std::stringstream ss(line);
    std::string token;
    while (std::getline(ss, token, '|')) {
        tokens.push_back(token);
    }

    if (tokens.size() < 5) {
        return FeeRecord();
    }

    double total = 0.0;
    double paid = 0.0;
    try { total = std::stod(tokens[3]); } catch (...) { total = 0.0; }
    try { paid = std::stod(tokens[4]); } catch (...) { paid = 0.0; }

    std::string date = (tokens.size() >= 6) ? tokens[5] : "";
    std::string method = (tokens.size() >= 7) ? tokens[6] : "UPI";
    std::string receipt = (tokens.size() >= 8) ? tokens[7] : "";

    return FeeRecord(tokens[0], tokens[1], tokens[2], total, paid, date, method, receipt);
}
