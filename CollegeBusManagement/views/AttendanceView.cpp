#include "AttendanceView.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QDialog>
#include <QLineEdit>
#include <QDate>

AttendanceView::AttendanceView(FileManager* fm, QWidget* parent)
    : QWidget(parent), fileManager(fm) {
    setupUI();
    refreshData();
}

void AttendanceView::setupUI() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 24, 24, 24);
    mainLayout->setSpacing(16);

    // Header & Actions
    auto* topLayout = new QHBoxLayout();
    auto* title = new QLabel("Attendance Management", this);
    title->setStyleSheet("font-size: 20px; font-weight: bold; color: #0f172a;");
    topLayout->addWidget(title);
    topLayout->addStretch();

    saveAttendanceBtn = new QPushButton("💾 Save Attendance", this);
    saveAttendanceBtn->setProperty("class", "primaryButton");
    saveAttendanceBtn->setCursor(Qt::PointingHandCursor);

    markAllPresentBtn = new QPushButton("✅ Mark All Present", this);
    markAllPresentBtn->setProperty("class", "successButton");
    markAllPresentBtn->setCursor(Qt::PointingHandCursor);

    markAllAbsentBtn = new QPushButton("❌ Mark All Absent", this);
    markAllAbsentBtn->setProperty("class", "dangerButton");
    markAllAbsentBtn->setCursor(Qt::PointingHandCursor);

    viewHistoryBtn = new QPushButton("📜 View History", this);
    viewHistoryBtn->setProperty("class", "secondaryButton");
    viewHistoryBtn->setCursor(Qt::PointingHandCursor);

    refreshBtn = new QPushButton("🔄 Refresh", this);
    refreshBtn->setProperty("class", "secondaryButton");
    refreshBtn->setCursor(Qt::PointingHandCursor);

    topLayout->addWidget(saveAttendanceBtn);
    topLayout->addWidget(markAllPresentBtn);
    topLayout->addWidget(markAllAbsentBtn);
    topLayout->addWidget(viewHistoryBtn);
    topLayout->addWidget(refreshBtn);
    mainLayout->addLayout(topLayout);

    // Filter Bar: Bus selector + Date Picker
    auto* filterBar = new QFrame(this);
    filterBar->setStyleSheet("background-color: #ffffff; border: 1px solid #e2e8f0; border-radius: 8px; padding: 12px;");
    auto* barLayout = new QHBoxLayout(filterBar);

    auto* busLbl = new QLabel("Select Bus:", filterBar);
    busLbl->setStyleSheet("font-weight: bold;");
    barLayout->addWidget(busLbl);

    busCombo = new QComboBox(filterBar);
    busCombo->setMinimumWidth(150);
    barLayout->addWidget(busCombo);

    barLayout->addSpacing(30);

    auto* dateLbl = new QLabel("Date:", filterBar);
    dateLbl->setStyleSheet("font-weight: bold;");
    barLayout->addWidget(dateLbl);

    datePicker = new QDateEdit(QDate::currentDate(), filterBar);
    datePicker->setCalendarPopup(true);
    datePicker->setDisplayFormat("yyyy-MM-dd");
    barLayout->addWidget(datePicker);

    barLayout->addStretch();
    mainLayout->addWidget(filterBar);

    // Attendance Table
    attendanceTable = new QTableWidget(this);
    attendanceTable->setColumnCount(6);
    attendanceTable->setHorizontalHeaderLabels({
        "Student ID", "Student Name", "Pickup Point", "Seat No", "Attendance Status", "Remarks"
    });
    attendanceTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    attendanceTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    attendanceTable->verticalHeader()->setVisible(false);
    mainLayout->addWidget(attendanceTable);

    // Connect signals
    connect(busCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AttendanceView::handleBusOrDateChanged);
    connect(datePicker, &QDateEdit::dateChanged, this, &AttendanceView::handleBusOrDateChanged);
    connect(saveAttendanceBtn, &QPushButton::clicked, this, &AttendanceView::handleSaveAttendance);
    connect(markAllPresentBtn, &QPushButton::clicked, this, [this]() { handleMarkAll(true); });
    connect(markAllAbsentBtn, &QPushButton::clicked, this, [this]() { handleMarkAll(false); });
    connect(viewHistoryBtn, &QPushButton::clicked, this, &AttendanceView::showHistoryDialog);
    connect(refreshBtn, &QPushButton::clicked, this, &AttendanceView::refreshData);
}

