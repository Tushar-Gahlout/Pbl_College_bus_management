#include "SeatAllocationView.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QDialog>
#include <QFormLayout>
#include <QSpinBox>
#include <QLineEdit>
#include <QScrollArea>
#include <QGroupBox>

SeatAllocationView::SeatAllocationView(FileManager* fm, QWidget* parent)
    : QWidget(parent), fileManager(fm) {
    setupUI();
    refreshData();
}

void SeatAllocationView::setupUI() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 24, 24, 24);
    mainLayout->setSpacing(16);

    // Top Header & Controls
    auto* topLayout = new QHBoxLayout();
    auto* title = new QLabel("Seat Allocation & Bus Layout", this);
    title->setStyleSheet("font-size: 20px; font-weight: bold; color: #0f172a;");
    topLayout->addWidget(title);
    topLayout->addStretch();

    assignSeatBtn = new QPushButton("💺 Assign Regular Seat", this);
    assignSeatBtn->setProperty("class", "primaryButton");
    assignSeatBtn->setCursor(Qt::PointingHandCursor);

    quickBookEmptyBtn = new QPushButton("🎟️ Book Empty Seat for Traveler", this);
    quickBookEmptyBtn->setProperty("class", "successButton");
    quickBookEmptyBtn->setCursor(Qt::PointingHandCursor);

    releaseSeatBtn = new QPushButton("❌ Release Seat", this);
    releaseSeatBtn->setProperty("class", "dangerButton");
    releaseSeatBtn->setCursor(Qt::PointingHandCursor);

    refreshBtn = new QPushButton("🔄 Refresh", this);
    refreshBtn->setProperty("class", "secondaryButton");
    refreshBtn->setCursor(Qt::PointingHandCursor);

    topLayout->addWidget(assignSeatBtn);
    topLayout->addWidget(quickBookEmptyBtn);
    topLayout->addWidget(releaseSeatBtn);
    topLayout->addWidget(refreshBtn);
    mainLayout->addLayout(topLayout);

    // Bus Selector and Statistics Bar
    auto* statsBar = new QFrame(this);
    statsBar->setStyleSheet("background-color: #ffffff; border: 1px solid #cbd5e1; border-radius: 8px; padding: 12px;");
    auto* barLayout = new QHBoxLayout(statsBar);

    auto* busLbl = new QLabel("Select Bus:", statsBar);
    busLbl->setStyleSheet("font-weight: bold; color: #0f172a;");
    barLayout->addWidget(busLbl);

    busSelectorCombo = new QComboBox(statsBar);
    busSelectorCombo->setMinimumWidth(160);
    busSelectorCombo->setStyleSheet("font-size: 13px; font-weight: 600; padding: 6px 10px;");
    barLayout->addWidget(busSelectorCombo);

    barLayout->addSpacing(20);

    shiftInfoLbl = new QLabel("Shift: Full Day", statsBar);
    shiftInfoLbl->setStyleSheet("background-color: #fef3c7; color: #92400e; padding: 4px 12px; border-radius: 12px; font-weight: bold; font-size: 12px;");
    barLayout->addWidget(shiftInfoLbl);

    barLayout->addSpacing(20);
    busCapacityLbl = new QLabel("Capacity: 0", statsBar);
    busCapacityLbl->setStyleSheet("font-weight: 700; color: #334155; font-size: 13px;");
    barLayout->addWidget(busCapacityLbl);

    occupiedCountLbl = new QLabel("Occupied: 0", statsBar);
    occupiedCountLbl->setStyleSheet("font-weight: 700; color: #dc2626; font-size: 13px;");
    barLayout->addWidget(occupiedCountLbl);

    availableCountLbl = new QLabel("Available: 0", statsBar);
    availableCountLbl->setStyleSheet("font-weight: 700; color: #16a34a; font-size: 13px;");
    barLayout->addWidget(availableCountLbl);

    barLayout->addStretch();
    mainLayout->addWidget(statsBar);

    // Split Content: Visual Grid on Left, Allocation Table on Right
    auto* contentLayout = new QHBoxLayout();
    contentLayout->setSpacing(16);

    // Visual Grid Container
    auto* gridBox = new QGroupBox("Bus Seating Layout (Visual 2x2)", this);
    gridBox->setStyleSheet("QGroupBox { font-size: 14px; font-weight: bold; color: #0f172a; }");
    auto* gridBoxLayout = new QVBoxLayout(gridBox);

    auto* scroll = new QScrollArea(gridBox);
    scroll->setWidgetResizable(true);
    scroll->setStyleSheet("border: none; background: transparent;");

    seatGridContainer = new QWidget();
    seatGridLayout = new QGridLayout(seatGridContainer);
    seatGridLayout->setSpacing(10);
    scroll->setWidget(seatGridContainer);
    gridBoxLayout->addWidget(scroll);

    contentLayout->addWidget(gridBox, 1);

    // Allocation Table on Right
    auto* tableBox = new QGroupBox("Allocated Passengers List", this);
    tableBox->setStyleSheet("QGroupBox { font-size: 14px; font-weight: bold; color: #0f172a; }");
    auto* tableBoxLayout = new QVBoxLayout(tableBox);

    seatTable = new QTableWidget(tableBox);
    seatTable->setColumnCount(4);
    seatTable->setHorizontalHeaderLabels({"Seat No", "Student ID", "Student Name", "Allocation Date"});
    seatTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    seatTable->verticalHeader()->setDefaultSectionSize(36);
    seatTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    seatTable->setSelectionMode(QAbstractItemView::SingleSelection);
    seatTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    seatTable->verticalHeader()->setVisible(false);
    tableBoxLayout->addWidget(seatTable);

    contentLayout->addWidget(tableBox, 1);

    mainLayout->addLayout(contentLayout);

    // Connect signals
    connect(busSelectorCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SeatAllocationView::handleBusChanged);
    connect(assignSeatBtn, &QPushButton::clicked, this, &SeatAllocationView::handleAssignSeat);
    connect(quickBookEmptyBtn, &QPushButton::clicked, this, [this]() { handleQuickBookEmpty(-1); });
    connect(releaseSeatBtn, &QPushButton::clicked, this, &SeatAllocationView::handleReleaseSeat);
    connect(refreshBtn, &QPushButton::clicked, this, &SeatAllocationView::refreshData);
}

