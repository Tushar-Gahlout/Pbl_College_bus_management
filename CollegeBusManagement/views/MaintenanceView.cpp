#include "MaintenanceView.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QDialog>
#include <QFormLayout>
#include <QDoubleSpinBox>
#include <QDateEdit>
#include <QLineEdit>
#include <iomanip>
#include <sstream>

MaintenanceView::MaintenanceView(FileManager* fm, QWidget* parent)
    : QWidget(parent), fileManager(fm) {
    setupUI();
    refreshData();
}

void MaintenanceView::setupUI() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 24, 24, 24);
    mainLayout->setSpacing(16);

    // Top Header & Action Buttons
    auto* topLayout = new QHBoxLayout();
    auto* title = new QLabel("Bus Maintenance & Servicing", this);
    title->setStyleSheet("font-size: 20px; font-weight: bold; color: #0f172a;");
    topLayout->addWidget(title);
    topLayout->addStretch();

    addRecordBtn = new QPushButton("➕ Add Service Record", this);
    addRecordBtn->setProperty("class", "primaryButton");
    addRecordBtn->setCursor(Qt::PointingHandCursor);

    editRecordBtn = new QPushButton("✏️ Edit Record", this);
    editRecordBtn->setProperty("class", "secondaryButton");
    editRecordBtn->setCursor(Qt::PointingHandCursor);

    deleteRecordBtn = new QPushButton("🗑️ Delete Record", this);
    deleteRecordBtn->setProperty("class", "dangerButton");
    deleteRecordBtn->setCursor(Qt::PointingHandCursor);

    toggleBusStatusBtn = new QPushButton("🛠️ Toggle Bus Status", this);
    toggleBusStatusBtn->setProperty("class", "secondaryButton");
    toggleBusStatusBtn->setCursor(Qt::PointingHandCursor);

    refreshBtn = new QPushButton("🔄 Refresh", this);
    refreshBtn->setProperty("class", "secondaryButton");
    refreshBtn->setCursor(Qt::PointingHandCursor);

    topLayout->addWidget(addRecordBtn);
    topLayout->addWidget(editRecordBtn);
    topLayout->addWidget(deleteRecordBtn);
    topLayout->addWidget(toggleBusStatusBtn);
    topLayout->addWidget(refreshBtn);
    mainLayout->addLayout(topLayout);

    // Filter Bar & Metrics
    auto* filterBar = new QFrame(this);
    filterBar->setStyleSheet("background-color: #ffffff; border: 1px solid #e2e8f0; border-radius: 8px; padding: 12px;");
    auto* barLayout = new QHBoxLayout(filterBar);

    auto* busLbl = new QLabel("Filter by Bus:", filterBar);
    busLbl->setStyleSheet("font-weight: bold;");
    barLayout->addWidget(busLbl);

    busFilterCombo = new QComboBox(filterBar);
    busFilterCombo->setMinimumWidth(150);
    busFilterCombo->addItem("All Buses");
    barLayout->addWidget(busFilterCombo);

    barLayout->addSpacing(30);

    totalCostLbl = new QLabel("Total Maintenance Spend: ₹0.00", filterBar);
    totalCostLbl->setStyleSheet("font-weight: 600; color: #dc2626; font-size: 13px;");
    barLayout->addWidget(totalCostLbl);

    barLayout->addSpacing(20);

    serviceCountLbl = new QLabel("Total Services: 0", filterBar);
    serviceCountLbl->setStyleSheet("font-weight: 600; color: #475569; font-size: 13px;");
    barLayout->addWidget(serviceCountLbl);

    barLayout->addStretch();
    mainLayout->addWidget(filterBar);

    // Maintenance Table
    maintenanceTable = new QTableWidget(this);
    maintenanceTable->setColumnCount(7);
    maintenanceTable->setHorizontalHeaderLabels({
        "Record ID", "Bus ID", "Service Type", "Cost (₹)", "Service Date", "Workshop / Garage", "Remarks"
    });
    maintenanceTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    maintenanceTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    maintenanceTable->setSelectionMode(QAbstractItemView::SingleSelection);
    maintenanceTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    maintenanceTable->verticalHeader()->setVisible(false);
    mainLayout->addWidget(maintenanceTable);

    // Connect signals
    connect(busFilterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MaintenanceView::handleFilterChanged);
    connect(addRecordBtn, &QPushButton::clicked, this, &MaintenanceView::handleAdd);
    connect(editRecordBtn, &QPushButton::clicked, this, &MaintenanceView::handleEdit);
    connect(deleteRecordBtn, &QPushButton::clicked, this, &MaintenanceView::handleDelete);
    connect(toggleBusStatusBtn, &QPushButton::clicked, this, &MaintenanceView::handleToggleStatus);
    connect(refreshBtn, &QPushButton::clicked, this, &MaintenanceView::refreshData);
    connect(maintenanceTable, &QTableWidget::doubleClicked, this, &MaintenanceView::handleEdit);
}

