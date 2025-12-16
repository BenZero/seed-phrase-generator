#include "address_checker.h"

#include <algorithm>
#include <cctype>
#include <sstream>

extern std::string walletgen_path;

std::unordered_map<std::string, std::string> wallets_data_base::btc_wallets_databases[2];
std::unordered_map<std::string, std::string> wallets_data_base::eth_wallets_databases[2];

namespace {
    std::string to_lower_copy(const std::string& value) {
        std::string copy = value;
        std::transform(copy.begin(), copy.end(), copy.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
        return copy;
    }

    double safe_to_double(const std::string& value) {
        try {
            return std::stod(value);
        }
        catch (...) {
            return 0.0;
        }
    }

    bool load_simple_database(const std::filesystem::path& file_path,
                              std::unordered_map<std::string, std::string>& destination) {
        std::ifstream file(file_path);
        if (!file.is_open()) {
            return false;
        }

        std::string line;
        while (std::getline(file, line)) {
            if (line.empty() || line[0] == '#') {
                continue;
            }

            std::istringstream line_stream(line);
            std::string address;
            std::string balance = "0";

            if (!(line_stream >> address)) {
                continue;
            }

            if (!(line_stream >> balance)) {
                balance = "0";
            }

            destination[to_lower_copy(address)] = balance;
        }

        return true;
    }
}

std::optional<std::filesystem::path> wallets_data_base::find_database_path(const std::string& database_name) {
    const std::filesystem::path local_path{ database_name };
    if (std::filesystem::exists(local_path)) {
        return local_path;
    }

    if (!walletgen_path.empty()) {
        const std::filesystem::path sibling = std::filesystem::path(walletgen_path) / database_name;
        if (std::filesystem::exists(sibling)) {
            return sibling;
        }
    }

    return std::nullopt;
}

bool wallets_data_base::btc_database_exists() {
    return static_cast<bool>(find_database_path(BTC_WAL_DATABASE_NAME));
}

bool wallets_data_base::load_btc_database() {
    const auto db_path = find_database_path(BTC_WAL_DATABASE_NAME);
    if (!db_path) {
        return false;
    }

    btc_wallets_databases[0].clear();
    btc_wallets_databases[1].clear();

    return load_simple_database(*db_path, btc_wallets_databases[0]);
}

std::string wallets_data_base::get_btc_balance(const std::string& address) {
    const auto normalized = to_lower_copy(address);
    auto it = btc_wallets_databases[0].find(normalized);
    if (it != btc_wallets_databases[0].end()) {
        return it->second;
    }

    it = btc_wallets_databases[1].find(normalized);
    if (it != btc_wallets_databases[1].end()) {
        return it->second;
    }

    return "0";
}

bool wallets_data_base::evm_database_exists() {
    return static_cast<bool>(find_database_path(EVM_WAL_DATABASE_NAME));
}

bool wallets_data_base::load_evm_database() {
    const auto db_path = find_database_path(EVM_WAL_DATABASE_NAME);
    if (!db_path) {
        return false;
    }

    eth_wallets_databases[0].clear();
    eth_wallets_databases[1].clear();

    return load_simple_database(*db_path, eth_wallets_databases[0]);
}

std::string wallets_data_base::get_evm_balance(const std::string& address) {
    const auto normalized = to_lower_copy(address);
    auto it = eth_wallets_databases[0].find(normalized);
    if (it != eth_wallets_databases[0].end()) {
        return it->second;
    }

    it = eth_wallets_databases[1].find(normalized);
    if (it != eth_wallets_databases[1].end()) {
        return it->second;
    }

    return "0";
}

bool wallet_checker::initialize(int way_param) {
    way = way_param;
    return true;
}

wallet_data wallet_checker::get_wallet_data(const wallet& wal, int type) {
    wallet_data data{};
    data.type = type;

    if (type == WAL_TYPE_BTC) {
        std::string balance = wallets_data_base::get_btc_balance(wal.address);
        if (balance == "0" && !wal.address_legacy.empty()) {
            balance = wallets_data_base::get_btc_balance(wal.address_legacy);
        }
        if (balance == "0" && !wal.address_p2sh.empty()) {
            balance = wallets_data_base::get_btc_balance(wal.address_p2sh);
        }

        data.address = wal.address;
        data.btc_amount = safe_to_double(balance);
    }
    else {
        const std::string balance = wallets_data_base::get_evm_balance(wal.address);
        data.address = wal.address;
        data.eth_amount = safe_to_double(balance);
    }

    return data;
}
