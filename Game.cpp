#include <iostream>
#include <vector>
#include <stdexcept>
#include <algorithm>

enum class SegmentState { Intact, Damaged, Destroyed };
enum class Orientation { Horizontal, Vertical };
enum class CellStatus { Unknown, Empty, Ship };

// Класс сегмента корабля
class ShipSegment {
public:
    enum class State { Intact, Damaged, Destroyed };

private:
    State state;

public:
    ShipSegment() : state(State::Intact) {}

    void takeDamage() {
        if (state == State::Intact)
            state = State::Damaged;
        else if (state == State::Damaged)
            state = State::Destroyed;
    }

    bool isDestroyed() const {
        return state == State::Destroyed;
    }
    
    std::string getState() const {
        switch (state) {
            case State::Intact: return "Intact";
            case State::Damaged: return "Damaged";
            case State::Destroyed: return "Destroyed";
            default: return "Unknown";
        }
    }
};

// Класс корабля
class Ship {
private:
    size_t length;
    Orientation orientation;
    std::vector<ShipSegment> segments;

public:
    Ship(size_t len, Orientation ori) : length(len), orientation(ori), segments(len) {}

    size_t getLength() const { return length; }

    Orientation getOrientation() const { return orientation; }

    void attackSegment(size_t index) {
        if (index >= length)
            throw std::out_of_range("Invalid segment index.");
        segments[index].takeDamage();
    }

    bool isDestroyed() const {
        for (const auto& segment : segments) {
            if (!segment.isDestroyed()) return false;
        }
        return true;
    }

    void printSegments() const {
        std::cout << "Segments: ";
        for (const auto& segment : segments) {
            std::cout << "[" << segment.getState() << "] ";
        }
        std::cout << std::endl;
    }

    ShipSegment& getSegment(size_t index) {
        if (index >= length)
            throw std::out_of_range("Invalid segment index.");
        return segments[index];
    }

    // Конструктор копирования
    Ship(const Ship& other) : length(other.length), orientation(other.orientation), segments(other.segments) {}

    // Конструктор перемещения
    Ship(Ship&& other) noexcept : length(other.length), orientation(other.orientation), segments(std::move(other.segments)) {
        other.length = 0;
        other.orientation = Orientation::Horizontal;
    }

    // Оператор присваивания копированием
    Ship& operator=(const Ship& other) {
        if (this != &other) {
            length = other.length;
            orientation = other.orientation;
            segments = other.segments;
        }
        return *this;
    }

    // Оператор присваивания перемещением
    Ship& operator=(Ship&& other) noexcept {
        if (this != &other) {
            length = other.length;
            orientation = other.orientation;
            segments = std::move(other.segments);

            other.length = 0;
            other.orientation = Orientation::Horizontal;
        }
        return *this;
    }
};

// Класс менеджера кораблей
class ShipManager {
private:
    std::vector<Ship> ships;

public:
    void addShip(const Ship& ship) {
        ships.push_back(ship);
    }

    void attackShip(size_t shipIndex, size_t segmentIndex) {
        if (shipIndex >= ships.size())
            throw std::out_of_range("Invalid ship index.");
        
        Ship& ship = ships[shipIndex];
        ship.attackSegment(segmentIndex);

        if (ship.isDestroyed()) {
            std::cout << "Ship " << shipIndex << " is destroyed!" << std::endl;
        }
    }

    std::vector<Ship>& getShips() { return ships; }  // Возвращаем неконстантную ссылку на корабли
    
    void printShips() const {
        for (size_t i = 0; i < ships.size(); ++i) {
            std::cout << "Ship " << i << ": ";
            ships[i].printSegments();
        }
    }
};

class GameField {
    size_t width, height;
    std::vector<std::vector<CellStatus>> field;
    ShipManager shipManager;

public:
    GameField(size_t w, size_t h) : width(w), height(h), field(h, std::vector<CellStatus>(w, CellStatus::Unknown)) {
        if (w < 1 || h < 1)
            throw std::invalid_argument("Field dimensions must be greater than 0.");
    }

    // Конструктор копирования
    GameField(const GameField& other) : width(other.width), height(other.height), field(other.field) {}

