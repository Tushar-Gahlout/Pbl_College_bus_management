#include "FeeManagementView.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QDialog>
#include <QFormLayout>
#include <QDoubleSpinBox>
#include <QDateEdit>
#include <QComboBox>
#include <QTextEdit>
#include <iomanip>
#include <sstream>

FeeManagementView::FeeManagementView(FileManager* fm, QWidget* parent)
    : QWidget(parent), fileManager(fm) {
    setupUI();
    refreshData();
}

void FeeManagementView::setupUI() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 24, 24, 24);
    mainLayout->setSpacing(16);

    // Top Header & Action Buttons
    auto* topLayout = new QHBoxLayout();
    auto* title = new QLabel("Fee Management & Receipts", this);
    title->setStyleSheet("font-size: 20px; font-weight: bold; color: #0f172a;");
    topLayout->addWidget(title);
    topLayout->addStretch();

    recordPaymentBtn = new QPushButton("💳 Record Payment", this);
    recordPaymentBtn->setProperty("class", "primaryButton");
    recordPaymentBtn->setCursor(Qt::PointingHandCursor);

    viewReceiptBtn = new QPushButton("🧾 Generate / View Receipt", this);
    viewReceiptBtn->setProperty("class", "secondaryButton");
    viewReceiptBtn->setCursor(Qt::PointingHandCursor);

    refreshBtn = new QPushButton("🔄 Refresh", this);
    refreshBtn->setProperty("class", "secondaryButton");
    refreshBtn->setCursor(Qt::PointingHandCursor);

    topLayout->addWidget(recordPaymentBtn);
    topLayout->addWidget(viewReceiptBtn);
    topLayout->addWidget(refreshBtn);
    mainLayout->addLayout(topLayout);

    // Financial KPI Summary Cards
    auto* kpiLayout = new QHBoxLayout();
    kpiLayout->setSpacing(16);

    auto makeKpi = [&](const QString& label, QLabel*& valLabel, const QString& color) {
        auto* card = new QFrame(this);
        card->setStyleSheet(QString(
            "background-color: #ffffff; border: 1px solid #e2e8f0; border-radius: 8px; padding: 12px; border-left: 5px solid %1;"
        ).arg(color));
        auto* lay = new QVBoxLayout(card);
        lay->setContentsMargins(8, 6, 8, 6);
        lay->setSpacing(4);

        auto* l = new QLabel(label, card);
        l->setStyleSheet("font-size: 11px; font-weight: bold; color: #64748b; text-transform: uppercase;");
        valLabel = new QLabel("₹0.00", card);
        valLabel->setStyleSheet(QString("font-size: 20px; font-weight: bold; color: %1;").arg(color));

        lay->addWidget(l);
        lay->addWidget(valLabel);
        return card;
    };

    kpiLayout->addWidget(makeKpi("Total Billed Transportation Fee", totalRevenueLbl, "#2563eb"));
    kpiLayout->addWidget(makeKpi("Total Fees Collected", totalCollectedLbl, "#16a34a"));
    kpiLayout->addWidget(makeKpi("Total Pending Balance", totalPendingLbl, "#dc2626"));
    mainLayout->addLayout(kpiLayout);

    // Search Bar
    searchEdit = new QLineEdit(this);
    searchEdit->setPlaceholderText("🔍 Search fee records by Student ID, Student Name, Status, or Receipt No...");
    searchEdit->setClearButtonEnabled(true);
    mainLayout->addWidget(searchEdit);

    // Fees Table
    feeTable = new QTableWidget(this);
    feeTable->setColumnCount(9);
    feeTable->setHorizontalHeaderLabels({
        "Fee ID", "Student ID", "Student Name", "Total Fee", "Paid Amount", "Pending Balance", "Status", "Payment Date", "Receipt No"
    });
    feeTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    feeTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    feeTable->setSelectionMode(QAbstractItemView::SingleSelection);
    feeTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    feeTable->verticalHeader()->setVisible(false);
    mainLayout->addWidget(feeTable);

    // Connect signals
    connect(searchEdit, &QLineEdit::textChanged, this, &FeeManagementView::handleSearch);
    connect(recordPaymentBtn, &QPushButton::clicked, this, &FeeManagementView::handleRecordPayment);
    connect(viewReceiptBtn, &QPushButton::clicked, this, &FeeManagementView::handleViewReceipt);
    connect(refreshBtn, &QPushButton::clicked, this, &FeeManagementView::refreshData);
    connect(feeTable, &QTableWidget::doubleClicked, this, &FeeManagementView::handleViewReceipt);
}

void FeeManagementView::refreshData() {
    handleSearch();
}

