#include "BusManagementView.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QDialog>
#include <QFormLayout>
#include <QSpinBox>
#include <QComboBox>
#include <QLabel>

BusManagementView::BusManagementView(FileManager* fm, QWidget* parent)
    : QWidget(parent), fileManager(fm) {
    setupUI();
    refreshData();
}

void BusManagementView::setupUI() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(16, 16, 16, 16);
    mainLayout->setSpacing(12);

    tabWidget = new QTabWidget(this);

    // Tab 1: Fleet Roster & Management
    auto* listTab = new QWidget(tabWidget);
    auto* listLayout = new QVBoxLayout(listTab);
    listLayout->setContentsMargins(16, 16, 16, 16);
    listLayout->setSpacing(14);

    // Header Row
    auto* topLayout = new QHBoxLayout();
    auto* title = new QLabel("Bus Fleet Management", listTab);
    title->setStyleSheet("font-size: 20px; font-weight: bold; color: #78350f;");
    topLayout->addWidget(title);
    topLayout->addStretch();
    listLayout->addLayout(topLayout);

    // Action Buttons Row
    auto* actionLayout = new QHBoxLayout();
    addBtn = new QPushButton("➕ Add Bus", listTab);
    addBtn->setProperty("class", "primaryButton");
    addBtn->setCursor(Qt::PointingHandCursor);

    editBtn = new QPushButton("✏️ Edit Bus", listTab);
    editBtn->setProperty("class", "secondaryButton");
    editBtn->setCursor(Qt::PointingHandCursor);

    deleteBtn = new QPushButton("🗑️ Delete Bus", listTab);
    deleteBtn->setProperty("class", "dangerButton");
    deleteBtn->setCursor(Qt::PointingHandCursor);

    viewPassengersBtn = new QPushButton("👥 Passengers", listTab);
    viewPassengersBtn->setProperty("class", "secondaryButton");
    viewPassengersBtn->setCursor(Qt::PointingHandCursor);

    trackMapBtn = new QPushButton("🗺️ Live Map", listTab);
    trackMapBtn->setProperty("class", "primaryButton");
    trackMapBtn->setCursor(Qt::PointingHandCursor);

    refreshBtn = new QPushButton("🔄 Refresh", listTab);
    refreshBtn->setProperty("class", "secondaryButton");
    refreshBtn->setCursor(Qt::PointingHandCursor);

    actionLayout->addWidget(addBtn);
    actionLayout->addWidget(editBtn);
    actionLayout->addWidget(deleteBtn);
    actionLayout->addWidget(viewPassengersBtn);
    actionLayout->addWidget(trackMapBtn);
    actionLayout->addWidget(refreshBtn);
    actionLayout->addStretch();
    listLayout->addLayout(actionLayout);

    // Search bar
    searchEdit = new QLineEdit(listTab);
    searchEdit->setPlaceholderText("🔍 Search bus by ID, Reg Number, Driver or Status...");
    searchEdit->setClearButtonEnabled(true);
    listLayout->addWidget(searchEdit);

    // Bus Table
    busTable = new QTableWidget(listTab);
    busTable->setColumnCount(10);
    busTable->setHorizontalHeaderLabels({
        "Bus ID", "Registration No", "Capacity", "Occupied", "Empty Seats", "Shift Schedule", "Assigned Driver", "Route ID", "Status", "Live Location & Map"
    });
    busTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    busTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    busTable->setSelectionMode(QAbstractItemView::SingleSelection);
    busTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    busTable->verticalHeader()->setVisible(false);
    listLayout->addWidget(busTable);

    // Tab 2: Embedded Live Bus Location GPS Map
    embeddedMapWidget = new BusMapWidget(fileManager, tabWidget);

    tabWidget->addTab(listTab, "📋 Bus Fleet Overview");
    tabWidget->addTab(embeddedMapWidget, "🗺️ Live Bus Location & Route GPS Map");
    mainLayout->addWidget(tabWidget);

    // Connect signals
    connect(searchEdit, &QLineEdit::textChanged, this, &BusManagementView::handleSearch);
    connect(addBtn, &QPushButton::clicked, this, &BusManagementView::handleAdd);
    connect(editBtn, &QPushButton::clicked, this, &BusManagementView::handleEdit);
    connect(deleteBtn, &QPushButton::clicked, this, &BusManagementView::handleDelete);
    connect(viewPassengersBtn, &QPushButton::clicked, this, &BusManagementView::handleViewPassengers);

    connect(trackMapBtn, &QPushButton::clicked, this, [this]() {
        int row = busTable->currentRow();
        QString bId = "";
        if (row >= 0 && busTable->item(row, 0)) {
            bId = busTable->item(row, 0)->text();
        } else if (busTable->rowCount() > 0 && busTable->item(0, 0)) {
            bId = busTable->item(0, 0)->text();
        }
        if (!bId.isEmpty()) {
            embeddedMapWidget->selectBus(bId);
        }
        tabWidget->setCurrentIndex(1); // Switch to Map Tab!
        emit requestTrackBus(bId);
    });

    connect(busTable, &QTableWidget::cellClicked, this, [this](int row, int col) {
        if (col == 9 && row >= 0 && busTable->item(row, 0)) {
            QString bId = busTable->item(row, 0)->text();
            embeddedMapWidget->selectBus(bId);
            tabWidget->setCurrentIndex(1);
        }
    });

    connect(refreshBtn, &QPushButton::clicked, this, &BusManagementView::refreshData);
    connect(busTable, &QTableWidget::doubleClicked, this, [this](const QModelIndex& index) {
        if (index.column() == 9 && index.row() >= 0 && busTable->item(index.row(), 0)) {
            QString bId = busTable->item(index.row(), 0)->text();
            embeddedMapWidget->selectBus(bId);
            tabWidget->setCurrentIndex(1);
        } else {
            handleEdit();
        }
    });
}