    // Конструктор перемещения
    GameField(GameField&& other) noexcept : width(other.width), height(other.height), field(std::move(other.field)) {
        // Перемещаем shipManager (он содержит все корабли)
        shipManager = std::move(other.shipManager);

        // После перемещения оставляем исходный объект в допустимом состоянии
        other.width = 0;
        other.height = 0;
    }

    // Оператор присваивания копированием
    GameField& operator=(const GameField& other) {
        if (this != &other) {
            width = other.width;
            height = other.height;
            field = other.field;
            shipManager = other.shipManager;  // Копирование shipManager
        }
        return *this;
    }

    // Оператор присваивания перемещением
    GameField& operator=(GameField&& other) noexcept {
        if (this != &other) {
            width = other.width;
            height = other.height;
            field = std::move(other.field);
            shipManager = std::move(other.shipManager);  // Перемещение shipManager

            other.width = 0;
            other.height = 0;
        }
        return *this;
    }

    bool placeShip(const Ship& ship, size_t x, size_t y) {
        size_t len = ship.getLength();
        Orientation ori = ship.getOrientation();

        // Validate placement
        if ((ori == Orientation::Horizontal && x + len > width) ||
            (ori == Orientation::Vertical && y + len > height)) {
            return false;
        }

        // Check for collisions
        for (size_t i = 0; i < len; ++i) {
            size_t tx = (ori == Orientation::Horizontal) ? x + i : x;
            size_t ty = (ori == Orientation::Vertical) ? y + i : y;

            if (field[ty][tx] != CellStatus::Unknown)
                return false;
        }

        // Place ship
        for (size_t i = 0; i < len; ++i) {
            size_t tx = (ori == Orientation::Horizontal) ? x + i : x;
            size_t ty = (ori == Orientation::Vertical) ? y + i : y;

            field[ty][tx] = CellStatus::Ship;
        }
        shipManager.addShip(ship);
        return true;
    }

    void attackCell(size_t x, size_t y) {
        if (x >= width || y >= height)
            throw std::out_of_range("Invalid attack coordinates.");

        // Находим сегмент корабля, если он есть в данной клетке
        for (size_t i = 0; i < shipManager.getShips().size(); ++i) {
            Ship& ship = shipManager.getShips()[i];
            size_t len = ship.getLength();
            Orientation ori = ship.getOrientation();

            for (size_t j = 0; j < len; ++j) {
                size_t tx = (ori == Orientation::Horizontal) ? x + j : x;
                size_t ty = (ori == Orientation::Vertical) ? y + j : y;

                if (field[ty][tx] == CellStatus::Ship) {
                    // Если попадание в сегмент, атакуем этот сегмент
                    ship.attackSegment(j);

                    // Обновляем поле: убираем корабельную клетку
                    field[ty][tx] = CellStatus::Empty;

                    std::cout << "Hit on ship " << i << ", segment " << j << std::endl;
                    return;
                }
            }
        }

        std::cout << "Miss!" << std::endl;
    }

    void printField() const {
        for (const auto& row : field) {
            for (CellStatus cell : row) {
                switch (cell) {
                case CellStatus::Unknown: std::cout << "? "; break;
                case CellStatus::Empty: std::cout << ". "; break;
                case CellStatus::Ship: std::cout << "S "; break;
                }
            }
            std::cout << std::endl;
        }
    }
};

int main() {
    GameField field(10, 10);
    ShipManager shipManager;
    
    // Создание и размещение кораблей
    Ship ship1(3, Orientation::Horizontal);
    Ship ship2(2, Orientation::Vertical);
    
    if (!field.placeShip(ship1, 2, 2)) {
        throw std::runtime_error("Ship placement failed.");
    }
    if (!field.placeShip(ship2, 5, 5)) {
        throw std::runtime_error("Ship placement failed.");
    }
    shipManager.addShip(ship1);
    shipManager.addShip(ship2);
    
    // Добавим возможность атаковать по клетке
    size_t x, y;
    std::cout << "Enter coordinates to attack (x y): ";
    std::cin >> x >> y;
    
    field.attackCell(x, y);
    
    // Печать состояния кораблей
    std::cout << "State of ships after attack:" << std::endl;
    shipManager.printShips();
    
    // Печать состояния поля после атаки
    std::cout << "Game field after attack:" << std::endl;
    field.printField();
}
