//
// Created by g on 7/20/21.
//

#include "EApp_t.h"
#include <sgx_trts.h>
#include <vector>
#include <list>
#include <iostream>
#include <cassert>
#include <algorithm>

using namespace std;

list<unsigned long long> ledger = {};

//void debug_print_ledger() {
//    cout << "==========Ledger==========" << endl;
//    auto uid = 1;
//    for (unsigned long long x : ledger) {
//        cout << "account " << uid << " has balance " << x << endl;
//        ++uid;
//    }
//    cout << "==========================" << endl;
//}

long ecall_create_user(unsigned long long bal) {
    ledger.push_back(bal);
    return ledger.size();
}

bool ecall_xfer(long from_uid, long to_uid, unsigned long long sum) {
    auto from_idx = from_uid - 1;
    auto to_idx = to_uid - 1;
    if (ledger.size() < from_idx || ledger.size() < to_idx)
        return false;
    auto from_iter = ledger.begin();
    advance(from_iter, from_idx);
    auto from_bal = *from_iter;
    if (from_bal < sum)
        return false;
    auto updated_from_bal = from_bal - sum;
    auto to_iter = ledger.begin();
    advance(to_iter, to_idx);
    *from_iter = updated_from_bal;
    *to_iter += sum;
    return true;
}

unsigned long long ecall_get_balance(long id) {
    if (ledger.size() < id)
        return 0;
    auto iter = ledger.begin();
    advance(iter, id - 1);
    auto bal = *iter;
    return bal;
}

int main() {
    auto user1 = ecall_create_user(50000);
    auto user2 = ecall_create_user(50);
//    debug_print_ledger();
    assert (ecall_get_balance(user1) == 50000);
    assert (ecall_get_balance(user2) == 50);
    assert (ecall_xfer(user1, user2, 50));
//    debug_print_ledger();
    assert (ecall_get_balance(user1) == (50000 - 50));
    assert (ecall_get_balance(user2) == (50 + 50));
    assert (!ecall_xfer(user2, user1, 100000));
    assert (!ecall_xfer(user1, user2, 100000));
    assert (!ecall_xfer(user2 + 1, user1 + 4, 100000));
    assert (!ecall_xfer(user1 + 3, user2 + 5, 100000));
//    debug_print_ledger();
    assert (ecall_get_balance(user1) == (50000 - 50));
    assert (ecall_get_balance(user2) == (50 + 50));

}
