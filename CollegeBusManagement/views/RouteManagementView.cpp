#include "RouteManagementView.h"
#include "../services/DataStructureManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QDialog>
#include <QFormLayout>
#include <QComboBox>
#include <QTextEdit>
#include <QLabel>
#include <QSet>
#include <QScrollArea>

RouteManagementView::RouteManagementView(FileManager* fm, QWidget* parent)
    : QWidget(parent), fileManager(fm), isReadOnly(false) {
    setupUI();
    refreshData();
}

void RouteManagementView::setupUI() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 24, 24, 24);
    mainLayout->setSpacing(16);

    // Top Header & Action Buttons (Row 1)
    auto* topLayout = new QHBoxLayout();
    titleLbl = new QLabel("Route Management", this);
    titleLbl->setStyleSheet("font-size: 20px; font-weight: bold; color: #0f172a;");
    topLayout->addWidget(titleLbl);
    topLayout->addStretch();

    addBtn = new QPushButton("➕ Add Route", this);
    addBtn->setProperty("class", "primaryButton");
    addBtn->setCursor(Qt::PointingHandCursor);

    editBtn = new QPushButton("✏️ Edit", this);
    editBtn->setProperty("class", "secondaryButton");
    editBtn->setCursor(Qt::PointingHandCursor);

    deleteBtn = new QPushButton("🗑️ Delete", this);
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

    // Search bar & Pure C DS Tools (Row 2)
    auto* searchToolLayout = new QHBoxLayout();
    searchEdit = new QLineEdit(this);
    searchEdit->setPlaceholderText("🔍 Search routes by ID, Name, Start, End, or Stops...");
    searchEdit->setClearButtonEnabled(true);
    searchToolLayout->addWidget(searchEdit, 2);

    stopsListBtn = new QPushButton("🔗 Stops (Linked List)", this);
    stopsListBtn->setProperty("class", "secondaryButton");
    stopsListBtn->setCursor(Qt::PointingHandCursor);
    stopsListBtn->setToolTip("View Route Stop sequence as Pure C Singly Linked List nodes");

    networkGraphBtn = new QPushButton("🗺️ Reachability (Graph BFS)", this);
    networkGraphBtn->setProperty("class", "secondaryButton");
    networkGraphBtn->setCursor(Qt::PointingHandCursor);
    networkGraphBtn->setToolTip("Test Bus Stop Connectivity using Pure C Graph BFS Traversal");

    searchToolLayout->addWidget(stopsListBtn);
    searchToolLayout->addWidget(networkGraphBtn);
    mainLayout->addLayout(searchToolLayout);

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
    connect(stopsListBtn, &QPushButton::clicked, this, &RouteManagementView::handleViewStopsList);
    connect(networkGraphBtn, &QPushButton::clicked, this, &RouteManagementView::handleCheckConnectivity);
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

void RouteManagementView::setReadOnly(bool readOnly) {
    isReadOnly = readOnly;
    addBtn->setVisible(!readOnly);
    editBtn->setVisible(!readOnly);
    deleteBtn->setVisible(!readOnly);

    if (readOnly) {
        titleLbl->setText("Bus Routes & Transit Schedule");
    } else {
        titleLbl->setText("Route Management");
    }
}

void RouteManagementView::handleAdd() {
    if (isReadOnly) return;
    showRouteFormDialog(nullptr);
}

