/*

    LR Parser Action Goto Table.
    Part of the ToyCompile Project.

    created on 2022.11.2

*/

#include <tc/core/LrParserTable.h>

using namespace std;
using namespace tc;

LrParserCommand LrParserTable::getCommand(int stateId, int symbolId) {
    if (table.count(stateId)) {
        auto& row = table[stateId];
        if (row.count(symbolId)) {
            return row[symbolId];
        }
    }

    // error.
    LrParserCommand err;
    err.type = LrParserCommandType::ERROR;
    return err;
}
