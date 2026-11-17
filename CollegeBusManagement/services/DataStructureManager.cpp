#include "DataStructureManager.h"
#include "FileManager.h"
#include "../models/Student.h"
#include "../models/Bus.h"
#include "../models/Route.h"

#include <iostream>
#include <sstream>

DataStructureManager* DataStructureManager::instance = nullptr;

DataStructureManager::DataStructureManager() {
    busFleetArray = c_darray_create(16);
    currentRouteList = c_llist_create();
    studentWaitingQueue = c_queue_create(30);
    adminUndoStack = c_stack_create(50);
    studentBST = c_bst_create();
    studentHashTable = c_htable_create(47);
    routeNetworkGraph = c_graph_create(64, false);
}

DataStructureManager::~DataStructureManager() {
    c_darray_destroy(busFleetArray);
    c_llist_destroy(currentRouteList);
    c_queue_destroy(studentWaitingQueue);
    c_stack_destroy(adminUndoStack);
    c_bst_destroy(studentBST);
    c_htable_destroy(studentHashTable);
    c_graph_destroy(routeNetworkGraph);
}

DataStructureManager* DataStructureManager::getInstance() {
    if (!instance) {
        instance = new DataStructureManager();
    }
    return instance;
}

void DataStructureManager::cleanupInstance() {
    delete instance;
    instance = nullptr;
}

void DataStructureManager::reloadAllFromFiles(FileManager* fm) {
    if (!fm) return;

    // 1. Ingest Buses into Dynamic Array
    c_darray_destroy(busFleetArray);
    busFleetArray = c_darray_create(16);
    auto buses = fm->getAllBuses();
    for (const auto& b : buses) {
        c_darray_append(busFleetArray, b.getBusId().c_str());
    }

    // 2. Ingest Students into BST and Hash Table
    c_bst_destroy(studentBST);
    studentBST = c_bst_create();

    c_htable_destroy(studentHashTable);
    studentHashTable = c_htable_create(47);

    auto students = fm->getAllStudents();
    for (const auto& s : students) {
        c_bst_insert(studentBST, s.getId().c_str(), s.getName().c_str(),
                     s.getDepartment().c_str(), s.getSemester(),
                     s.getBusNumber().c_str(), s.getSeatNumber());

        c_htable_insert(studentHashTable, s.getId().c_str(), s.getName().c_str(),
                        s.getDepartment().c_str(), s.getBusNumber().c_str(),
                        s.getSeatNumber(), s.getPhone().c_str());
    }

    // 3. Build Route Graph from routes.txt
    auto routes = fm->getAllRoutes();
    buildNetworkFromRoutes(routes);

    if (!routes.empty()) {
        loadRouteStops(routes.front().getStopsAsString());
    }
}

// ============================================================
// 1. DYNAMIC ARRAY
// ============================================================
void DataStructureManager::registerBus(const std::string& busId) {
    if (c_darray_find(busFleetArray, busId.c_str()) < 0) {
        c_darray_append(busFleetArray, busId.c_str());
    }
}

void DataStructureManager::unregisterBus(const std::string& busId) {
    c_darray_remove_value(busFleetArray, busId.c_str());
}

std::vector<std::string> DataStructureManager::getAllBusesFromDArray() const {
    std::vector<std::string> res;
    size_t sz = c_darray_size(busFleetArray);
    res.reserve(sz);
    for (size_t i = 0; i < sz; ++i) {
        const char* val = c_darray_get(busFleetArray, i);
        if (val) res.push_back(val);
    }
    return res;
}

// ============================================================
// 2. SINGLY LINKED LIST
// ============================================================
void DataStructureManager::loadRouteStops(const std::string& stopsSemicolonSeparated) {
    c_llist_destroy(currentRouteList);
    currentRouteList = c_llist_create();

    std::stringstream ss(stopsSemicolonSeparated);
    std::string token;
    int seq = 1;
    while (std::getline(ss, token, ';')) {
        if (!token.empty()) {
            c_llist_insert_back(currentRouteList, token.c_str(), seq++);
        }
    }
}