void MaintenanceView::refreshData() {
    QString current = busFilterCombo->currentText();
    busFilterCombo->blockSignals(true);
    busFilterCombo->clear();
    busFilterCombo->addItem("All Buses");

    auto buses = fileManager->getAllBuses();
    for (const auto& b : buses) {
        busFilterCombo->addItem(QString::fromStdString(b.getBusId()));
    }
    busFilterCombo->blockSignals(false);

    int idx = busFilterCombo->findText(current);
    if (idx >= 0) busFilterCombo->setCurrentIndex(idx);

    handleFilterChanged();
}

void MaintenanceView::handleFilterChanged() {
    std::string selectedBus = busFilterCombo->currentText().toStdString();
    std::vector<MaintenanceRecord> list;
    if (selectedBus == "All Buses" || selectedBus.empty()) {
        list = fileManager->getAllMaintenance();
    } else {
        list = fileManager->getMaintenanceForBus(selectedBus);
    }

    double totalCost = 0.0;
    for (const auto& m : list) {
        totalCost += m.getCost();
    }

    std::ostringstream ss;
    ss << "Total Maintenance Spend: ₹" << std::fixed << std::setprecision(2) << totalCost;
    totalCostLbl->setText(QString::fromStdString(ss.str()));
    serviceCountLbl->setText(QString("Total Services: %1").arg(list.size()));

    maintenanceTable->setRowCount(0);
    for (size_t i = 0; i < list.size(); ++i) {
        const auto& m = list[i];
        maintenanceTable->insertRow(i);

        maintenanceTable->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(m.getRecordId())));
        maintenanceTable->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(m.getBusId())));
        maintenanceTable->setItem(i, 2, new QTableWidgetItem(QString::fromStdString(m.getServiceType())));

        std::ostringstream costStream;
        costStream << "₹" << std::fixed << std::setprecision(2) << m.getCost();
        maintenanceTable->setItem(i, 3, new QTableWidgetItem(QString::fromStdString(costStream.str())));

        maintenanceTable->setItem(i, 4, new QTableWidgetItem(QString::fromStdString(m.getServiceDate())));
        maintenanceTable->setItem(i, 5, new QTableWidgetItem(QString::fromStdString(m.getWorkshop())));
        maintenanceTable->setItem(i, 6, new QTableWidgetItem(QString::fromStdString(m.getRemarks())));
    }
}

void MaintenanceView::handleAdd() {
    showMaintenanceFormDialog(nullptr);
}

void MaintenanceView::handleEdit() {
    int row = maintenanceTable->currentRow();
    if (row < 0) {
        QMessageBox::information(this, "Select Record", "Please select a maintenance record from the table to edit.");
        return;
    }

    std::string rId = maintenanceTable->item(row, 0)->text().toStdString();
    auto all = fileManager->getAllMaintenance();
    for (auto& m : all) {
        if (m.getRecordId() == rId) {
            showMaintenanceFormDialog(&m);
            return;
        }
    }
}

void MaintenanceView::handleDelete() {
    int row = maintenanceTable->currentRow();
    if (row < 0) {
        QMessageBox::information(this, "Select Record", "Please select a maintenance record to delete.");
        return;
    }

    QString rId = maintenanceTable->item(row, 0)->text();
    auto reply = QMessageBox::question(this, "Confirm Deletion",
        QString("Are you sure you want to delete maintenance record '%1'?").arg(rId),
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        try {
            fileManager->deleteMaintenanceRecord(rId.toStdString());
            QMessageBox::information(this, "Success", "Record deleted successfully.");
            refreshData();
        } catch (const std::exception& e) {
            QMessageBox::critical(this, "Error", QString("Failed to delete record: %1").arg(e.what()));
        }
    }
}

void MaintenanceView::handleToggleStatus() {
    int row = maintenanceTable->currentRow();
    std::string bId = "";
    if (row >= 0) {
        bId = maintenanceTable->item(row, 1)->text().toStdString();
    } else if (busFilterCombo->currentText() != "All Buses") {
        bId = busFilterCombo->currentText().toStdString();
    }

    if (bId.empty()) {
        QMessageBox::information(this, "Select Bus", "Please select a record from the table or filter by a specific bus to toggle status.");
        return;
    }

    Bus b = fileManager->getBusById(bId);
    if (b.getBusId().empty()) return;

    QString newStatus = (b.getStatus() == "Active") ? "Maintenance" : "Active";
    auto reply = QMessageBox::question(this, "Update Bus Status",
        QString("Change status of Bus %1 from '%2' to '%3'?").arg(QString::fromStdString(bId), QString::fromStdString(b.getStatus()), newStatus),
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        b.setStatus(newStatus.toStdString());
        fileManager->updateBus(b);
        QMessageBox::information(this, "Status Updated", QString("Bus %1 status is now set to '%2'.").arg(QString::fromStdString(bId), newStatus));
        refreshData();
    }
}