void AttendanceView::refreshData() {
    QString currentBus = busCombo->currentText();
    busCombo->blockSignals(true);
    busCombo->clear();

    auto buses = fileManager->getAllBuses();
    for (const auto& b : buses) {
        busCombo->addItem(QString::fromStdString(b.getBusId()));
    }
    busCombo->blockSignals(false);

    int idx = busCombo->findText(currentBus);
    if (idx >= 0) {
        busCombo->setCurrentIndex(idx);
    } else if (busCombo->count() > 0) {
        busCombo->setCurrentIndex(0);
    }

    handleBusOrDateChanged();
}

void AttendanceView::handleBusOrDateChanged() {
    if (busCombo->count() == 0) return;

    std::string bId = busCombo->currentText().toStdString();
    std::string dateStr = datePicker->date().toString("yyyy-MM-dd").toStdString();

    auto allStudents = fileManager->getAllStudents();
    auto existingAttendance = fileManager->getAttendanceByBusAndDate(bId, dateStr);

    attendanceTable->setRowCount(0);
    int row = 0;
    for (const auto& s : allStudents) {
        if (s.getBusNumber() == bId) {
            attendanceTable->insertRow(row);

            // Student ID
            attendanceTable->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(s.getId())));
            attendanceTable->item(row, 0)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);

            // Name
            attendanceTable->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(s.getName())));
            attendanceTable->item(row, 1)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);

            // Pickup
            attendanceTable->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(s.getPickupPoint())));
            attendanceTable->item(row, 2)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);

            // Seat No
            QString seatStr = (s.getSeatNumber() > 0) ? QString::number(s.getSeatNumber()) : "None";
            attendanceTable->setItem(row, 3, new QTableWidgetItem(seatStr));
            attendanceTable->item(row, 3)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);

            // Existing status & remarks if marked
            std::string status = "Present";
            std::string remarks = "Regular";
            for (const auto& att : existingAttendance) {
                if (att.getStudentId() == s.getId()) {
                    status = att.getStatus();
                    remarks = att.getRemarks();
                    break;
                }
            }

            // Status Combo Cell Widget
            auto* statusCombo = new QComboBox();
            statusCombo->addItem("Present");
            statusCombo->addItem("Absent");
            statusCombo->setCurrentText(QString::fromStdString(status));
            attendanceTable->setCellWidget(row, 4, statusCombo);

            // Remarks LineEdit Cell Widget
            auto* remarksEdit = new QLineEdit();
            remarksEdit->setText(QString::fromStdString(remarks));
            attendanceTable->setCellWidget(row, 5, remarksEdit);

            row++;
        }
    }
}

void AttendanceView::handleMarkAll(bool present) {
    for (int r = 0; r < attendanceTable->rowCount(); ++r) {
        auto* combo = qobject_cast<QComboBox*>(attendanceTable->cellWidget(r, 4));
        if (combo) {
            combo->setCurrentText(present ? "Present" : "Absent");
        }
    }
}

