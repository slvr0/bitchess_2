TEMPLATE = app
CONFIG += console c++17
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    tests/chess_board_tests.cpp \
    utils/global_utils.cpp \
    core/chess_castle.cpp \
    core/chess_board.cpp \
    core/chess_attack_tables.cpp \
    tests/attack_tables_test.cpp \
    core/move_generator.cpp \
    core/chess_move.cpp \
    tests/move_gen_test.cpp

HEADERS +=\
    core/chess_castle.h \
    core/chess_board.h \
    core/chess_attack_tables.h \
    core/move_generator.h \
    core/chess_move.h

DISTFILES += \
    mcts/asdf.txt \
    nn/asdf.txt \
    tests/asdf.txt \
    utils/asdf.txt \
    wrappers/asdf.txt