void MaintenanceView::showMaintenanceFormDialog(MaintenanceRecord* existing) {
    bool isEditing = (existing != nullptr);

    QDialog dlg(this);
    dlg.setWindowTitle(isEditing ? "Edit Maintenance Record" : "Add Maintenance Record");
    dlg.setFixedSize(480, 440);

    auto* layout = new QVBoxLayout(&dlg);
    layout->setSpacing(14);
    layout->setContentsMargins(20, 20, 20, 20);

    auto* formLayout = new QFormLayout();
    formLayout->setSpacing(10);

    auto* idEdit = new QLineEdit(&dlg);
    idEdit->setPlaceholderText("e.g. MNT004");
    if (isEditing) {
        idEdit->setText(QString::fromStdString(existing->getRecordId()));
        idEdit->setEnabled(false);
    } else {
        idEdit->setText(QString("MNT%1").arg(rand() % 900 + 100));
    }

    auto* busCombo = new QComboBox(&dlg);
    auto buses = fileManager->getAllBuses();
    for (const auto& b : buses) {
        busCombo->addItem(QString::fromStdString(b.getBusId()));
    }
    if (isEditing) busCombo->setCurrentText(QString::fromStdString(existing->getBusId()));

    auto* serviceCombo = new QComboBox(&dlg);
    serviceCombo->setEditable(true);
    serviceCombo->addItems({
        "Routine General Servicing",
        "Oil Change & Filter Replacement",
        "Brake Pads & Disc Overhaul",
        "Tire Replacement & Wheel Alignment",
        "Engine Diagnostics & Repair",
        "Battery & Electrical Inspection",
        "AC & Cooling System Maintenance"
    });
    if (isEditing) serviceCombo->setCurrentText(QString::fromStdString(existing->getServiceType()));

    auto* costSpin = new QDoubleSpinBox(&dlg);
    costSpin->setRange(100.0, 500000.0);
    costSpin->setPrefix("₹ ");
    costSpin->setValue(isEditing ? existing->getCost() : 5000.0);

    auto* dateEdit = new QDateEdit(QDate::currentDate(), &dlg);
    dateEdit->setCalendarPopup(true);
    dateEdit->setDisplayFormat("yyyy-MM-dd");
    if (isEditing && !existing->getServiceDate().empty()) {
        dateEdit->setDate(QDate::fromString(QString::fromStdString(existing->getServiceDate()), "yyyy-MM-dd"));
    }

    auto* workshopEdit = new QLineEdit(&dlg);
    workshopEdit->setPlaceholderText("Workshop or Garage Name");
    workshopEdit->setText(isEditing ? QString::fromStdString(existing->getWorkshop()) : "Authorized College Workshop");

    auto* remarksEdit = new QLineEdit(&dlg);
    remarksEdit->setPlaceholderText("Service details and technician remarks");
    if (isEditing) remarksEdit->setText(QString::fromStdString(existing->getRemarks()));

    formLayout->addRow("Record ID *:", idEdit);
    formLayout->addRow("Bus ID *:", busCombo);
    formLayout->addRow("Service Type *:", serviceCombo);
    formLayout->addRow("Cost (INR) *:", costSpin);
    formLayout->addRow("Service Date:", dateEdit);
    formLayout->addRow("Workshop:", workshopEdit);
    formLayout->addRow("Remarks:", remarksEdit);

    layout->addLayout(formLayout);

    auto* btnLayout = new QHBoxLayout();
    auto* saveBtn = new QPushButton(isEditing ? "Save Changes" : "Save Record", &dlg);
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
        if (id.isEmpty()) {
            QMessageBox::warning(&dlg, "Validation Error", "Record ID cannot be empty.");
            return;
        }

        MaintenanceRecord m(id.toStdString(),
                              busCombo->currentText().toStdString(),
                              serviceCombo->currentText().trimmed().toStdString(),
                              costSpin->value(),
                              dateEdit->date().toString("yyyy-MM-dd").toStdString(),
                              workshopEdit->text().trimmed().toStdString(),
                              remarksEdit->text().trimmed().toStdString());

        try {
            if (isEditing) {
                fileManager->updateMaintenanceRecord(m);
                QMessageBox::information(&dlg, "Success", "Record updated successfully!");
            } else {
                fileManager->addMaintenanceRecord(m);
                QMessageBox::information(&dlg, "Success", "Record added successfully!");
            }
            dlg.accept();
        } catch (const std::exception& e) {
            QMessageBox::critical(&dlg, "Error", QString::fromStdString(e.what()));
        }
    });

    if (dlg.exec() == QDialog::Accepted) {
        refreshData();
    }
}