std::vector<std::string> DataStructureManager::getRouteStopsAsVector() const {
    std::vector<std::string> res;
    if (!currentRouteList) return res;

    CListNode* curr = currentRouteList->head;
    while (curr) {
        if (curr->stop_name) res.push_back(curr->stop_name);
        curr = curr->next;
    }
    return res;
}

void DataStructureManager::addRouteStopBack(const std::string& stopName) {
    int seq = (int)c_llist_size(currentRouteList) + 1;
    c_llist_insert_back(currentRouteList, stopName.c_str(), seq);
}

// ============================================================
// 3. CIRCULAR QUEUE
// ============================================================
bool DataStructureManager::enqueueWaitingStudent(const std::string& studentId, const std::string& studentName, const std::string& busId) {
    DS_Status st = c_queue_enqueue(studentWaitingQueue, studentId.c_str(), studentName.c_str(), busId.c_str(), "Just now");
    return (st == DS_SUCCESS);
}

bool DataStructureManager::dequeueWaitingStudent(std::string& outStudentId, std::string& outStudentName, std::string& outBusId) {
    CWaitingStudent ws;
    DS_Status st = c_queue_dequeue(studentWaitingQueue, &ws);
    if (st == DS_SUCCESS) {
        outStudentId = ws.student_id ? ws.student_id : "";
        outStudentName = ws.student_name ? ws.student_name : "";
        outBusId = ws.bus_id ? ws.bus_id : "";
        free(ws.student_id);
        free(ws.student_name);
        free(ws.bus_id);
        free(ws.request_time);
        return true;
    }
    return false;
}

bool DataStructureManager::peekWaitingStudent(std::string& outStudentId, std::string& outStudentName, std::string& outBusId) const {
    CWaitingStudent ws;
    DS_Status st = c_queue_peek(studentWaitingQueue, &ws);
    if (st == DS_SUCCESS) {
        outStudentId = ws.student_id ? ws.student_id : "";
        outStudentName = ws.student_name ? ws.student_name : "";
        outBusId = ws.bus_id ? ws.bus_id : "";
        return true;
    }
    return false;
}

bool DataStructureManager::isStudentInWaitingQueue(const std::string& studentId) const {
    return c_queue_contains_student(studentWaitingQueue, studentId.c_str());
}

int DataStructureManager::getWaitingQueueSize() const {
    return c_queue_size(studentWaitingQueue);
}

// ============================================================
// 4. STACK
// ============================================================
void DataStructureManager::pushUndoAction(CUndoActionType type, const std::string& targetId, const std::string& name, const std::string& prevState) {
    c_stack_push(adminUndoStack, type, targetId.c_str(), name.c_str(), prevState.c_str());
}

bool DataStructureManager::popUndoAction(CUndoAction& outAction) {
    return (c_stack_pop(adminUndoStack, &outAction) == DS_SUCCESS);
}

bool DataStructureManager::hasUndoActions() const {
    return !c_stack_is_empty(adminUndoStack);
}

size_t DataStructureManager::getUndoStackDepth() const {
    return c_stack_size(adminUndoStack);
}

// ============================================================
// 5. BINARY SEARCH TREE
// ============================================================
void DataStructureManager::insertStudentBST(const Student& s) {
    c_bst_insert(studentBST, s.getId().c_str(), s.getName().c_str(),
                 s.getDepartment().c_str(), s.getSemester(),
                 s.getBusNumber().c_str(), s.getSeatNumber());
}

void DataStructureManager::deleteStudentBST(const std::string& studentId) {
    c_bst_delete(studentBST, studentId.c_str());
}

bool DataStructureManager::searchStudentBST(const std::string& studentId, std::string& outName, std::string& outDept) const {
    const CBSTNode* node = c_bst_search(studentBST, studentId.c_str());
    if (node) {
        outName = node->student_name ? node->student_name : "";
        outDept = node->department ? node->department : "";
        return true;
    }
    return false;
}

