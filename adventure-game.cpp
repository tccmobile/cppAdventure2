#include <iostream>
#include <string>
#include <vector>
#include <limits>
#include <memory>
#include <algorithm>
using namespace std;

class Enemy {
public:
    std::string name;
    int health;
    int damage;

    Enemy(const std::string& n, int h, int d) 
        : name(n), health(h), damage(d) {}

    bool isAlive() const { return health > 0; }
};

class Location {
public:
    std::string name;
    std::string description;
    std::vector<std::shared_ptr<Location> > connections;
    std::vector<std::string> items;
    bool isEndLocation;
    std::shared_ptr<Enemy> enemy;

    Location(const std::string& n, const std::string& desc, bool isEnd = false)
        : name(n), description(desc), isEndLocation(isEnd), enemy(nullptr) {}

    void addConnection(std::shared_ptr<Location> location) {
        connections.push_back(location);
    }

    void addItem(const std::string& item) {
        items.push_back(item);
    }

    void addEnemy(const std::string& name, int health, int damage) {
        enemy = std::make_shared<Enemy>(name, health, damage);
    }
};

class Player {
public:
    std::vector<std::string> inventory;
    int health = 100;
    int damage = 20;

    void addItem(const std::string& item) {
        inventory.push_back(item);
        std::cout << "You picked up: " << item << std::endl;
    }

    bool hasItem(const std::string& item) const {
        return find(inventory.begin(), inventory.end(), item) != inventory.end();
    }

    void showInventory() const {
        std::cout << "\nInventory:" << std::endl;
        if (inventory.empty()) {
            std::cout << "Empty" << std::endl;
            return;
        }
        for (const auto& item : inventory) {
            std::cout << "- " << item << std::endl;
        }
    }

    bool isAlive() const { return health > 0; }

    bool hasWeapon() const {
        return hasItem("Rusty Sword");
    }

    int getDamage() const {
        return hasWeapon() ? damage : 0;  // No damage without a weapon
    }
};

class Game {
private:
    std::shared_ptr<Location> currentLocation;
    Player player;

    void clearScreen() {
        #ifdef _WIN32
            system("cls");
        #else
            system("clear");
        #endif
    }

    void displayLocation() {
        clearScreen();
        std::cout << "\n=== " << currentLocation->name << " ===" << std::endl;
        std::cout << currentLocation->description << std::endl;
        
        if (!currentLocation->items.empty()) {
            std::cout << "\nYou see:" << std::endl;
            for (const auto& item : currentLocation->items) {
                std::cout << "- " << item << std::endl;
            }
        }

        std::cout << "\nPossible exits:" << std::endl;
        for (size_t i = 0; i < currentLocation->connections.size(); ++i) {
            std::cout << i + 1 << ". Go to " << currentLocation->connections[i]->name << std::endl;
        }
    }

    void handleItems() {
        if (!currentLocation->items.empty()) {
            std::cout << "\nWould you like to pick up any items? (y/n): ";
            char choice;
            std::cin >> choice;
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

            if (choice == 'y' || choice == 'Y') {
                for (auto it = currentLocation->items.begin(); it != currentLocation->items.end();) {
                    player.addItem(*it);
                    it = currentLocation->items.erase(it);
                }
            }
        }
    }

