/*
 * 程序进入点。
 * 创建于 2022年9月26日
 * 
 * 职责：解析命令行参数，启动子程序。
 */

#include <iostream>
#include <fstream>

#include <tc/main/LexerServer/LexerServer.h>
#include <tc/core/Dfa.h>

using namespace std;

int readCh(istream& in, int& r, int& c) {
    int ch = in.get();
    if (ch == '\n') {
        c = 1;
        r++;
    } else if (ch == '\r') {
        // nothing
    } else {
        c++;
    }

    return ch;
}

int main(int argc, const char* argv[]) {

    ifstream fin("resources/c-dfa.tcdf");
    ofstream fout("./testout.txt");
    if (!fout.is_open()) {
        cout << "failed to open fout." << endl;
        return -1;
    }
    Dfa dfa(fin);

    int row = 1;
    int col = 1;

    while (true) {

        if (cin.peek() == EOF) {
            break;
        }

        auto cinPos1 = cin.tellg();
        fout << "<" << row << ", " << col << "> ";
        const DfaStateNode* node = dfa.recognize(cin);

        auto cinPos2 = cin.tellg();

        cout << "id: " << node->stateInfo.id << endl;
        cout << "peek: " << cin.peek() << endl;
        cout << "cin pos1: " << cinPos1 << endl;
        cout << "cin pos2: " << cinPos2 << endl;
        // cout << "cin good: " << cin.good() << endl;
        // cout << "cin fail: " << cin.fail() << endl;

        if (node) {
            
            cin.seekg(cinPos1);
            cout << "|";
            
            fout << "|";
            for (int i = 0; i < cinPos2 - cinPos1; i++) {
                int ch = readCh(cin, row, col);
                if (ch < 32 || ch > 126) {
                    ch = '@';
                }
                cout << char(ch);
                fout << char(ch);
            }

            fout << "|";
            fout << endl;
            cout << "|\n";

            if (!node->stateInfo.isFinal) {
                cout << "not final!" << endl;
            }
        } else {
            cout << "nullptr." << endl;
            break;
        }

        int ch;
        while ((ch = cin.peek()) == '\n' || ch == ' ' || ch == '\r') {
            cout << "ignore ch: " << ch << endl;
            readCh(cin, row, col);
            
        }

        cout << endl;
    }

    return 0;
}