void SeatAllocationView::refreshData() {
    QString currentSelectedBus = busSelectorCombo->currentText();
    busSelectorCombo->blockSignals(true);
    busSelectorCombo->clear();

    auto buses = fileManager->getAllBuses();
    for (const auto& b : buses) {
        busSelectorCombo->addItem(QString::fromStdString(b.getBusId()));
    }
    busSelectorCombo->blockSignals(false);

    int idx = busSelectorCombo->findText(currentSelectedBus);
    if (idx >= 0) {
        busSelectorCombo->setCurrentIndex(idx);
    } else if (busSelectorCombo->count() > 0) {
        busSelectorCombo->setCurrentIndex(0);
    }

    if (busSelectorCombo->count() > 0) {
        handleBusChanged(busSelectorCombo->currentIndex());
    }
}

void SeatAllocationView::handleBusChanged(int /*index*/) {
    if (busSelectorCombo->count() == 0) return;
    std::string bId = busSelectorCombo->currentText().toStdString();
    Bus b = fileManager->getBusById(bId);
    if (b.getBusId().empty()) return;

    auto seats = fileManager->getSeatsForBus(bId);

    shiftInfoLbl->setText("Shift: " + QString::fromStdString(b.getShiftType()));
    if (b.getShiftType().find("Half") != std::string::npos) {
        shiftInfoLbl->setStyleSheet("background-color: #ffedd5; color: #c2410c; padding: 4px 12px; border-radius: 12px; font-weight: bold; font-size: 12px;");
    } else {
        shiftInfoLbl->setStyleSheet("background-color: #dbeafe; color: #1e40af; padding: 4px 12px; border-radius: 12px; font-weight: bold; font-size: 12px;");
    }

    busCapacityLbl->setText(QString("Capacity: %1").arg(b.getCapacity()));
    occupiedCountLbl->setText(QString("Occupied: %1").arg(seats.size()));
    availableCountLbl->setText(QString("Available: %1").arg(b.getCapacity() - static_cast<int>(seats.size())));

    // Render Table
    seatTable->setRowCount(0);
    for (size_t i = 0; i < seats.size(); ++i) {
        const auto& s = seats[i];
        seatTable->insertRow(i);
        seatTable->setItem(i, 0, new QTableWidgetItem(QString("Seat %1").arg(s.getSeatNumber())));
        seatTable->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(s.getStudentId())));
        seatTable->setItem(i, 2, new QTableWidgetItem(QString::fromStdString(s.getStudentName())));
        seatTable->setItem(i, 3, new QTableWidgetItem(QString::fromStdString(s.getAllocationDate())));
    }

    // Render Visual Layout Grid
    renderSeatGrid(bId);
}