void RouteManagementView::handleEdit() {
    if (isReadOnly) return;
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
    if (isReadOnly) return;
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

void RouteManagementView::handleViewStopsList() {
    int row = routeTable->currentRow();
    if (row < 0) {
        if (routeTable->rowCount() > 0) {
            row = 0; // default to first route
        } else {
            QMessageBox::information(this, "No Routes", "No routes available to inspect.");
            return;
        }
    }

    std::string rId = routeTable->item(row, 0)->text().toStdString();
    Route r = fileManager->getRouteById(rId);

    auto* dsm = DataStructureManager::getInstance();
    dsm->loadRouteStops(r.getStopsAsString());
    auto stops = dsm->getRouteStopsAsVector();

    QDialog dlg(this);
    dlg.setWindowTitle(QString("Pure C Singly Linked List - Route %1 (%2)").arg(QString::fromStdString(rId), QString::fromStdString(r.getRouteName())));
    dlg.resize(620, 440);

    auto* layout = new QVBoxLayout(&dlg);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(12);

    auto* infoLbl = new QLabel(
        "<b>Data Structure Demonstration: Singly Linked List</b><br>"
        "Each bus stop is stored in a dynamically allocated node (<code>CSinglyNode</code>) containing:<br>"
        "• <code>char* stop_name</code><br>"
        "• <code>struct CSinglyNode* next</code><br>"
        "Traverses sequentially from <code>head</code> to <code>tail</code> ending at <code>NULL</code>.", &dlg);
    infoLbl->setStyleSheet("background-color: #eff6ff; border: 1px solid #bfdbfe; border-radius: 6px; padding: 10px; color: #1e40af; font-size: 13px;");
    layout->addWidget(infoLbl);

    auto* seqLbl = new QLabel(&dlg);
    seqLbl->setWordWrap(true);
    QString seqStr = "<b>Linked Sequence:</b><br>";
    for (size_t i = 0; i < stops.size(); ++i) {
        seqStr += QString("<span style='background-color:#e0f2fe; padding:2px 8px; border-radius:4px; font-weight:bold;'>[%1] %2</span> &rarr; ")
            .arg(i + 1).arg(QString::fromStdString(stops[i]));
    }
    seqStr += "<span style='color:#dc2626; font-weight:bold;'>NULL</span>";
    seqLbl->setText(seqStr);
    seqLbl->setStyleSheet("background-color: #f8fafc; border: 1px dashed #cbd5e1; border-radius: 6px; padding: 12px; font-size: 13px;");
    layout->addWidget(seqLbl);

    auto* table = new QTableWidget(&dlg);
    table->setColumnCount(3);
    table->setHorizontalHeaderLabels({"Node Index", "Stop Name", "Next Pointer"});
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->setRowCount(static_cast<int>(stops.size()));

    for (size_t i = 0; i < stops.size(); ++i) {
        table->setItem(i, 0, new QTableWidgetItem(QString("Node #%1").arg(i + 1)));
        table->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(stops[i])));
        QString nextStr = (i + 1 < stops.size()) ? QString("&rarr; %1").arg(QString::fromStdString(stops[i + 1])) : "NULL (Tail)";
        table->setItem(i, 2, new QTableWidgetItem(nextStr));
    }
    layout->addWidget(table);

    auto* closeBtn = new QPushButton("Close", &dlg);
    closeBtn->setProperty("class", "secondaryButton");
    connect(closeBtn, &QPushButton::clicked, &dlg, &QDialog::accept);
    layout->addWidget(closeBtn, 0, Qt::AlignRight);

    dlg.exec();
}

