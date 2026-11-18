cmake_minimum_required(VERSION 3.16)

project(CollegeBusManagement
    VERSION 1.0
    DESCRIPTION "College Bus Management System - Pure C Data Structures & C++ OOPS PBL Project"
    LANGUAGES C CXX
)

# Enforce C++17 and C11 Standards
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Enable Qt Meta-Object Compiler, Resource Compiler, and User Interface Compiler
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find Qt 6 Packages (Widgets, Core, Gui)
find_package(Qt6 COMPONENTS Widgets Core Gui REQUIRED)

# Project Sources
set(PROJECT_SOURCES
    main.cpp

    # Models (Demonstrating OOPS: Abstraction, Inheritance, Encapsulation, Polymorphism)
    models/Person.h
    models/Student.h
    models/Student.cpp
    models/Driver.h
    models/Driver.cpp
    models/Bus.h
    models/Bus.cpp
    models/Route.h
    models/Route.cpp
    models/Seat.h
    models/Seat.cpp
    models/Attendance.h
    models/Attendance.cpp
    models/FeeRecord.h
    models/FeeRecord.cpp
    models/MaintenanceRecord.h
    models/MaintenanceRecord.cpp
    models/User.h

    # Services (File Handling, Exception Handling, Business Logic)
    services/Exceptions.h
    services/FileManager.h
    services/FileManager.cpp
    services/DataStructureManager.h
    services/DataStructureManager.cpp

    # Pure C Data Structures (C11, Structs, Pointers, Manual Memory Management)
    ds_c/c_common.h
    ds_c/c_dynamic_array.h
    ds_c/c_dynamic_array.c
    ds_c/c_linked_list.h
    ds_c/c_linked_list.c
    ds_c/c_circular_queue.h
    ds_c/c_circular_queue.c
    ds_c/c_stack.h
    ds_c/c_stack.c
    ds_c/c_bst.h
    ds_c/c_bst.c
    ds_c/c_hash_table.h
    ds_c/c_hash_table.c
    ds_c/c_graph.h
    ds_c/c_graph.c

    # Views (Modern Qt 6 GUI Widgets)
    views/LoginDialog.h
    views/LoginDialog.cpp
    views/MainWindow.h
    views/MainWindow.cpp
    views/AdminDashboardView.h
    views/AdminDashboardView.cpp
    views/StudentManagementView.h
    views/StudentManagementView.cpp
    views/BusManagementView.h
    views/BusManagementView.cpp
    views/RouteManagementView.h
    views/RouteManagementView.cpp
    views/DriverManagementView.h
    views/DriverManagementView.cpp
    views/SeatAllocationView.h
    views/SeatAllocationView.cpp
    views/AttendanceView.h
    views/AttendanceView.cpp
    views/FeeManagementView.h
    views/FeeManagementView.cpp
    views/MaintenanceView.h
    views/MaintenanceView.cpp
    views/ReportsView.h
    views/ReportsView.cpp
    views/StudentPortalView.h
    views/StudentPortalView.cpp
    views/DriverPortalView.h
    views/DriverPortalView.cpp
    views/ConductorPortalView.h
    views/ConductorPortalView.cpp
    views/BusMapWidget.h
    views/BusMapWidget.cpp

    # Embedded Qt Resources
    resources/resources.qrc
)

# Executable Target
add_executable(CollegeBusManagement ${PROJECT_SOURCES})

# Include Directories
target_include_directories(CollegeBusManagement PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/models
    ${CMAKE_CURRENT_SOURCE_DIR}/services
    ${CMAKE_CURRENT_SOURCE_DIR}/ds_c
    ${CMAKE_CURRENT_SOURCE_DIR}/views
)

# Link Libraries
target_link_libraries(CollegeBusManagement PRIVATE
    Qt6::Widgets
    Qt6::Core
    Qt6::Gui
)

# Copy 'data' and 'resources' directory to binary output directory for direct execution
add_custom_command(TARGET CollegeBusManagement POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/data
        $<TARGET_FILE_DIR:CollegeBusManagement>/data
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/resources
        $<TARGET_FILE_DIR:CollegeBusManagement>/resources
    COMMENT "Copying data files and stylesheets to build folder..."
)
