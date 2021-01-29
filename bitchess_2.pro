TEMPLATE = app
CONFIG += console c++17
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    tests/chess_board_tests.cpp \
    utils/global_utils.cpp \
    core/chess_castle.cpp \
    core/chess_board.cpp \
    tests/attack_tables_test.cpp \
    core/move_generator.cpp \
    core/chess_move.cpp \
    tests/move_gen_test.cpp \
    utils/chess_enums.cpp \
    core/zobr_hash.cpp \
    autoplay/board_wrapper.cpp \
    mcts/mcts_search.cpp \
    mcts/mcts_rollout.cpp \
    mcts/mcts_node.cpp \
    net/data_encoder.cpp \
    net/board_tensor.cpp \
    core/chess_attack_tables.cpp \
    autoplay/auto_mcts_search_thread.cpp

HEADERS +=\
    core/chess_castle.h \
    core/chess_board.h \
    core/move_generator.h \
    core/chess_move.h \
    core/zobr_hash.h \
    autoplay/board_wrapper.h \
    mcts/mcts_search.h \
    mcts/mcts_rollout.h \
    mcts/mcts_node.h \
    net/data_encoder.h \
    net/board_tensor.h \
    core/chess_attack_tables.h \
    autoplay/auto_mcts_search_thread.h

LIBS += -pthread

DISTFILES += \
    mcts/asdf.txt \
    nn/asdf.txt \
    tests/asdf.txt \
    utils/asdf.txt \
    wrappers/asdf.txt \
    autoplay/asdf.txt \
    autoplay/asdf.txt \
    mcts/asdf.txt \
    net/asdf.txt