void SeatAllocationView::renderSeatGrid(const std::string& busId) {
    // Clear existing layout items
    QLayoutItem* item;
    while ((item = seatGridLayout->takeAt(0)) != nullptr) {
        if (item->widget()) delete item->widget();
        delete item;
    }

    Bus b = fileManager->getBusById(busId);
    auto seats = fileManager->getSeatsForBus(busId);

    int capacity = b.getCapacity();
    if (capacity <= 0) capacity = 40;

    // Front Driver indicator
    auto* driverRow = new QLabel("🚌 [ Front / Driver Cabin ]", seatGridContainer);
    driverRow->setAlignment(Qt::AlignCenter);
    driverRow->setStyleSheet("background-color: #1e293b; color: #ffffff; font-size: 13px; font-weight: bold; border-radius: 6px; padding: 8px;");
    seatGridLayout->addWidget(driverRow, 0, 0, 1, 5);

    int row = 1;
    for (int seatNum = 1; seatNum <= capacity; seatNum += 4) {
        for (int col = 0; col < 4; ++col) {
            int currentSeat = seatNum + col;
            if (currentSeat > capacity) break;

            int gridCol = (col < 2) ? col : (col + 1); // Column 2 is Aisle

            // Check if currentSeat is allocated
            bool isAllocated = false;
            std::string occupantName = "";
            std::string occupantId = "";
            for (const auto& s : seats) {
                if (s.getSeatNumber() == currentSeat) {
                    isAllocated = true;
                    occupantName = s.getStudentName();
                    occupantId = s.getStudentId();
                    break;
                }
            }

            auto* btn = new QPushButton(QString::number(currentSeat), seatGridContainer);
            btn->setFixedSize(54, 46);

            if (isAllocated) {
                btn->setStyleSheet(
                    "QPushButton { background-color: #dc2626; color: #ffffff; font-size: 14px; font-weight: 700; "
                    "border-radius: 8px; border: 1.5px solid #b91c1c; }"
                    "QPushButton:hover { background-color: #b91c1c; }"
                );
                btn->setToolTip(QString("Seat %1: OCCUPIED by %2 (%3)").arg(currentSeat).arg(QString::fromStdString(occupantName)).arg(QString::fromStdString(occupantId)));
            } else {
                btn->setStyleSheet(
                    "QPushButton { background-color: #16a34a; color: #ffffff; font-size: 14px; font-weight: 700; "
                    "border-radius: 8px; border: 1.5px solid #15803d; }"
                    "QPushButton:hover { background-color: #15803d; }"
                );
                btn->setToolTip(QString("Seat %1: EMPTY & UNASSIGNED\nClick here to book this seat for another traveler!").arg(currentSeat));
                btn->setCursor(Qt::PointingHandCursor);
                connect(btn, &QPushButton::clicked, this, [this, currentSeat]() {
                    handleQuickBookEmpty(currentSeat);
                });
            }

            seatGridLayout->addWidget(btn, row, gridCol);
        }

        // Add Aisle spacer label in column 2
        auto* aisleLbl = new QLabel(" | ", seatGridContainer);
        aisleLbl->setAlignment(Qt::AlignCenter);
        aisleLbl->setStyleSheet("color: #94a3b8; font-weight: bold;");
        seatGridLayout->addWidget(aisleLbl, row, 2);

        row++;
    }
}

