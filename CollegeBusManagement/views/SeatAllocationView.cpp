#include "SeatAllocationView.h"
#include "../services/DataStructureManager.h"
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

    assignSeatBtn = new QPushButton("💺 Assign Seat", this);
    assignSeatBtn->setProperty("class", "primaryButton");
    assignSeatBtn->setCursor(Qt::PointingHandCursor);

    quickBookEmptyBtn = new QPushButton("🎟️ Book Empty Seat", this);
    quickBookEmptyBtn->setProperty("class", "successButton");
    quickBookEmptyBtn->setCursor(Qt::PointingHandCursor);

    releaseSeatBtn = new QPushButton("❌ Release Seat", this);
    releaseSeatBtn->setProperty("class", "dangerButton");
    releaseSeatBtn->setCursor(Qt::PointingHandCursor);

    waitingQueueBtn = new QPushButton("⏳ Waiting List (Queue)", this);
    waitingQueueBtn->setProperty("class", "secondaryButton");
    waitingQueueBtn->setCursor(Qt::PointingHandCursor);
    waitingQueueBtn->setToolTip("Manage Student Waiting List via Pure C Circular Queue (FIFO)");

    refreshBtn = new QPushButton("🔄 Refresh", this);
    refreshBtn->setProperty("class", "secondaryButton");
    refreshBtn->setCursor(Qt::PointingHandCursor);

    topLayout->addWidget(assignSeatBtn);
    topLayout->addWidget(quickBookEmptyBtn);
    topLayout->addWidget(releaseSeatBtn);
    topLayout->addWidget(waitingQueueBtn);
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
    connect(waitingQueueBtn, &QPushButton::clicked, this, &SeatAllocationView::handleWaitingQueue);
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
        auto reply = QMessageBox::question(this, "Bus Capacity Full",
            QString("Bus %1 is currently at full capacity (0 seats available).\n\n"
                    "Would you like to open the Waiting List (Pure C Circular Queue) to enqueue a student?")
                    .arg(QString::fromStdString(bId)),
            QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            handleWaitingQueue();
        }
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

            // Record in C Stack for Undo
            DataStructureManager::getInstance()->pushUndoAction(UNDO_SEAT_ALLOCATED, bId, std::to_string(seatNum), sId);

            QMessageBox::information(&dlg, "Seat Allocated", QString("Seat %1 successfully allocated to %2.\n(Logged to C Undo Stack)")
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

        auto* dsm = DataStructureManager::getInstance();
        if (dsm->getWaitingQueueSize() > 0) {
            std::string qStuId, qStuName, qBusId;
            if (dsm->peekWaitingStudent(qStuId, qStuName, qBusId) && (qBusId == bId || qBusId.empty())) {
                auto autoAllocReply = QMessageBox::question(this, "Waiting List (Circular Queue FIFO)",
                    QString("Seat %1 on Bus %2 has just been released.\n\n"
                            "The next student waiting in the Pure C Circular Queue is:\n"
                            "• %1 (%2)\n\n"
                            "Would you like to allocate this seat to the waiting student now in strict FIFO order?")
                            .arg(QString::fromStdString(qStuName), QString::fromStdString(qStuId)),
                    QMessageBox::Yes | QMessageBox::No);

                if (autoAllocReply == QMessageBox::Yes) {
                    dsm->dequeueWaitingStudent(qStuId, qStuName, qBusId);
                    fileManager->allocateSeat(bId, seatNum, qStuId, qStuName);
                    Student waitingStu = fileManager->getStudentById(qStuId);
                    if (!waitingStu.getId().empty()) {
                        waitingStu.setBusNumber(bId);
                        waitingStu.setSeatNumber(seatNum);
                        fileManager->updateStudent(waitingStu);
                    }
                    QMessageBox::information(this, "Queue Auto-Allocated",
                        QString("Seat %1 successfully allocated to queued student %2 (%3)!\n(Pure C Circular Queue: Dequeued in FIFO order)")
                            .arg(seatNum).arg(QString::fromStdString(qStuName), QString::fromStdString(qStuId)));
                } else {
                    QMessageBox::information(this, "Seat Released", QString("Seat %1 is now available.").arg(seatNum));
                }
            } else {
                QMessageBox::information(this, "Seat Released", QString("Seat %1 is now available.").arg(seatNum));
            }
        } else {
            QMessageBox::information(this, "Seat Released", QString("Seat %1 is now available.").arg(seatNum));
        }
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

void SeatAllocationView::handleWaitingQueue() {
    auto* dsm = DataStructureManager::getInstance();
    std::string currentBusId = busSelectorCombo->currentText().toStdString();

    QDialog dlg(this);
    dlg.setWindowTitle("Pure C Circular Queue - Student Seat Waiting List");
    dlg.setFixedSize(580, 480);

    auto* layout = new QVBoxLayout(&dlg);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(12);

    auto* infoLbl = new QLabel(
        "<b>Data Structure Demonstration: Circular Queue (FIFO)</b><br>"
        "Implements a static array ring buffer with <code>front</code> and <code>rear</code> indices modulo capacity.<br>"
        "Enqueues at <i>rear</i> (O(1)) and dequeues at <i>front</i> (O(1)) preserving strict arrival order.", &dlg);
    infoLbl->setStyleSheet("background-color: #fefce8; border: 1px solid #fde047; border-radius: 6px; padding: 10px; color: #854d0e; font-size: 13px;");
    layout->addWidget(infoLbl);

    auto* countLbl = new QLabel(QString("Current Waiting Students in Queue: <b>%1</b>").arg(dsm->getWaitingQueueSize()), &dlg);
    countLbl->setStyleSheet("font-size: 13px; font-weight: bold; color: #0f172a;");
    layout->addWidget(countLbl);

    // Form to enqueue
    auto* enqueueBox = new QGroupBox("Enqueue Student to Waiting List", &dlg);
    auto* enqueueLayout = new QHBoxLayout(enqueueBox);

    auto* studentCombo = new QComboBox(enqueueBox);
    auto allStudents = fileManager->getAllStudents();
    for (const auto& s : allStudents) {
        if (!dsm->isStudentInWaitingQueue(s.getId())) {
            studentCombo->addItem(QString::fromStdString(s.getId() + " - " + s.getName()), QString::fromStdString(s.getId()));
        }
    }
    enqueueLayout->addWidget(studentCombo, 1);

    auto* enqueueBtn = new QPushButton("➕ Enqueue", enqueueBox);
    enqueueBtn->setProperty("class", "primaryButton");
    enqueueLayout->addWidget(enqueueBtn);

    layout->addWidget(enqueueBox);

    // Queue peek info
    auto* queueStatusBox = new QGroupBox("Front of Queue (Next in line to get seat)", &dlg);
    auto* statusLayout = new QVBoxLayout(queueStatusBox);
    auto* peekLbl = new QLabel(queueStatusBox);
    peekLbl->setStyleSheet("font-size: 13px; color: #334155; padding: 6px;");

    auto updateQueueView = [&]() {
        int qSize = dsm->getWaitingQueueSize();
        countLbl->setText(QString("Current Waiting Students in Queue: <b>%1</b>").arg(qSize));

        std::string pId, pName, pBus;
        if (dsm->peekWaitingStudent(pId, pName, pBus)) {
            peekLbl->setText(QString("<b>Next Passenger:</b> %1 (%2) [Target Bus: %3]")
                .arg(QString::fromStdString(pName), QString::fromStdString(pId), QString::fromStdString(pBus)));
        } else {
            peekLbl->setText("<i>Queue is currently empty.</i>");
        }
    };
    updateQueueView();
    statusLayout->addWidget(peekLbl);
    layout->addWidget(queueStatusBox);

    auto* btnLayout = new QHBoxLayout();
    auto* dequeueBtn = new QPushButton("Dequeue First Student", &dlg);
    dequeueBtn->setProperty("class", "dangerButton");

    auto* closeBtn = new QPushButton("Close", &dlg);
    closeBtn->setProperty("class", "secondaryButton");

    btnLayout->addWidget(dequeueBtn);
    btnLayout->addStretch();
    btnLayout->addWidget(closeBtn);
    layout->addLayout(btnLayout);

    connect(enqueueBtn, &QPushButton::clicked, [&]() {
        if (studentCombo->count() == 0) {
            QMessageBox::warning(&dlg, "No Students", "No eligible students available to enqueue.");
            return;
        }

        std::string sId = studentCombo->currentData().toString().toStdString();
        Student s = fileManager->getStudentById(sId);
        if (dsm->enqueueWaitingStudent(sId, s.getName(), currentBusId)) {
            QMessageBox::information(&dlg, "Enqueued Successfully",
                QString("Student %1 (%2) added to Pure C Circular Queue!").arg(QString::fromStdString(s.getName()), QString::fromStdString(sId)));
            studentCombo->removeItem(studentCombo->currentIndex());
            updateQueueView();
        } else {
            QMessageBox::warning(&dlg, "Queue Full / Error", "Failed to enqueue student (Queue may be full or student is already queued).");
        }
    });

    connect(dequeueBtn, &QPushButton::clicked, [&]() {
        std::string sId, sName, sBus;
        if (dsm->dequeueWaitingStudent(sId, sName, sBus)) {
            QMessageBox::information(&dlg, "Dequeued",
                QString("Dequeued %1 (%2) from front of Pure C Circular Queue (FIFO).").arg(QString::fromStdString(sName), QString::fromStdString(sId)));
            updateQueueView();
        } else {
            QMessageBox::information(&dlg, "Queue Empty", "Circular Queue is already empty.");
        }
    });

    connect(closeBtn, &QPushButton::clicked, &dlg, &QDialog::accept);

    dlg.exec();
}


