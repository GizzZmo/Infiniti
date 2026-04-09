#include "bitboard.h"
#include "uci.h"

int main() {
    init_bitboards();
    UCI uci;
    uci.loop();
    return 0;
}