void BusManagementView::refreshData() {
    handleSearch();
    if (embeddedMapWidget) {
        embeddedMapWidget->refreshData();
    }
}

void BusManagementView::handleSearch() {
    std::string query = searchEdit->text().trimmed().toStdString();
    std::vector<Bus> list = fileManager->searchBuses(query);

    busTable->setRowCount(0);
    for (size_t i = 0; i < list.size(); ++i) {
        const auto& b = list[i];
        busTable->insertRow(i);

        busTable->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(b.getBusId())));
        busTable->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(b.getRegNumber())));
        busTable->setItem(i, 2, new QTableWidgetItem(QString::number(b.getCapacity())));
        busTable->setItem(i, 3, new QTableWidgetItem(QString::number(b.getOccupiedSeats())));

        auto* emptySeatsItem = new QTableWidgetItem(QString("🟢 %1 Empty").arg(b.getAvailableSeats()));
        emptySeatsItem->setForeground(QBrush(QColor("#16a34a")));
        busTable->setItem(i, 4, emptySeatsItem);

        auto* shiftItem = new QTableWidgetItem(QString::fromStdString(b.getShiftType()));
        if (b.getShiftType().find("Half") != std::string::npos) {
            shiftItem->setForeground(QBrush(QColor("#d97706")));
        } else {
            shiftItem->setForeground(QBrush(QColor("#2563eb")));
        }
        busTable->setItem(i, 5, shiftItem);

        busTable->setItem(i, 6, new QTableWidgetItem(QString::fromStdString(b.getAssignedDriver())));
        busTable->setItem(i, 7, new QTableWidgetItem(QString::fromStdString(b.getRouteId())));

        auto* statusItem = new QTableWidgetItem(QString::fromStdString(b.getStatus()));
        if (b.getStatus() == "Active") {
            statusItem->setForeground(QBrush(QColor("#16a34a")));
        } else if (b.getStatus() == "Maintenance") {
            statusItem->setForeground(QBrush(QColor("#d97706")));
        } else {
            statusItem->setForeground(QBrush(QColor("#dc2626")));
        }
        busTable->setItem(i, 8, statusItem);
        busTable->setItem(i, 9, new QTableWidgetItem(QString("📍 ") + QString::fromStdString(b.getCurrentLocation())));
    }
}

void BusManagementView::handleAdd() {
    showBusFormDialog(nullptr);
}

void BusManagementView::handleEdit() {
    int row = busTable->currentRow();
    if (row < 0) {
        QMessageBox::information(this, "Select Bus", "Please select a bus from the table to edit.");
        return;
    }

    std::string bId = busTable->item(row, 0)->text().toStdString();
    Bus b = fileManager->getBusById(bId);
    if (!b.getBusId().empty()) {
        showBusFormDialog(&b);
    }
}