void RouteManagementView::handleCheckConnectivity() {
    auto routes = fileManager->getAllRoutes();
    auto* dsm = DataStructureManager::getInstance();
    dsm->buildNetworkFromRoutes(routes);

    QSet<QString> stopSet;
    for (const auto& r : routes) {
        QString sp = QString::fromStdString(r.getStartPoint()).trimmed();
        if (sp == "College") sp = "College Campus";
        if (!sp.isEmpty()) stopSet.insert(sp);

        QString ep = QString::fromStdString(r.getEndPoint()).trimmed();
        if (ep == "College") ep = "College Campus";
        if (!ep.isEmpty()) stopSet.insert(ep);

        for (const auto& s : r.getStops()) {
            QString st = QString::fromStdString(s).trimmed();
            if (st == "College") st = "College Campus";
            if (!st.isEmpty()) stopSet.insert(st);
        }
    }

    if (stopSet.isEmpty()) {
        QMessageBox::information(this, "No Stops", "No bus stops available in the route network.");
        return;
    }

    QDialog dlg(this);
    dlg.setWindowTitle("Pure C Graph - Bus Stop Connectivity & BFS Traversal");
    dlg.resize(620, 520);

    auto* layout = new QVBoxLayout(&dlg);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(12);

    auto* infoLbl = new QLabel(
        "<b>Data Structure Demonstration: Graph & Breadth-First Search (BFS)</b><br>"
        "Vertices represent bus stops; edges represent transit links between consecutive stops.<br>"
        "BFS explores neighbor vertices level-by-level in <b>O(V + E)</b> time complexity.", &dlg);
    infoLbl->setStyleSheet("background-color: #f0fdf4; border: 1px solid #bbf7d0; border-radius: 6px; padding: 10px; color: #166534; font-size: 13px;");
    layout->addWidget(infoLbl);

    auto* formLayout = new QFormLayout();
    auto* fromCombo = new QComboBox(&dlg);
    auto* toCombo = new QComboBox(&dlg);

    QStringList sortedStops = stopSet.values();
    sortedStops.sort();
    for (const auto& stop : sortedStops) {
        fromCombo->addItem(stop);
        toCombo->addItem(stop);
    }
    if (toCombo->count() > 1) toCombo->setCurrentIndex(1);

    formLayout->addRow("Origin Bus Stop:", fromCombo);
    formLayout->addRow("Destination Bus Stop:", toCombo);
    layout->addLayout(formLayout);

    auto* resultScroll = new QScrollArea(&dlg);
    resultScroll->setWidgetResizable(true);
    resultScroll->setStyleSheet("border: 1px dashed #cbd5e1; border-radius: 6px; background-color: #f8fafc;");

    auto* resultLbl = new QLabel("Select stops and click 'Run Graph BFS Reachability Check' to test connectivity.", resultScroll);
    resultLbl->setStyleSheet("padding: 12px; font-size: 13px; color: #334155;");
    resultLbl->setWordWrap(true);
    resultScroll->setWidget(resultLbl);
    layout->addWidget(resultScroll, 1);

    auto* btnLayout = new QHBoxLayout();
    auto* checkBtn = new QPushButton("🔍 Run Graph BFS Reachability Check", &dlg);
    checkBtn->setProperty("class", "primaryButton");
    auto* closeBtn = new QPushButton("Close", &dlg);
    closeBtn->setProperty("class", "secondaryButton");

    btnLayout->addWidget(checkBtn);
    btnLayout->addStretch();
    btnLayout->addWidget(closeBtn);
    layout->addLayout(btnLayout);

    connect(closeBtn, &QPushButton::clicked, &dlg, &QDialog::accept);
    connect(checkBtn, &QPushButton::clicked, [&]() {
        std::string startStop = fromCombo->currentText().trimmed().toStdString();
        std::string targetStop = toCombo->currentText().trimmed().toStdString();

        bool reachable = dsm->checkStopsReachable(startStop, targetStop);
        auto bfsHops = dsm->getBfsTraversal(startStop);

        QString statusHtml;
        if (reachable) {
            statusHtml = QString(
                "<span style='color:#16a34a; font-weight:bold; font-size:15px;'>✅ Route Reachable via Bus Network!</span><br>"
                "There exists a valid transit path connecting <b>%1</b> &rarr; <b>%2</b>.<br><br>"
                "<b>Pure C Breadth-First Search (BFS) Traversal from '%1':</b><br>"
            ).arg(QString::fromStdString(startStop), QString::fromStdString(targetStop));
        } else {
            statusHtml = QString(
                "<span style='color:#dc2626; font-weight:bold; font-size:15px;'>❌ Not Reachable</span><br>"
                "No connected path found between <b>%1</b> and <b>%2</b> in the current route network.<br><br>"
                "<b>Reachable Stops from '%1':</b><br>"
            ).arg(QString::fromStdString(startStop), QString::fromStdString(targetStop));
        }

        for (const auto& hop : bfsHops) {
            statusHtml += QString("&nbsp;&nbsp;• <b>%1</b> (Distance: %2 hops)<br>")
                .arg(QString::fromStdString(hop.first)).arg(hop.second);
        }

        resultLbl->setText(statusHtml);
    });

    dlg.exec();
}