void AttendanceView::handleSaveAttendance() {
    if (busCombo->count() == 0 || attendanceTable->rowCount() == 0) {
        QMessageBox::information(this, "Empty List", "No students are currently assigned to this bus.");
        return;
    }

    std::string bId = busCombo->currentText().toStdString();
    std::string dateStr = datePicker->date().toString("yyyy-MM-dd").toStdString();

    std::vector<Attendance> records;
    for (int r = 0; r < attendanceTable->rowCount(); ++r) {
        std::string sId = attendanceTable->item(r, 0)->text().toStdString();
        std::string sName = attendanceTable->item(r, 1)->text().toStdString();

        auto* statusCombo = qobject_cast<QComboBox*>(attendanceTable->cellWidget(r, 4));
        std::string status = statusCombo ? statusCombo->currentText().toStdString() : "Present";

        auto* remarksEdit = qobject_cast<QLineEdit*>(attendanceTable->cellWidget(r, 5));
        std::string remarks = remarksEdit ? remarksEdit->text().trimmed().toStdString() : "";

        std::string recId = "ATT-" + dateStr + "-" + sId;
        records.emplace_back(recId, dateStr, bId, sId, sName, status, remarks);
    }

    try {
        fileManager->markAttendance(records);
        QMessageBox::information(this, "Attendance Saved",
            QString("Attendance for Bus %1 on %2 saved successfully (%3 records).")
                .arg(QString::fromStdString(bId), QString::fromStdString(dateStr)).arg(records.size()));
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Error Saving Attendance", QString::fromStdString(e.what()));
    }
}

void AttendanceView::showHistoryDialog() {
    QDialog dlg(this);
    dlg.setWindowTitle("Previous Attendance Records History");
    dlg.setFixedSize(750, 480);

    auto* layout = new QVBoxLayout(&dlg);

    auto* filterLayout = new QHBoxLayout();
    auto* historyBusCombo = new QComboBox(&dlg);
    historyBusCombo->addItem("All Buses");
    auto buses = fileManager->getAllBuses();
    for (const auto& b : buses) {
        historyBusCombo->addItem(QString::fromStdString(b.getBusId()));
    }
    filterLayout->addWidget(new QLabel("Filter Bus:", &dlg));
    filterLayout->addWidget(historyBusCombo);
    filterLayout->addStretch();
    layout->addLayout(filterLayout);

    auto* table = new QTableWidget(&dlg);
    table->setColumnCount(7);
    table->setHorizontalHeaderLabels({
        "Record ID", "Date", "Bus ID", "Student ID", "Student Name", "Status", "Remarks"
    });
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    layout->addWidget(table);

    auto loadHistory = [&](const QString& busFilter) {
        table->setRowCount(0);
        auto allAtt = fileManager->getAllAttendance();
        int r = 0;
        for (const auto& a : allAtt) {
            if (busFilter != "All Buses" && a.getBusId() != busFilter.toStdString()) {
                continue;
            }
            table->insertRow(r);
            table->setItem(r, 0, new QTableWidgetItem(QString::fromStdString(a.getRecordId())));
            table->setItem(r, 1, new QTableWidgetItem(QString::fromStdString(a.getDate())));
            table->setItem(r, 2, new QTableWidgetItem(QString::fromStdString(a.getBusId())));
            table->setItem(r, 3, new QTableWidgetItem(QString::fromStdString(a.getStudentId())));
            table->setItem(r, 4, new QTableWidgetItem(QString::fromStdString(a.getStudentName())));

            auto* statusItem = new QTableWidgetItem(QString::fromStdString(a.getStatus()));
            if (a.getStatus() == "Present") statusItem->setForeground(QBrush(QColor("#16a34a")));
            else statusItem->setForeground(QBrush(QColor("#dc2626")));

            table->setItem(r, 5, statusItem);
            table->setItem(r, 6, new QTableWidgetItem(QString::fromStdString(a.getRemarks())));
            r++;
        }
    };

    connect(historyBusCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [&](int) {
        loadHistory(historyBusCombo->currentText());
    });

    loadHistory("All Buses");

    auto* closeBtn = new QPushButton("Close", &dlg);
    closeBtn->setProperty("class", "primaryButton");
    connect(closeBtn, &QPushButton::clicked, &dlg, &QDialog::accept);
    layout->addWidget(closeBtn, 0, Qt::AlignRight);

    dlg.exec();
}