void FeeManagementView::handleSearch() {
    std::string query = searchEdit->text().trimmed().toStdString();
    std::vector<FeeRecord> list = fileManager->searchFees(query);

    double totalRev = 0.0;
    double totalPaid = 0.0;
    double totalPend = 0.0;

    auto allFees = fileManager->getAllFees();
    for (const auto& f : allFees) {
        totalRev += f.getTotalAmount();
        totalPaid += f.getPaidAmount();
        totalPend += f.getPendingAmount();
    }

    auto fmtMoney = [](double val) {
        std::ostringstream ss;
        ss << "₹" << std::fixed << std::setprecision(2) << val;
        return QString::fromStdString(ss.str());
    };

    totalRevenueLbl->setText(fmtMoney(totalRev));
    totalCollectedLbl->setText(fmtMoney(totalPaid));
    totalPendingLbl->setText(fmtMoney(totalPend));

    feeTable->setRowCount(0);
    for (size_t i = 0; i < list.size(); ++i) {
        const auto& f = list[i];
        feeTable->insertRow(i);

        feeTable->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(f.getFeeId())));
        feeTable->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(f.getStudentId())));
        feeTable->setItem(i, 2, new QTableWidgetItem(QString::fromStdString(f.getStudentName())));
        feeTable->setItem(i, 3, new QTableWidgetItem(fmtMoney(f.getTotalAmount())));
        feeTable->setItem(i, 4, new QTableWidgetItem(fmtMoney(f.getPaidAmount())));
        feeTable->setItem(i, 5, new QTableWidgetItem(fmtMoney(f.getPendingAmount())));

        auto* statusItem = new QTableWidgetItem(QString::fromStdString(f.getStatus()));
        if (f.getStatus() == "Paid") {
            statusItem->setForeground(QBrush(QColor("#16a34a")));
        } else if (f.getStatus() == "Partial") {
            statusItem->setForeground(QBrush(QColor("#d97706")));
        } else {
            statusItem->setForeground(QBrush(QColor("#dc2626")));
        }
        feeTable->setItem(i, 6, statusItem);

        feeTable->setItem(i, 7, new QTableWidgetItem(QString::fromStdString(f.getPaymentDate())));
        feeTable->setItem(i, 8, new QTableWidgetItem(QString::fromStdString(f.getReceiptNumber())));
    }
}

void FeeManagementView::handleRecordPayment() {
    showPaymentDialog();
}

void FeeManagementView::handleViewReceipt() {
    int row = feeTable->currentRow();
    if (row < 0) {
        QMessageBox::information(this, "Select Record", "Please select a fee record from the table to view the receipt.");
        return;
    }

    std::string sId = feeTable->item(row, 1)->text().toStdString();
    FeeRecord fee = fileManager->getFeeByStudentId(sId);
    if (!fee.getFeeId().empty()) {
        showReceiptDialog(fee);
    }
}

void FeeManagementView::showPaymentDialog() {
    QDialog dlg(this);
    dlg.setWindowTitle("Record Student Fee Payment");
    dlg.setFixedSize(460, 380);

    auto* layout = new QVBoxLayout(&dlg);
    layout->setSpacing(14);
    layout->setContentsMargins(20, 20, 20, 20);

    auto* formLayout = new QFormLayout();
    formLayout->setSpacing(10);

    auto* studentCombo = new QComboBox(&dlg);
    auto allStudents = fileManager->getAllStudents();
    for (const auto& s : allStudents) {
        FeeRecord f = fileManager->getFeeByStudentId(s.getId());
        double pending = f.getFeeId().empty() ? 15000.0 : f.getPendingAmount();
        studentCombo->addItem(
            QString("%1 - %2 (Pending: ₹%3)").arg(QString::fromStdString(s.getId()), QString::fromStdString(s.getName())).arg(pending),
            QString::fromStdString(s.getId())
        );
    }

    auto* amountSpin = new QDoubleSpinBox(&dlg);
    amountSpin->setRange(1.0, 100000.0);
    amountSpin->setValue(5000.0);
    amountSpin->setPrefix("₹ ");

    auto* dateEdit = new QDateEdit(QDate::currentDate(), &dlg);
    dateEdit->setCalendarPopup(true);
    dateEdit->setDisplayFormat("yyyy-MM-dd");

    auto* methodCombo = new QComboBox(&dlg);
    methodCombo->addItems({"UPI / QR", "Credit/Debit Card", "Net Banking", "Cash"});

    auto* receiptEdit = new QLineEdit(&dlg);
    receiptEdit->setText(QString("REC-%1-%2").arg(QDate::currentDate().toString("yyyyMMdd")).arg(rand() % 900 + 100));

    formLayout->addRow("Select Student *:", studentCombo);
    formLayout->addRow("Payment Amount *:", amountSpin);
    formLayout->addRow("Payment Date:", dateEdit);
    formLayout->addRow("Payment Method:", methodCombo);
    formLayout->addRow("Receipt Number:", receiptEdit);

    layout->addLayout(formLayout);

    auto* btnLayout = new QHBoxLayout();
    auto* saveBtn = new QPushButton("Process Payment", &dlg);
    saveBtn->setProperty("class", "primaryButton");
    auto* cancelBtn = new QPushButton("Cancel", &dlg);
    cancelBtn->setProperty("class", "secondaryButton");

    btnLayout->addStretch();
    btnLayout->addWidget(cancelBtn);
    btnLayout->addWidget(saveBtn);
    layout->addLayout(btnLayout);

    connect(cancelBtn, &QPushButton::clicked, &dlg, &QDialog::reject);
    connect(saveBtn, &QPushButton::clicked, [&]() {
        if (studentCombo->count() == 0) return;

        std::string sId = studentCombo->currentData().toString().toStdString();
        double amount = amountSpin->value();
        std::string date = dateEdit->date().toString("yyyy-MM-dd").toStdString();
        std::string method = methodCombo->currentText().toStdString();
        std::string receipt = receiptEdit->text().trimmed().toStdString();

        try {
            // Call overloaded recordPayment
            fileManager->recordPayment(sId, amount, date, method, receipt);
            QMessageBox::information(&dlg, "Payment Recorded",
                QString("Successfully recorded payment of ₹%1 for %2.\nReceipt No: %3")
                    .arg(amount).arg(QString::fromStdString(sId), QString::fromStdString(receipt)));
            dlg.accept();
        } catch (const std::exception& e) {
            QMessageBox::critical(&dlg, "Payment Failed", QString::fromStdString(e.what()));
        }
    });

    if (dlg.exec() == QDialog::Accepted) {
        refreshData();
    }
}

