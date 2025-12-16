#pragma once

#include <string>
#include <string_view>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <filesystem>
#include <optional>

#include <Windows.h>


#define CHECKER_ERROR_FAILED 1
#define CHECKER_ERROR_BUSY 2
#define CHECKER_ERROR_BANNED 3
#define CHECKER_ERROR_UNKNOWN 4

#define WAL_TYPE_BTC 0
#define WAL_TYPE_EVM 1


#define BTC_WAL_DATABASE_NAME "btc_database.txt"
#define EVM_WAL_DATABASE_NAME "evm_database.txt"

struct wallet {
    std::string address;
    std::string address_legacy;
    std::string address_p2sh;
    std::string public_key;
    std::string private_key;
    std::string mnemonic;
};

struct wallet_data {
    std::string address;
    int type = WAL_TYPE_BTC;
    double btc_amount = 0.0;
    double eth_amount = 0.0;
    double bnb_amount = 0.0;
    double matic_amount = 0.0;
};


static class wallets_data_base {
public:
    static bool btc_database_exists();
    static bool load_btc_database();
    static std::string get_btc_balance(const std::string& address);

    static bool evm_database_exists();
    static bool load_evm_database();
    static std::string get_evm_balance(const std::string& address);

private:

    static std::optional<std::filesystem::path> find_database_path(const std::string& database_name);
    static std::unordered_map<std::string, std::string> btc_wallets_databases[2];
    static std::unordered_map<std::string, std::string> eth_wallets_databases[2];

};


class wallet_checker {
public:
    bool initialize(int way = 0);

    wallet_data get_wallet_data(const wallet& wal, int type);

private:
    int way;
};
