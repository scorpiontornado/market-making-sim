#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <map>
#include <stdexcept>
#include <set>

using namespace std;

// ========================
// Basic Data Structures
// ========================

struct Order {
    string traderName;
    string side; // "BUY" or "SELL"
    int quantity;
    int price;
    int timestamp;
};
// comparator for sellOrders (lowest price first)
struct SellOrderCompare {
    bool operator()(const Order& a, const Order& b) const {
        if (a.price == b.price) return a.timestamp > b.timestamp;
        return a.price > b.price; // min-heap
    }
};
// comparator for buyOrders (highest price first)
struct BuyOrderCompare {
    bool operator()(const Order& a, const Order& b) const {
        if (a.price == b.price) return a.timestamp > b.timestamp;
        return a.price < b.price;
    }
};

struct Trader {
    string name;
    int position = 0; // current position
    int pnl = 0;      // profit/loss
};

// ========================
// Exchange / OrderBook
// ========================

class OrderBook {
private:
    multiset<Order, BuyOrderCompare> buyOrders;
    multiset<Order, SellOrderCompare> sellOrders;
    map<string, Trader> traders;

public:
    void addOrder(const Order& order) {
        if (order.side == "BUY") {
            buyOrders.insert(order);
        } else if (order.side == "SELL") {
            sellOrders.insert(order);
        } else {
            throw domain_error("order.side is not BUY or SELL.");
        }
    }

    void matchOrders() {
        // TODO: implement matching engine
        // update positions and pnl
    }

    void printStatus() {
        cout << "Order Book:\n";
        cout << "  Bids:\n";
        for (const auto& o : buyOrders)
            cout << "    " << o.traderName << " " << o.quantity << " @ " << o.price << "\n";

        cout << "  Asks:\n";
        for (const auto& o : sellOrders)
            cout << "    " << o.traderName << " " << o.quantity << " @ " << o.price << "\n";

        cout << "Traders:\n";
        for (const auto& [name, t] : traders) {
            cout << "  " << name << ": position=" << t.position << ", pnl=" << t.pnl << "\n";
        }
    }

    void registerTrader(const string& name) {
        if (traders.find(name) == traders.end()) {
            traders[name] = Trader{name};
        }
    }
};

// ========================
// Helper Functions
// ========================

// Mainly just for practice: A capitalise function
void capitalise(string& line) {
    for (char& c : line) {
        c = std::tolower(static_cast<unsigned char>(c));
    }
    line[0] = std::toupper(static_cast<unsigned char>(line[0]));
}
void upper(string& line) {
    for (char& c : line) {
        c = std::toupper(static_cast<unsigned char>(c));
    }
}

Order parseOrder(const string& line) {
    stringstream ss(line);
    string side, trader;
    int qty, price;

    if (!(ss >> side >> trader >> qty >> price)) {
        throw invalid_argument("Invalid format: expected SIDE TRADER QTY PRICE");
    }
    upper(side);
    capitalise(trader);

    // Check for extra tokens
    string extra;
    if (ss >> extra) {
        throw invalid_argument("Invalid format: extra input detected");
    }
    // Validate side
    if (side != "BUY" && side != "SELL") {
        throw invalid_argument("Side must be BUY or SELL");
    }
    // Validate quantity and price
    if (qty <= 0) {
        throw invalid_argument("Quantity must be positive");
    }
    if (price <= 0) {
        throw invalid_argument("Price must be positive");
    }

    return Order{trader, side, qty, price};
}

// ========================
// Main Loop
// ========================

int main() {
    OrderBook ob;

    cout << "Welcome to Mini Exchange!\n"
    << "Please place trades in the format SIDE TRADER QTY PRICE (e.g. SELL TIM 10 300).\n"
    << "Type EXIT to quit.\n";

    string line;
    while (true) {
        cout << "> ";
        getline(cin, line); // getline 
        if (line == "EXIT") break;

        try {
            Order o = parseOrder(line);
            ob.registerTrader(o.traderName);
            ob.addOrder(o);
            ob.matchOrders();
            ob.printStatus();
        } catch (const invalid_argument& e) {
            std::cout << e.what() << "\n";
        } catch (const domain_error& e) {
            std::cout << e.what() << "\n";
        }
    }

    cout << "Goodbye!\n";
    return 0;
}