void SeatAllocationView::handleAssignSeat() {
    if (busSelectorCombo->count() == 0) return;
    std::string bId = busSelectorCombo->currentText().toStdString();
    Bus b = fileManager->getBusById(bId);

    if (b.isFull()) {
        QMessageBox::warning(this, "Bus Full", QString("Bus %1 has reached its maximum capacity (%2 seats)!").arg(QString::fromStdString(bId)).arg(b.getCapacity()));
        return;
    }

    QDialog dlg(this);
    dlg.setWindowTitle(QString("Assign Seat on Bus %1").arg(QString::fromStdString(bId)));
    dlg.setFixedSize(440, 280);

    auto* layout = new QVBoxLayout(&dlg);
    layout->setSpacing(14);
    layout->setContentsMargins(20, 20, 20, 20);

    auto* formLayout = new QFormLayout();
    formLayout->setSpacing(10);

    // Available seats combo
    auto seats = fileManager->getSeatsForBus(bId);
    auto* seatCombo = new QComboBox(&dlg);
    for (int i = 1; i <= b.getCapacity(); ++i) {
        bool occupied = false;
        for (const auto& s : seats) {
            if (s.getSeatNumber() == i) {
                occupied = true;
                break;
            }
        }
        if (!occupied) {
            seatCombo->addItem(QString("Seat %1").arg(i), i);
        }
    }

    // Student combo (students needing a seat)
    auto* studentCombo = new QComboBox(&dlg);
    auto allStudents = fileManager->getAllStudents();
    for (const auto& s : allStudents) {
        if (s.getBusNumber() == bId || s.getBusNumber() == "None" || s.getSeatNumber() == 0) {
            QString label = QString::fromStdString(s.getId()) + " - " + QString::fromStdString(s.getName());
            studentCombo->addItem(label, QString::fromStdString(s.getId()));
        }
    }

    if (seatCombo->count() == 0) {
        QMessageBox::warning(this, "No Seats Available", "All seats on this bus are already allocated.");
        return;
    }

    if (studentCombo->count() == 0) {
        QMessageBox::warning(this, "No Students", "No unassigned students available to allocate.");
        return;
    }

    formLayout->addRow("Bus ID:", new QLabel(QString::fromStdString(bId) + " (" + QString::fromStdString(b.getShiftType()) + ")", &dlg));
    formLayout->addRow("Available Seat:", seatCombo);
    formLayout->addRow("Student:", studentCombo);
    layout->addLayout(formLayout);

    auto* btnLayout = new QHBoxLayout();
    auto* saveBtn = new QPushButton("Allocate Seat", &dlg);
    saveBtn->setProperty("class", "primaryButton");
    auto* cancelBtn = new QPushButton("Cancel", &dlg);
    cancelBtn->setProperty("class", "secondaryButton");

    btnLayout->addStretch();
    btnLayout->addWidget(cancelBtn);
    btnLayout->addWidget(saveBtn);
    layout->addLayout(btnLayout);

    connect(cancelBtn, &QPushButton::clicked, &dlg, &QDialog::reject);
    connect(saveBtn, &QPushButton::clicked, [&]() {
        int seatNum = seatCombo->currentData().toInt();
        std::string sId = studentCombo->currentData().toString().toStdString();
        Student s = fileManager->getStudentById(sId);

        try {
            fileManager->allocateSeat(bId, seatNum, sId, s.getName());

            s.setBusNumber(bId);
            s.setSeatNumber(seatNum);
            fileManager->updateStudent(s);

            QMessageBox::information(&dlg, "Seat Allocated", QString("Seat %1 successfully allocated to %2.")
                .arg(seatNum).arg(QString::fromStdString(s.getName())));
            dlg.accept();
        } catch (const std::exception& e) {
            QMessageBox::critical(&dlg, "Allocation Error", QString::fromStdString(e.what()));
        }
    });

    if (dlg.exec() == QDialog::Accepted) {
        refreshData();
    }
}

void SeatAllocationView::handleReleaseSeat() {
    int selectedRow = seatTable->currentRow();
    if (selectedRow < 0) {
        QMessageBox::warning(this, "Selection Required", "Please select a seat from the allocated passengers list to release.");
        return;
    }

    QString seatStr = seatTable->item(selectedRow, 0)->text();
    QString sId = seatTable->item(selectedRow, 1)->text();
    QString sName = seatTable->item(selectedRow, 2)->text();

    int seatNum = seatStr.replace("Seat ", "").trimmed().toInt();
    std::string bId = busSelectorCombo->currentText().toStdString();

    auto reply = QMessageBox::question(this, "Confirm Seat Release",
        QString("Are you sure you want to release Seat %1 currently allocated to %2 (%3)?")
            .arg(seatNum).arg(sName, sId),
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        fileManager->releaseSeat(bId, seatNum);

        Student s = fileManager->getStudentById(sId.toStdString());
        if (!s.getId().empty()) {
            s.setSeatNumber(0);
            fileManager->updateStudent(s);
        }

        QMessageBox::information(this, "Seat Released", QString("Seat %1 is now available.").arg(seatNum));
        refreshData();
    }
}