void BusManagementView::handleDelete() {
    int row = busTable->currentRow();
    if (row < 0) {
        QMessageBox::information(this, "Select Bus", "Please select a bus from the table to delete.");
        return;
    }

    QString bId = busTable->item(row, 0)->text();
    auto reply = QMessageBox::question(this, "Confirm Deletion",
        QString("Are you sure you want to delete Bus '%1'?\nAny assigned students will have their bus assignment reset.").arg(bId),
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        try {
            fileManager->deleteBus(bId.toStdString());
            QMessageBox::information(this, "Success", "Bus deleted successfully.");
            refreshData();
        } catch (const std::exception& e) {
            QMessageBox::critical(this, "Error", QString("Failed to delete bus: %1").arg(e.what()));
        }
    }
}

void BusManagementView::handleViewPassengers() {
    int row = busTable->currentRow();
    if (row < 0) {
        QMessageBox::information(this, "Select Bus", "Please select a bus to view its passengers.");
        return;
    }
    std::string bId = busTable->item(row, 0)->text().toStdString();
    showPassengersDialog(bId);
}

void BusManagementView::showPassengersDialog(const std::string& busId) {
    QDialog dlg(this);
    dlg.setWindowTitle(QString("Passenger List - Bus %1").arg(QString::fromStdString(busId)));
    dlg.setFixedSize(650, 400);

    auto* layout = new QVBoxLayout(&dlg);
    auto* table = new QTableWidget(&dlg);
    table->setColumnCount(5);
    table->setHorizontalHeaderLabels({"Student ID", "Name", "Department", "Pickup Point", "Seat Number"});
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);

    auto allStudents = fileManager->getAllStudents();
    int count = 0;
    for (const auto& s : allStudents) {
        if (s.getBusNumber() == busId) {
            table->insertRow(count);
            table->setItem(count, 0, new QTableWidgetItem(QString::fromStdString(s.getId())));
            table->setItem(count, 1, new QTableWidgetItem(QString::fromStdString(s.getName())));
            table->setItem(count, 2, new QTableWidgetItem(QString::fromStdString(s.getDepartment())));
            table->setItem(count, 3, new QTableWidgetItem(QString::fromStdString(s.getPickupPoint())));
            table->setItem(count, 4, new QTableWidgetItem(QString::number(s.getSeatNumber())));
            count++;
        }
    }

    auto* lbl = new QLabel(QString("Total Passengers assigned to Bus %1: %2").arg(QString::fromStdString(busId)).arg(count), &dlg);
    lbl->setStyleSheet("font-weight: bold; font-size: 14px;");

    layout->addWidget(lbl);
    layout->addWidget(table);

    auto* closeBtn = new QPushButton("Close", &dlg);
    closeBtn->setProperty("class", "primaryButton");
    connect(closeBtn, &QPushButton::clicked, &dlg, &QDialog::accept);
    layout->addWidget(closeBtn, 0, Qt::AlignRight);

    dlg.exec();
}