static void bstCollectVisitor(const CBSTNode* node, void* user_data) {
    auto* vec = reinterpret_cast<std::vector<std::string>*>(user_data);
    if (vec && node && node->student_id) {
        std::string record = std::string(node->student_id) + " - " +
                             (node->student_name ? node->student_name : "") + " (" +
                             (node->department ? node->department : "") + ", Bus: " +
                             (node->bus_number ? node->bus_number : "None") + ")";
        vec->push_back(record);
    }
}

std::vector<std::string> DataStructureManager::getStudentsInorderBST() const {
    std::vector<std::string> list;
    c_bst_inorder(studentBST, bstCollectVisitor, &list);
    return list;
}

// ============================================================
// 6. HASH TABLE
// ============================================================
void DataStructureManager::insertStudentHash(const Student& s) {
    c_htable_insert(studentHashTable, s.getId().c_str(), s.getName().c_str(),
                    s.getDepartment().c_str(), s.getBusNumber().c_str(),
                    s.getSeatNumber(), s.getPhone().c_str());
}

void DataStructureManager::deleteStudentHash(const std::string& studentId) {
    c_htable_delete(studentHashTable, studentId.c_str());
}

bool DataStructureManager::lookupStudentHash(const std::string& studentId, std::string& outName, std::string& outDept, std::string& outBus, int& outSeat) const {
    const CHashEntry* entry = c_htable_get(studentHashTable, studentId.c_str());
    if (entry) {
        outName = entry->name ? entry->name : "";
        outDept = entry->department ? entry->department : "";
        outBus = entry->bus_number ? entry->bus_number : "";
        outSeat = entry->seat_number;
        return true;
    }
    return false;
}

// ============================================================
// 7. GRAPH
// ============================================================
void DataStructureManager::buildNetworkFromRoutes(const std::vector<Route>& routes) {
    c_graph_destroy(routeNetworkGraph);
    routeNetworkGraph = c_graph_create(256, false); // Undirected transit network

    auto trim = [](const std::string& s) -> std::string {
        size_t first = s.find_first_not_of(" \t\r\n");
        if (first == std::string::npos) return "";
        size_t last = s.find_last_not_of(" \t\r\n");
        return s.substr(first, last - first + 1);
    };

    for (const auto& r : routes) {
        std::vector<std::string> path;

        std::string startP = trim(r.getStartPoint());
        if (startP == "College") startP = "College Campus";
        if (!startP.empty()) path.push_back(startP);

        // Add intermediate stops
        for (const auto& stop : r.getStops()) {
            std::string st = trim(stop);
            if (st == "College") st = "College Campus";
            if (!st.empty()) {
                if (path.empty() || path.back() != st) {
                    path.push_back(st);
                }
            }
        }

        std::string endP = trim(r.getEndPoint());
        if (endP == "College") endP = "College Campus";
        if (!endP.empty()) {
            if (path.empty() || path.back() != endP) {
                path.push_back(endP);
            }
        }

        // Add vertices and edges between consecutive stops in route path
        for (size_t i = 0; i < path.size(); ++i) {
            c_graph_add_vertex(routeNetworkGraph, path[i].c_str());
            if (i > 0) {
                c_graph_add_edge(routeNetworkGraph, path[i - 1].c_str(), path[i].c_str(), 4, r.getRouteId().c_str());
            }
        }
    }
}

bool DataStructureManager::checkStopsReachable(const std::string& startStop, const std::string& targetStop) const {
    return c_graph_is_reachable(routeNetworkGraph, startStop.c_str(), targetStop.c_str());
}

static void bfsCollector(const char* stop_name, int level, void* user_data) {
    auto* vec = reinterpret_cast<std::vector<std::pair<std::string, int>>*>(user_data);
    if (vec && stop_name) {
        vec->push_back({std::string(stop_name), level});
    }
}

std::vector<std::pair<std::string, int>> DataStructureManager::getBfsTraversal(const std::string& startStop) const {
    std::vector<std::pair<std::string, int>> res;
    c_graph_bfs(routeNetworkGraph, startStop.c_str(), bfsCollector, &res);
    return res;
}
