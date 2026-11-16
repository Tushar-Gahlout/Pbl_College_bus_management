#include "RouteManagementView.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QDialog>
#include <QFormLayout>
#include <QComboBox>
#include <QTextEdit>
#include <QLabel>

RouteManagementView::RouteManagementView(FileManager* fm, QWidget* parent)
    : QWidget(parent), fileManager(fm) {
    setupUI();
    refreshData();
}

void RouteManagementView::setupUI() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 24, 24, 24);
    mainLayout->setSpacing(16);

    // Top Header & Action Buttons
    auto* topLayout = new QHBoxLayout();
    auto* title = new QLabel("Route Management", this);
    title->setStyleSheet("font-size: 20px; font-weight: bold; color: #0f172a;");
    topLayout->addWidget(title);
    topLayout->addStretch();

    addBtn = new QPushButton("➕ Add New Route", this);
    addBtn->setProperty("class", "primaryButton");
    addBtn->setCursor(Qt::PointingHandCursor);

    editBtn = new QPushButton("✏️ Edit Route", this);
    editBtn->setProperty("class", "secondaryButton");
    editBtn->setCursor(Qt::PointingHandCursor);

    deleteBtn = new QPushButton("🗑️ Delete Route", this);
    deleteBtn->setProperty("class", "dangerButton");
    deleteBtn->setCursor(Qt::PointingHandCursor);

    refreshBtn = new QPushButton("🔄 Refresh", this);
    refreshBtn->setProperty("class", "secondaryButton");
    refreshBtn->setCursor(Qt::PointingHandCursor);

    topLayout->addWidget(addBtn);
    topLayout->addWidget(editBtn);
    topLayout->addWidget(deleteBtn);
    topLayout->addWidget(refreshBtn);
    mainLayout->addLayout(topLayout);

    // Search bar
    searchEdit = new QLineEdit(this);
    searchEdit->setPlaceholderText("🔍 Search routes by ID, Name, Start, End, or Stops...");
    searchEdit->setClearButtonEnabled(true);
    mainLayout->addWidget(searchEdit);

    // Routes Table
    routeTable = new QTableWidget(this);
    routeTable->setColumnCount(8);
    routeTable->setHorizontalHeaderLabels({
        "Route ID", "Route Name", "Start Point", "End Point", "Stops", "Pickup Time", "Drop Time", "Assigned Bus"
    });
    routeTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    routeTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    routeTable->setSelectionMode(QAbstractItemView::SingleSelection);
    routeTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    routeTable->verticalHeader()->setVisible(false);
    mainLayout->addWidget(routeTable);

    // Signals
    connect(searchEdit, &QLineEdit::textChanged, this, &RouteManagementView::handleSearch);
    connect(addBtn, &QPushButton::clicked, this, &RouteManagementView::handleAdd);
    connect(editBtn, &QPushButton::clicked, this, &RouteManagementView::handleEdit);
    connect(deleteBtn, &QPushButton::clicked, this, &RouteManagementView::handleDelete);
    connect(refreshBtn, &QPushButton::clicked, this, &RouteManagementView::refreshData);
    connect(routeTable, &QTableWidget::doubleClicked, this, &RouteManagementView::handleEdit);
}

void RouteManagementView::refreshData() {
    handleSearch();
}

void RouteManagementView::handleSearch() {
    std::string query = searchEdit->text().trimmed().toStdString();
    std::vector<Route> list = fileManager->searchRoutes(query);

    routeTable->setRowCount(0);
    for (size_t i = 0; i < list.size(); ++i) {
        const auto& r = list[i];
        routeTable->insertRow(i);

        routeTable->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(r.getRouteId())));
        routeTable->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(r.getRouteName())));
        routeTable->setItem(i, 2, new QTableWidgetItem(QString::fromStdString(r.getStartPoint())));
        routeTable->setItem(i, 3, new QTableWidgetItem(QString::fromStdString(r.getEndPoint())));
        routeTable->setItem(i, 4, new QTableWidgetItem(QString::fromStdString(r.getStopsAsString())));
        routeTable->setItem(i, 5, new QTableWidgetItem(QString::fromStdString(r.getPickupTime())));
        routeTable->setItem(i, 6, new QTableWidgetItem(QString::fromStdString(r.getDropTime())));
        routeTable->setItem(i, 7, new QTableWidgetItem(QString::fromStdString(r.getAssignedBus())));
    }
}

void RouteManagementView::handleAdd() {
    showRouteFormDialog(nullptr);
}

void RouteManagementView::handleEdit() {
    int row = routeTable->currentRow();
    if (row < 0) {
        QMessageBox::information(this, "Select Route", "Please select a route from the table to edit.");
        return;
    }

    std::string rId = routeTable->item(row, 0)->text().toStdString();
    Route r = fileManager->getRouteById(rId);
    if (!r.getRouteId().empty()) {
        showRouteFormDialog(&r);
    }
}

