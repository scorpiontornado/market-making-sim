#include <algorithm>
#include <chrono>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <sys/types.h>
#include <vector>

// ========================
// Basic Data Structures
// ========================

enum Side { BUY, SELL };
struct Trader {
    std::string name;
    int position = 0; // current position
    int pnl = 0;      // profit/loss
};
std::ostream &operator<<(std::ostream &os, const Trader &t) {
    return os << t.name << ": position=" << t.position << ", pnl=" << t.pnl;
}

// TODO: move back to OrderBook & use unique/shared_ptr (concurrency issues)
std::vector<Trader> traders;
std::map<std::string, int> traderNameToId;

struct Order {
    // std::unique_ptr<Trader> trader; // TODO: figure out how to use
    int traderId; // XXX: potential concurrency issues
    Side side;
    int quantity;
    int price;
    std::chrono::time_point<std::chrono::system_clock> timestamp;
};
std::ostream &operator<<(std::ostream &os, const Order &o) {
    // TODO: trader name rather than ID in order?
    return os << traders[o.traderId].name << " " << o.quantity << " @ "
              << o.price;
}

// comparator for buyOrders (highest price first; FIFO / price-time-priority)
// TODO: replace with operator<=> perhaps? (What to do if side differs?)
struct BuyOrderCompare {
    bool operator()(const Order &a, const Order &b) const {
        if (a.price == b.price) return a.timestamp < b.timestamp;
        return a.price > b.price; // max-heap
    }
};
// comparator for sellOrders (lowest price first; FIFO / price-time-priority)
struct SellOrderCompare {
    bool operator()(const Order &a, const Order &b) const {
        if (a.price == b.price) return a.timestamp < b.timestamp;
        return a.price < b.price; // min-heap
    }
};

struct Transaction {
    int buyerId; // XXX: potential concurrency issues?
    int sellerId;
    // aggressor?
    int quantity;
    int price;
    int timestamp;
};

// ========================
// Helper Functions
// ========================

// Mainly just for practice: A capitalise function
void capitalise(std::string &line) {
    // TODO: remove casts?
    for (char &c : line) {
        c = std::tolower(static_cast<unsigned char>(c));
    }
    if (line.size()) {
        line[0] = std::toupper(static_cast<unsigned char>(line[0]));
    }
}
void upper(std::string &line) {
    // for (char &c : line) {
    //     c = std::toupper(static_cast<unsigned char>(c));
    // }

    std::transform(line.begin(), line.end(), line.begin(), ::toupper);
}

// ========================
// Exchange / OrderBook
// ========================

class OrderBook {
  private:
    std::multiset<Order, BuyOrderCompare> buyOrders;
    std::multiset<Order, SellOrderCompare> sellOrders;
    // std::map<std::string, Trader> traders;

  public:
    void addOrder(const Order &order) {
        if (order.side == BUY) {
            buyOrders.insert(order);
        } else if (order.side == SELL) {
            sellOrders.insert(order);
        }
    }

    void matchOrders() {
        while (buyOrders.size() && sellOrders.size() &&
               buyOrders.begin()->price >= sellOrders.begin()->price) {
            auto buy = buyOrders.extract(buyOrders.begin());
            auto sell = sellOrders.extract(sellOrders.begin());

            int qty = std::min(buy.value().quantity, sell.value().quantity);
            int price = buy.value().timestamp < sell.value().timestamp
                            ? buy.value().price
                            : sell.value().price;

            // Update positions & pnl
            int buyTrader = buy.value().traderId;
            int sellTrader = sell.value().traderId;

            traders[buyTrader].position += qty;
            traders[sellTrader].position -= qty;

            traders[buyTrader].pnl -= price * qty;
            traders[sellTrader].pnl += price * qty;

            // Update order quantities & insert if positive
            if ((buy.value().quantity -= qty) > 0) {
                buyOrders.insert(std::move(buy));
            }

            if ((sell.value().quantity -= qty) > 0) {
                sellOrders.insert(std::move(sell));
            }

            // TODO: add transaction record
        }
    }

    void printStatus() {
        std::cout << "Order Book:\n";
        std::cout << "  Bids:\n";
        for (const auto &bid : buyOrders) {
            std::cout << "    " << bid << "\n";
        }

        std::cout << "  Asks:\n";
        for (const auto &ask : sellOrders) {
            std::cout << "    " << ask << "\n";
        }

        std::cout << "Traders:\n";
        for (const auto &t : traders) {
            std::cout << "    " << t << "\n";
        }
        auto topBuy = buyOrders.begin();
        auto topSell = sellOrders.begin();
        if (topBuy == buyOrders.end() || topSell == sellOrders.end()) {
            return;
        }
        if (topBuy->price >= topSell->price) {
            std::cout << "A sale should go through!\n";
        }
    }

    int registerTrader(const std::string &name) {
        // (Kept for pedagogical purposes)
        // auto sameName =
        //     std::find_if(traders.begin(), traders.end(),
        //                     [&](Trader t) { return t.name == name; });

        if (!traderNameToId.contains(name)) {
            traders.emplace_back(Trader{name});
            traderNameToId[name] = traders.size() - 1;
        }

        return traderNameToId[name];
    }

    Order parseOrder(const std::string &line) {
        std::stringstream ss(line);
        std::string sideStr, name;
        Side side;
        int qty, price;

        if (!(ss >> sideStr >> name >> qty >> price)) {
            throw std ::invalid_argument(
                "Invalid format: expected SIDE NAME QTY PRICE");
        }
        upper(sideStr);
        capitalise(name);

        // Check for extra tokens
        std::string extra;
        if (ss >> extra) {
            throw std::invalid_argument("Invalid format: extra input detected");
        }
        // Validate side
        if (sideStr == "ASK" || sideStr == "SELL") {
            side = SELL;
        } else if (sideStr == "BID" || sideStr == "BUY") {
            side = BUY;
        } else {
            throw std::invalid_argument("Side must be BUY, BID, SELL, or ASK");
        }

        // Validate quantity and price
        if (qty <= 0) {
            throw std::invalid_argument("Quantity must be positive");
        }
        if (price <= 0) {
            throw std::invalid_argument("Price must be positive");
        }

        int traderId = registerTrader(name);
        auto now = std::chrono::system_clock::now();

        return Order{traderId, side, qty, price, now};
    }
};

// ========================
// Main Loop
// ========================

int main() {
    OrderBook ob;

    std::cout
        << "Welcome to Mini Exchange!\n"
        << "Please place trades in the format SIDE TRADER QTY PRICE (e.g. "
           "SELL TIM 10 300).\n"
        << "Type EXIT to quit.\n";

    std::string line;
    while (true) {
        std::cout << "> ";
        std::getline(std::cin, line); // getline
        if (line == "EXIT") break;

        try {
            Order o = ob.parseOrder(line); // TODO: abstract out parseOrder?
            ob.addOrder(o);
            ob.matchOrders();
            ob.printStatus();
        } catch (const std::invalid_argument &e) {
            std::cout << e.what() << "\n";
        } catch (const std::domain_error &e) {
            std::cout << e.what() << "\n";
        }
    }

    std::cout << "Goodbye!\n";
    return 0;
}
