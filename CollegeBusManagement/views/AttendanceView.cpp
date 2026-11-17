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
    filterBar->setStyleSheet("background-color: #ffffff; border: 1px solid #cbd5e1; border-radius: 8px; padding: 12px;");
    auto* barLayout = new QHBoxLayout(filterBar);

    auto* busLbl = new QLabel("Select Bus:", filterBar);
    busLbl->setStyleSheet("font-weight: bold; color: #0f172a;");
    barLayout->addWidget(busLbl);

    busCombo = new QComboBox(filterBar);
    busCombo->setMinimumWidth(160);
    busCombo->setStyleSheet("font-size: 13px; font-weight: 600; padding: 6px 10px;");
    barLayout->addWidget(busCombo);

    barLayout->addSpacing(30);

    auto* dateLbl = new QLabel("Date:", filterBar);
    dateLbl->setStyleSheet("font-weight: bold; color: #0f172a;");
    barLayout->addWidget(dateLbl);

    datePicker = new QDateEdit(QDate::currentDate(), filterBar);
    datePicker->setCalendarPopup(true);
    datePicker->setDisplayFormat("yyyy-MM-dd");
    datePicker->setStyleSheet("font-size: 13px; font-weight: 600; padding: 6px 10px; min-width: 130px;");
    barLayout->addWidget(datePicker);

    barLayout->addStretch();
    mainLayout->addWidget(filterBar);

    // Attendance Table with Prominent, Wide Remarks Section
    attendanceTable = new QTableWidget(this);
    attendanceTable->setColumnCount(6);
    attendanceTable->setHorizontalHeaderLabels({
        "Student ID", "Student Name", "Pickup Point", "Seat No", "Attendance Status", "Remarks / Boarding Notes"
    });

    // Configure column sizing so remarks is spacious and fully visible
    attendanceTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    attendanceTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Interactive);
    attendanceTable->setColumnWidth(1, 170);
    attendanceTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Interactive);
    attendanceTable->setColumnWidth(2, 170);
    attendanceTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    attendanceTable->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Interactive);
    attendanceTable->setColumnWidth(4, 160);
    attendanceTable->horizontalHeader()->setSectionResizeMode(5, QHeaderView::Stretch); // Stretch remarks

    attendanceTable->verticalHeader()->setDefaultSectionSize(44); // Spacious rows
    attendanceTable->verticalHeader()->setVisible(false);
    attendanceTable->setSelectionBehavior(QAbstractItemView::SelectRows);
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
            auto* idItem = new QTableWidgetItem(QString::fromStdString(s.getId()));
            idItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
            idItem->setFont(QFont(font().family(), 10, QFont::Bold));
            attendanceTable->setItem(row, 0, idItem);

            // Name
            auto* nameItem = new QTableWidgetItem(QString::fromStdString(s.getName()));
            nameItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
            attendanceTable->setItem(row, 1, nameItem);

            // Pickup Point
            auto* pickupItem = new QTableWidgetItem(QString::fromStdString(s.getPickupPoint()));
            pickupItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
            attendanceTable->setItem(row, 2, pickupItem);

            // Seat No
            QString seatStr = (s.getSeatNumber() > 0) ? QString("Seat %1").arg(s.getSeatNumber()) : "None";
            auto* seatItem = new QTableWidgetItem(seatStr);
            seatItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
            seatItem->setTextAlignment(Qt::AlignCenter);
            attendanceTable->setItem(row, 3, seatItem);

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

            // Status Combo Cell Widget with prominent color badges
            auto* statusCombo = new QComboBox();
            statusCombo->addItem("Present");
            statusCombo->addItem("Absent");
            statusCombo->setCurrentText(QString::fromStdString(status));

            auto applyComboStyle = [statusCombo]() {
                if (statusCombo->currentText() == "Present") {
                    statusCombo->setStyleSheet(
                        "QComboBox { font-size: 13px; font-weight: bold; padding: 4px 10px; "
                        "border: 1.5px solid #16a34a; border-radius: 6px; background-color: #f0fdf4; color: #15803d; }"
                    );
                } else {
                    statusCombo->setStyleSheet(
                        "QComboBox { font-size: 13px; font-weight: bold; padding: 4px 10px; "
                        "border: 1.5px solid #dc2626; border-radius: 6px; background-color: #fef2f2; color: #b91c1c; }"
                    );
                }
            };
            applyComboStyle();
            connect(statusCombo, &QComboBox::currentTextChanged, applyComboStyle);
            attendanceTable->setCellWidget(row, 4, statusCombo);

            // Remarks LineEdit Cell Widget (Large, clear, and visible)
            auto* remarksEdit = new QLineEdit();
            remarksEdit->setText(QString::fromStdString(remarks));
            remarksEdit->setPlaceholderText("Enter attendance remarks / boarding notes...");
            remarksEdit->setStyleSheet(
                "QLineEdit { font-size: 13px; font-weight: 500; padding: 6px 12px; "
                "border: 1.5px solid #cbd5e1; border-radius: 6px; background-color: #ffffff; color: #0f172a; }"
                "QLineEdit:focus { border: 1.5px solid #2563eb; background-color: #f8fafc; }"
            );
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
    dlg.setFixedSize(880, 520);

    auto* layout = new QVBoxLayout(&dlg);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(14);

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
        "Record ID", "Date", "Bus ID", "Student ID", "Student Name", "Status", "Remarks / Notes"
    });

    table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    table->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Interactive);
    table->setColumnWidth(4, 160);
    table->horizontalHeader()->setSectionResizeMode(5, QHeaderView::Interactive);
    table->setColumnWidth(5, 120);
    table->horizontalHeader()->setSectionResizeMode(6, QHeaderView::Stretch); // Remarks column stretches wide
    table->verticalHeader()->setDefaultSectionSize(36);
    table->verticalHeader()->setVisible(false);
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
            statusItem->setFont(QFont(font().family(), 10, QFont::Bold));
            if (a.getStatus() == "Present") {
                statusItem->setForeground(QBrush(QColor("#16a34a")));
            } else {
                statusItem->setForeground(QBrush(QColor("#dc2626")));
            }

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