void RouteManagementView::handleDelete() {
    int row = routeTable->currentRow();
    if (row < 0) {
        QMessageBox::information(this, "Select Route", "Please select a route to delete.");
        return;
    }

    QString rId = routeTable->item(row, 0)->text();
    auto reply = QMessageBox::question(this, "Confirm Deletion",
        QString("Are you sure you want to delete route '%1'?").arg(rId),
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        try {
            fileManager->deleteRoute(rId.toStdString());
            QMessageBox::information(this, "Success", "Route deleted successfully.");
            refreshData();
        } catch (const std::exception& e) {
            QMessageBox::critical(this, "Error", QString("Failed to delete route: %1").arg(e.what()));
        }
    }
}

void RouteManagementView::showRouteFormDialog(Route* existing) {
    bool isEditing = (existing != nullptr);

    QDialog dlg(this);
    dlg.setWindowTitle(isEditing ? "Edit Route" : "Add New Route");
    dlg.setFixedSize(520, 520);

    auto* layout = new QVBoxLayout(&dlg);
    layout->setSpacing(14);
    layout->setContentsMargins(20, 20, 20, 20);

    auto* formLayout = new QFormLayout();
    formLayout->setSpacing(10);

    auto* idEdit = new QLineEdit(&dlg);
    idEdit->setPlaceholderText("e.g. R04");
    if (isEditing) {
        idEdit->setText(QString::fromStdString(existing->getRouteId()));
        idEdit->setEnabled(false);
    }

    auto* nameEdit = new QLineEdit(&dlg);
    nameEdit->setPlaceholderText("e.g. West City Corridor");
    if (isEditing) nameEdit->setText(QString::fromStdString(existing->getRouteName()));

    auto* startEdit = new QLineEdit(&dlg);
    startEdit->setPlaceholderText("Origin point (e.g. West Terminal)");
    if (isEditing) startEdit->setText(QString::fromStdString(existing->getStartPoint()));

    auto* endEdit = new QLineEdit(&dlg);
    endEdit->setPlaceholderText("Destination point (e.g. College Campus)");
    if (isEditing) endEdit->setText(QString::fromStdString(existing->getEndPoint()));

    auto* stopsEdit = new QTextEdit(&dlg);
    stopsEdit->setPlaceholderText("Enter stops separated by semicolons (;) or commas (,)\ne.g. Stop 1; Stop 2; Stop 3");
    stopsEdit->setMaximumHeight(80);
    if (isEditing) {
        stopsEdit->setPlainText(QString::fromStdString(existing->getStopsAsString()));
    }

    auto* pickTimeEdit = new QLineEdit(&dlg);
    pickTimeEdit->setPlaceholderText("e.g. 07:30 AM");
    pickTimeEdit->setText(isEditing ? QString::fromStdString(existing->getPickupTime()) : "07:30 AM");

    auto* dropTimeEdit = new QLineEdit(&dlg);
    dropTimeEdit->setPlaceholderText("e.g. 05:00 PM");
    dropTimeEdit->setText(isEditing ? QString::fromStdString(existing->getDropTime()) : "05:00 PM");

    auto* busCombo = new QComboBox(&dlg);
    busCombo->addItem("None");
    auto buses = fileManager->getAllBuses();
    for (const auto& b : buses) {
        busCombo->addItem(QString::fromStdString(b.getBusId()));
    }
    if (isEditing) busCombo->setCurrentText(QString::fromStdString(existing->getAssignedBus()));

    formLayout->addRow("Route ID *:", idEdit);
    formLayout->addRow("Route Name *:", nameEdit);
    formLayout->addRow("Start Point *:", startEdit);
    formLayout->addRow("End Point *:", endEdit);
    formLayout->addRow("Route Stops:", stopsEdit);
    formLayout->addRow("Morning Pickup Time:", pickTimeEdit);
    formLayout->addRow("Evening Drop Time:", dropTimeEdit);
    formLayout->addRow("Assigned Bus:", busCombo);

    layout->addLayout(formLayout);

    auto* btnLayout = new QHBoxLayout();
    auto* saveBtn = new QPushButton(isEditing ? "Save Changes" : "Add Route", &dlg);
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
        QString name = nameEdit->text().trimmed();
        QString start = startEdit->text().trimmed();
        QString end = endEdit->text().trimmed();

        if (id.isEmpty() || name.isEmpty() || start.isEmpty() || end.isEmpty()) {
            QMessageBox::warning(&dlg, "Validation Error", "Please fill in all mandatory fields (ID, Name, Start, End).");
            return;
        }

        Route r;
        r.setRouteId(id.toStdString());
        r.setRouteName(name.toStdString());
        r.setStartPoint(start.toStdString());
        r.setEndPoint(end.toStdString());
        r.setStopsFromString(stopsEdit->toPlainText().toStdString());
        r.setPickupTime(pickTimeEdit->text().trimmed().toStdString());
        r.setDropTime(dropTimeEdit->text().trimmed().toStdString());
        r.setAssignedBus(busCombo->currentText().toStdString());

        try {
            if (isEditing) {
                fileManager->updateRoute(r);
                QMessageBox::information(&dlg, "Success", "Route updated successfully!");
            } else {
                fileManager->addRoute(r);
                QMessageBox::information(&dlg, "Success", "Route added successfully!");
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