void SeatAllocationView::handleQuickBookEmpty(int preselectedSeat) {
    if (busSelectorCombo->count() == 0) return;
    std::string bId = busSelectorCombo->currentText().toStdString();
    Bus b = fileManager->getBusById(bId);

    auto emptySeats = fileManager->getEmptySeatsForBus(bId);
    if (emptySeats.empty()) {
        QMessageBox::warning(this, "Bus Full", QString("Bus %1 has no empty seats remaining!").arg(QString::fromStdString(bId)));
        return;
    }

    QDialog dlg(this);
    dlg.setWindowTitle(QString("Book Empty Seat - Bus %1 (%2)").arg(QString::fromStdString(bId), QString::fromStdString(b.getShiftType())));
    dlg.setFixedSize(480, 320);

    auto* layout = new QVBoxLayout(&dlg);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(12);

    auto* formLayout = new QFormLayout();
    formLayout->setSpacing(10);

    auto* seatCombo = new QComboBox(&dlg);
    for (int sNum : emptySeats) {
        seatCombo->addItem(QString("Seat #%1 (Empty)").arg(sNum), sNum);
        if (preselectedSeat == sNum) {
            seatCombo->setCurrentIndex(seatCombo->count() - 1);
        }
    }

    auto* nameEdit = new QLineEdit(&dlg);
    nameEdit->setPlaceholderText("Passenger Name (e.g. Guest / Traveling Student)");

    auto* contactEdit = new QLineEdit(&dlg);
    contactEdit->setPlaceholderText("Contact Phone or Student ID");

    auto* travelCombo = new QComboBox(&dlg);
    travelCombo->addItems({"Day Pass Traveler", "Ad-hoc Guest", "Faculty / Staff", "Substitute Student"});

    formLayout->addRow("Bus ID:", new QLabel(QString::fromStdString(b.getBusId()) + " (" + QString::fromStdString(b.getShiftType()) + ")", &dlg));
    formLayout->addRow("Select Empty Seat *:", seatCombo);
    formLayout->addRow("Passenger Name *:", nameEdit);
    formLayout->addRow("Contact / ID *:", contactEdit);
    formLayout->addRow("Travel Pass Type:", travelCombo);

    layout->addLayout(formLayout);

    auto* btnLayout = new QHBoxLayout();
    auto* bookBtn = new QPushButton("Confirm Seat Booking", &dlg);
    bookBtn->setProperty("class", "primaryButton");
    auto* cancelBtn = new QPushButton("Cancel", &dlg);
    cancelBtn->setProperty("class", "secondaryButton");

    btnLayout->addStretch();
    btnLayout->addWidget(cancelBtn);
    btnLayout->addWidget(bookBtn);
    layout->addLayout(btnLayout);

    connect(cancelBtn, &QPushButton::clicked, &dlg, &QDialog::reject);
    connect(bookBtn, &QPushButton::clicked, [&]() {
        QString name = nameEdit->text().trimmed();
        QString contact = contactEdit->text().trimmed();

        if (name.isEmpty() || contact.isEmpty()) {
            QMessageBox::warning(&dlg, "Input Error", "Please provide passenger name and contact.");
            return;
        }

        int seatNum = seatCombo->currentData().toInt();
        std::string travelType = travelCombo->currentText().toStdString();

        try {
            fileManager->quickBookEmptySeat(bId, seatNum, name.toStdString(), contact.toStdString(), travelType);
            QMessageBox::information(&dlg, "Seat Booked!",
                QString("Seat #%1 successfully booked on Bus %2 for %3 (%4).")
                    .arg(seatNum).arg(QString::fromStdString(bId), name, QString::fromStdString(travelType)));
            dlg.accept();
        } catch (const std::exception& e) {
            QMessageBox::critical(&dlg, "Booking Error", QString::fromStdString(e.what()));
        }
    });

    if (dlg.exec() == QDialog::Accepted) {
        refreshData();
    }
}

void SeatAllocationView::setReadOnly(bool readOnly) {
    assignSeatBtn->setVisible(!readOnly);
    releaseSeatBtn->setVisible(!readOnly);
}

void SeatAllocationView::selectBus(const QString& busId) {
    int idx = busSelectorCombo->findText(busId);
    if (idx != -1) {
        busSelectorCombo->setCurrentIndex(idx);
    }
}

