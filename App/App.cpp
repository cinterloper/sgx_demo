//
// Created by g on 7/20/21.
//
#include <cstdio>
#include <cstring>
#include <iostream>
#include <sgx_urts.h>
#include <time.h>
#include <chrono>
#include "EApp_u.h"
#include "error_print.h"
#include <stdbool.h>
#include <cassert>

using namespace std;
using actbal = unsigned long long;
sgx_enclave_id_t enclave_id = 0;


int init_enclave() {
    string launch_token_path = "enclave.token";
    string enclave_name = "enclave.signed.so";
    const char *token_path = launch_token_path.c_str();

    sgx_launch_token_t token{0};
    sgx_status_t status = SGX_ERROR_UNEXPECTED;
    int updated = 0;

    /*
     * obtain enclave launch token
     */
    /* if it exists, load it */
    FILE *fp = fopen(token_path, "rb");

    /* if it does not exist, create it */
    if (fp == NULL && (fp = fopen(token_path, "wb")) == NULL) {
        cerr << "could not create / open path: " << token_path << endl;
    }
    if (fp != NULL) {
        //read it
        size_t read_num = fread(token, 1, sizeof(sgx_launch_token_t), fp);

        //if invalid clear buffer
        if (read_num != 0 && read_num != sizeof(sgx_launch_token_t)) {
            memset(&token, 0x0, sizeof(sgx_launch_token_t));
            cerr << "warning: invalid launch token read: " << launch_token_path << endl;
        }

    }
    //init enclave
    status = sgx_create_enclave(enclave_name.c_str(), SGX_DEBUG_FLAG, &token, &updated, &enclave_id, NULL);
    if (status != SGX_SUCCESS) {
        cerr << "sgx error: " << status << endl;
        if (fp != NULL) {
            fclose(fp);
        }
        return -1;
    }

    //save token if updated

    if (updated == 0 || fp == NULL) {
        if (fp != NULL) {
            fclose(fp);
        }
        return 0;
    }

    fp = freopen(token_path, "wb", fp);
    if (fp == NULL) return 0;

    size_t write_num = fwrite(token, 1, sizeof(sgx_launch_token_t), fp);
    if (write_num != sizeof(sgx_launch_token_t)) {
        cerr << "failed to save launch token: " << token_path << endl;
    }
    fclose(fp);
    return 0;


}

bool test() {
    long *user1 = new long;
    assert(ecall_create_user(enclave_id, user1, 50000) == SGX_SUCCESS);
    long *user2 = new long;
    assert(ecall_create_user(enclave_id, user2, 50) == SGX_SUCCESS);
////    debug_print_ledger();
    auto *user1_bal_1 = new actbal;
    assert (ecall_get_balance(enclave_id, user1_bal_1, *user1) == SGX_SUCCESS);
    assert (*user1_bal_1 == 50000);
    auto *user2_bal_1 = new actbal;
    assert (ecall_get_balance(enclave_id, user2_bal_1, *user2) == SGX_SUCCESS);
    assert (*user2_bal_1 == 50);
    auto *xfer_success = new bool;
    assert (ecall_xfer(enclave_id, xfer_success, *user1, *user2, 50) == SGX_SUCCESS);
    assert (*xfer_success);
////    debug_print_ledger();
    auto *user1_bal_2 = new actbal;
    assert (ecall_get_balance(enclave_id, user1_bal_2, *user1) == SGX_SUCCESS);
    assert (*user1_bal_2 == (50000 - 50));
    auto *user2_bal_2 = new actbal;
    assert (ecall_get_balance(enclave_id, user2_bal_2, *user2) == SGX_SUCCESS);
    assert (*user2_bal_2 == (50 + 50));
    auto *xfer_success_2 = new bool;
    assert (ecall_xfer(enclave_id, xfer_success_2, *user2, *user1, 100000) == SGX_SUCCESS);
    assert (*xfer_success_2 == false);
    auto *xfer_success_3 = new bool;
    assert (ecall_xfer(enclave_id, xfer_success_3, *user1, *user2, 100000) == SGX_SUCCESS);
    assert (*xfer_success_3 == false);

    assert (ecall_xfer(enclave_id, xfer_success_3, *user1 + 3, *user2 + 4, 100000) == SGX_SUCCESS);
    assert (*xfer_success_3 == false);
////    debug_print_ledger();
    auto *user1_bal_3 = new actbal;
    assert (ecall_get_balance(enclave_id, user1_bal_3, *user1) == SGX_SUCCESS);
    assert (*user1_bal_2 == (50000 - 50));
    auto *user2_bal_3 = new actbal;
    assert (ecall_get_balance(enclave_id, user2_bal_3, *user2) == SGX_SUCCESS);
    assert (*user2_bal_2 == (50 + 50));

    return true;
}

int main() {
    if (init_enclave() < 0) {
        cerr << "failed to init enclave" << endl;
        return -1;
    }

    auto res = test();
    if (res) {
        cout << "tests pass" << endl;
    } else {
        cout << "tests fail" << endl;
    }


    sgx_destroy_enclave(enclave_id);


}