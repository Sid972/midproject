cmake_minimum_required(VERSION 3.30)
project(exchange_project LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Tell CMake where Qt lives
list(APPEND CMAKE_PREFIX_PATH "D:/Qt/6.9.0/mingw_64")

# Turn on moc / uic / rcc automatically
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
# (or simply: qt_standard_project_setup())

# Find Qt 6
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/20200317.csv 20200317.csv COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/20200601.csv 20200601.csv COPYONLY)
# Your executable
add_executable(exchange_project
        main.cpp
        MerkelMain.cpp
        OrderBook.cpp
        CSVReader.cpp
        TextPlotter.cpp
        Candlestick.cpp
        OrderBookEntry.cpp
        Wallet.cpp
        CurrencySelector.cpp
        CurrencySelector.h
)

target_link_libraries(exchange_project
        PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets
)