void BusManagementView::showBusFormDialog(Bus* existing) {
    bool isEditing = (existing != nullptr);

    QDialog dlg(this);
    dlg.setWindowTitle(isEditing ? "Edit Bus Details" : "Add New Bus");
    dlg.setFixedSize(450, 420);

    auto* layout = new QVBoxLayout(&dlg);
    layout->setSpacing(14);
    layout->setContentsMargins(20, 20, 20, 20);

    auto* formLayout = new QFormLayout();
    formLayout->setSpacing(10);

    auto* idEdit = new QLineEdit(&dlg);
    idEdit->setPlaceholderText("e.g. BUS05");
    if (isEditing) {
        idEdit->setText(QString::fromStdString(existing->getBusId()));
        idEdit->setEnabled(false);
    }

    auto* regEdit = new QLineEdit(&dlg);
    regEdit->setPlaceholderText("e.g. DL-01-AB-1234");
    if (isEditing) regEdit->setText(QString::fromStdString(existing->getRegNumber()));

    auto* capSpin = new QSpinBox(&dlg);
    capSpin->setRange(10, 80);
    capSpin->setValue(isEditing ? existing->getCapacity() : 40);

    // Driver selector
    auto* driverCombo = new QComboBox(&dlg);
    driverCombo->addItem("None");
    auto drivers = fileManager->getAllDrivers();
    for (const auto& d : drivers) {
        driverCombo->addItem(QString::fromStdString(d.getName()));
    }
    if (isEditing) driverCombo->setCurrentText(QString::fromStdString(existing->getAssignedDriver()));

    // Route selector
    auto* routeCombo = new QComboBox(&dlg);
    routeCombo->addItem("None");
    auto routes = fileManager->getAllRoutes();
    for (const auto& r : routes) {
        routeCombo->addItem(QString::fromStdString(r.getRouteId()) + " - " + QString::fromStdString(r.getRouteName()),
                            QString::fromStdString(r.getRouteId()));
    }
    if (isEditing) {
        for (int i = 0; i < routeCombo->count(); ++i) {
            if (routeCombo->itemData(i).toString().toStdString() == existing->getRouteId()) {
                routeCombo->setCurrentIndex(i);
                break;
            }
        }
    }

    auto* statusCombo = new QComboBox(&dlg);
    statusCombo->addItems({"Active", "Maintenance", "Inactive"});
    if (isEditing) statusCombo->setCurrentText(QString::fromStdString(existing->getStatus()));

    auto* shiftCombo = new QComboBox(&dlg);
    shiftCombo->addItems({"Full Day", "Half Day (Morning)", "Half Day (Evening)"});
    if (isEditing) shiftCombo->setCurrentText(QString::fromStdString(existing->getShiftType()));

    auto* locationEdit = new QLineEdit(&dlg);
    locationEdit->setPlaceholderText("Current stop or location (e.g. Sector 14 / Campus)");
    locationEdit->setText(isEditing ? QString::fromStdString(existing->getCurrentLocation()) : "College Campus");

    formLayout->addRow("Bus ID *:", idEdit);
    formLayout->addRow("Registration No *:", regEdit);
    formLayout->addRow("Total Capacity *:", capSpin);
    formLayout->addRow("Shift Schedule *:", shiftCombo);
    formLayout->addRow("Assigned Driver:", driverCombo);
    formLayout->addRow("Assigned Route:", routeCombo);
    formLayout->addRow("Current Status:", statusCombo);
    formLayout->addRow("Current Location:", locationEdit);

    layout->addLayout(formLayout);

    auto* btnLayout = new QHBoxLayout();
    auto* saveBtn = new QPushButton(isEditing ? "Save Changes" : "Add Bus", &dlg);
    saveBtn->setProperty("class", "primaryButton");
    auto* cancelBtn = new QPushButton("Cancel", &dlg);
    cancelBtn->setProperty("class", "secondaryButton");

    btnLayout->addStretch();
    btnLayout->addWidget(cancelBtn);
    btnLayout->addWidget(saveBtn);
    layout->addLayout(btnLayout);

    connect(cancelBtn, &QPushButton::clicked, &dlg, &QDialog::reject);
    connect(saveBtn, &QPushButton::clicked, [&]() {
        QString id = idEdit->text().trimmed();
        QString reg = regEdit->text().trimmed();

        if (id.isEmpty() || reg.isEmpty()) {
            QMessageBox::warning(&dlg, "Validation Error", "Please fill in Bus ID and Registration Number.");
            return;
        }

        std::string selectedDriver = driverCombo->currentText().toStdString();
        std::string selectedRoute = routeCombo->currentData().toString().toStdString();
        if (selectedRoute.empty()) selectedRoute = "None";

        int occupied = isEditing ? existing->getOccupiedSeats() : 0;
        std::string shiftType = shiftCombo->currentText().toStdString();
        std::string location = locationEdit->text().trimmed().toStdString();
        if (location.empty()) location = "College Campus";

        Bus b(id.toStdString(), reg.toStdString(), capSpin->value(), occupied,
              selectedDriver, selectedRoute, statusCombo->currentText().toStdString(),
              shiftType, location);

        try {
            if (isEditing) {
                fileManager->updateBus(b);
                QMessageBox::information(&dlg, "Success", "Bus updated successfully!");
            } else {
                fileManager->addBus(b);
                QMessageBox::information(&dlg, "Success", "Bus added successfully!");
            }
            dlg.accept();
        } catch (const std::exception& e) {
            QMessageBox::critical(&dlg, "Operation Failed", QString::fromStdString(e.what()));
        }
    });

    if (dlg.exec() == QDialog::Accepted) {
        refreshData();
    }
}