    void handleCombat() {
        if (!currentLocation->enemy || !currentLocation->enemy->isAlive()) {
            return;
        }

        auto& enemy = currentLocation->enemy;
        std::cout << "\nA " << enemy->name << " appears!" << std::endl;
        
        if (!player.hasWeapon()) {
            std::cout << "You have no weapon to defend yourself!" << std::endl;
            std::cout << "The " << enemy->name << " attacks you, and you're defenseless." << std::endl;
            player.health = 0;
            std::cout << "\nGame Over! You were defeated by the " << enemy->name << "." << std::endl;
            exit(0);
        }

        while (enemy->isAlive() && player.isAlive()) {
            std::cout << "\nYour health: " << player.health 
                      << " | " << enemy->name << "'s health: " << enemy->health << std::endl;
            
            std::cout << "\nWhat would you like to do?" << std::endl;
            std::cout << "1. Attack" << std::endl;
            std::cout << "2. Run away" << std::endl;
            
            std::string choice;
            std::cout << "Enter your choice: ";
            std::getline(std::cin, choice);

            if (choice == "2") {
                std::cout << "You run away from the " << enemy->name << "!" << std::endl;
                return;
            }
            
            if (choice == "1") {
                // Player attacks - now using getDamage() instead of damage
                enemy->health -= player.getDamage();
                std::cout << "You hit the " << enemy->name << " for " << player.getDamage() << " damage!" << std::endl;
                
                // Enemy attacks back if still alive
                if (enemy->isAlive()) {
                    player.health -= enemy->damage;
                    std::cout << "The " << enemy->name << " hits you for " << enemy->damage << " damage!" << std::endl;
                }
            }
            
            if (!player.isAlive()) {
                std::cout << "\nGame Over! You were defeated by the " << enemy->name << "." << std::endl;
                exit(0);
            }
            
            if (!enemy->isAlive()) {
                std::cout << "\nYou defeated the " << enemy->name << "!" << std::endl;
                std::cout << "Press Enter to continue...";
                std::cin.get();
            }
        }
    }

    // Recursive function to explore locations
    void exploreLocation() {
        if (currentLocation->isEndLocation) {
            displayLocation();
            std::cout << "\nCongratulations! You've reached the end of your adventure!" << std::endl;
            return;
        }

        while (true) {
            displayLocation();
            handleCombat();
            handleItems();
            
            std::cout << "\nWhat would you like to do?" << std::endl;
            std::cout << "1-" << currentLocation->connections.size() << ". Move to a new location" << std::endl;
            std::cout << "i. Check inventory" << std::endl;
            std::cout << "q. Quit game" << std::endl;
            
            std::string choice;
            std::cout << "\nEnter your choice: ";
            std::getline(std::cin, choice);

            if (choice == "q" || choice == "Q") {
                std::cout << "Thanks for playing!" << std::endl;
                return;
            }
            
            if (choice == "i" || choice == "I") {
                player.showInventory();
                std::cout << "\nPress Enter to continue...";
                std::cin.get();
                continue;
            }

            try {
                std::vector<std::shared_ptr<Location>>::size_type index = std::stoi(choice) - 1;
                if (index >= 0 && index < static_cast<int>(currentLocation->connections.size())) {
                    currentLocation = currentLocation->connections[index];
                    exploreLocation(); // Recursive call
                    return;
                }
            } catch (...) {
                // Invalid input, will show menu again
            }
            
            std::cout << "Invalid choice. Press Enter to continue...";
            std::cin.get();
        }
    }

public:
    Game(std::shared_ptr<Location> startLocation) : currentLocation(startLocation) {}

    void start() {
        std::cout << "Welcome to the Adventure Game!" << std::endl;
        std::cout << "Press Enter to begin...";
        std::cin.get();
        exploreLocation();
    }
};

int main() {
    // Create locations
    auto cave = std::make_shared<Location>("Cave", 
        "You're in a dimly lit cave. Water drips from the ceiling.");
    auto forest = std::make_shared<Location>("Forest", 
        "You're in a dense forest. Sunlight filters through the leaves.");
    auto ruins = std::make_shared<Location>("Ancient Ruins", 
        "You stand before crumbling stone walls covered in mysterious symbols.");
    auto temple = std::make_shared<Location>("Temple", 
        "You've reached a magnificent temple atop a mountain.", true);

    // Add connections
    cave->addConnection(forest);
    forest->addConnection(cave);
    forest->addConnection(ruins);
    ruins->addConnection(forest);
    ruins->addConnection(temple);

    // Add items
    cave->addItem("Rusty Sword");
    forest->addItem("Magic Stone");
    ruins->addItem("Ancient Key");

    // Add enemies to locations
    forest->addEnemy("Wild Wolf", 50, 10);
    ruins->addEnemy("Ancient Guardian", 80, 15);

    // Create and start game
    Game game(cave);
    game.start();

    return 0;
}
