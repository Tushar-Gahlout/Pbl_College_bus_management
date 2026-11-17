#ifndef DATASTRUCTUREMANAGER_H
#define DATASTRUCTUREMANAGER_H

#include <string>
#include <vector>
#include <memory>

#include "../ds_c/c_common.h"
#include "../ds_c/c_dynamic_array.h"
#include "../ds_c/c_linked_list.h"
#include "../ds_c/c_circular_queue.h"
#include "../ds_c/c_stack.h"
#include "../ds_c/c_bst.h"
#include "../ds_c/c_hash_table.h"
#include "../ds_c/c_graph.h"

// Forward declare model types
class Student;
class Bus;
class Route;
class Seat;
class FileManager;

/**
 * @brief C++ Singleton Bridge orchestrating the 7 Pure C Data Structures.
 * Demonstrates Abstraction, Encapsulation, RAII and Bridge Pattern.
 */
class DataStructureManager {
private:
    static DataStructureManager* instance;

    // Pointers to Pure C Data Structures
    CDynamicArray* busFleetArray;
    CSinglyLinkedList* currentRouteList;
    CCircularQueue* studentWaitingQueue;
    CStack* adminUndoStack;
    CBinarySearchTree* studentBST;
    CHashTable* studentHashTable;
    CGraph* routeNetworkGraph;

    DataStructureManager();
    ~DataStructureManager();

public:
    static DataStructureManager* getInstance();
    static void cleanupInstance();

    // Prevent copy and assignment (Singleton pattern)
    DataStructureManager(const DataStructureManager&) = delete;
    DataStructureManager& operator=(const DataStructureManager&) = delete;

    // Initialization & Synchronized Ingest from existing File storage
    void reloadAllFromFiles(FileManager* fileManager);

    // 1. Dynamic Array (Bus fleet ID tracking)
    void registerBus(const std::string& busId);
    void unregisterBus(const std::string& busId);
    std::vector<std::string> getAllBusesFromDArray() const;

    // 2. Singly Linked List (Active route stop sequences)
    void loadRouteStops(const std::string& stopsSemicolonSeparated);
    std::vector<std::string> getRouteStopsAsVector() const;
    void addRouteStopBack(const std::string& stopName);

    // 3. Circular Queue (Waiting list for full buses)
    bool enqueueWaitingStudent(const std::string& studentId, const std::string& studentName, const std::string& busId);
    bool dequeueWaitingStudent(std::string& outStudentId, std::string& outStudentName, std::string& outBusId);
    bool peekWaitingStudent(std::string& outStudentId, std::string& outStudentName, std::string& outBusId) const;
    bool isStudentInWaitingQueue(const std::string& studentId) const;
    int getWaitingQueueSize() const;

    // 4. Stack (Admin Undo)
    void pushUndoAction(CUndoActionType type, const std::string& targetId, const std::string& name, const std::string& prevState);
    bool popUndoAction(CUndoAction& outAction);
    bool hasUndoActions() const;
    size_t getUndoStackDepth() const;

    // 5. Binary Search Tree (Student Indexing & Sorted Traversal)
    void insertStudentBST(const Student& s);
    void deleteStudentBST(const std::string& studentId);
    bool searchStudentBST(const std::string& studentId, std::string& outName, std::string& outDept) const;
    std::vector<std::string> getStudentsInorderBST() const;

    // 6. Hash Table (O(1) Student ID Lookups)
    void insertStudentHash(const Student& s);
    void deleteStudentHash(const std::string& studentId);
    bool lookupStudentHash(const std::string& studentId, std::string& outName, std::string& outDept, std::string& outBus, int& outSeat) const;

    // 7. Graph (Bus Network Connectivity)
    void buildNetworkFromRoutes(const std::vector<Route>& routes);
    bool checkStopsReachable(const std::string& startStop, const std::string& targetStop) const;
    std::vector<std::pair<std::string, int>> getBfsTraversal(const std::string& startStop) const;
};

#endif // DATASTRUCTUREMANAGER_H