void FeeManagementView::showReceiptDialog(const FeeRecord& fee) {
    QDialog dlg(this);
    dlg.setWindowTitle(QString("Transportation Fee Receipt - %1").arg(QString::fromStdString(fee.getReceiptNumber())));
    dlg.setFixedSize(500, 520);

    auto* layout = new QVBoxLayout(&dlg);
    layout->setContentsMargins(20, 20, 20, 20);

    Student s = fileManager->getStudentById(fee.getStudentId());

    auto* receiptText = new QTextEdit(&dlg);
    receiptText->setReadOnly(true);
    receiptText->setStyleSheet("font-family: monospace; font-size: 13px; background-color: #f8fafc; border: 1px solid #cbd5e1; padding: 12px;");

    std::ostringstream ss;
    ss << "====================================================\n"
       << "         COLLEGE TRANSPORTATION DEPARTMENT         \n"
       << "              OFFICIAL PAYMENT RECEIPT              \n"
       << "====================================================\n\n"
       << " Receipt No   : " << (fee.getReceiptNumber().empty() ? "N/A" : fee.getReceiptNumber()) << "\n"
       << " Payment Date : " << (fee.getPaymentDate().empty() ? "N/A" : fee.getPaymentDate()) << "\n"
       << " Payment Mode : " << fee.getPaymentMethod() << "\n\n"
       << " ---------------- STUDENT INFORMATION ---------------\n"
       << " Student ID   : " << fee.getStudentId() << "\n"
       << " Student Name : " << fee.getStudentName() << "\n"
       << " Department   : " << s.getDepartment() << " (Sem " << s.getSemester() << ")\n"
       << " Assigned Bus : " << s.getBusNumber() << " (Seat: " << s.getSeatNumber() << ")\n"
       << " Pickup Point : " << s.getPickupPoint() << "\n\n"
       << " ---------------- FEE BREAKDOWN (INR) ---------------\n"
       << std::fixed << std::setprecision(2)
       << " Total Annual Bus Fee   : ₹" << fee.getTotalAmount() << "\n"
       << " Total Amount Paid      : ₹" << fee.getPaidAmount() << "\n"
       << " Balance Pending        : ₹" << fee.getPendingAmount() << "\n"
       << " Status                 : " << fee.getStatus() << "\n\n"
       << "====================================================\n"
       << " Authorized Signature: College Transport Office     \n"
       << " This is an automated computer-generated receipt.   \n"
       << "====================================================\n";

    receiptText->setPlainText(QString::fromStdString(ss.str()));
    layout->addWidget(receiptText);

    auto* btnLayout = new QHBoxLayout();
    auto* closeBtn = new QPushButton("Close", &dlg);
    closeBtn->setProperty("class", "primaryButton");
    connect(closeBtn, &QPushButton::clicked, &dlg, &QDialog::accept);
    btnLayout->addStretch();
    btnLayout->addWidget(closeBtn);
    layout->addLayout(btnLayout);

    dlg.exec();
}
