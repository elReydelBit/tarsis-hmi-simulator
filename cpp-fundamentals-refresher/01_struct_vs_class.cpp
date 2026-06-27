#include <iostream>

// ============================================================
// Demonstrates the ONLY real difference between struct and class
// in C++: default member visibility. Internally, both generate
// identical machine code.
// ============================================================

// struct: members are PUBLIC by default
struct PointStruct
{
    int x = 0;
    int y = 0;

    // Declared here, implemented below (outside the struct body)
    void movePointS(int dx, int dy);
};

// Out-of-class definition. The "PointStruct::" prefix (scope
// resolution operator) tells the compiler this function belongs
// to PointStruct, not a free-floating function with the same name.
void PointStruct::movePointS(int dx, int dy)
{
    x += dx;
    y += dy;
}

// class: members are PRIVATE by default
class PointClass
{
    int x = 0;
    int y = 0;

public:
    void movePointC(int dx, int dy);

    // Extra method: without this, there is NO way to read x/y
    // from outside. This is the real-world consequence of
    // private-by-default — access must be explicit, never assumed.
    void printPointC() const;
};

void PointClass::movePointC(int dx, int dy)
{
    x += dx;
    y += dy;
}

void PointClass::printPointC() const
{
    std::cout << "PointClass after move: (" << x << ", " << y << ")\n";
}

int main()
{
    // --- struct: direct access works, members are public ---
    PointStruct pointStructOne;
    pointStructOne.x = 5; // OK: public by default
    pointStructOne.movePointS(2, 3);
    std::cout << "PointStruct after move: ("
              << pointStructOne.x << ", " << pointStructOne.y << ")\n";

    // --- class: direct access is blocked, members are private ---
    PointClass pointClassOne;
    // pointClassOne.x = 5; // ERROR: x is private, no direct access allowed
    pointClassOne.movePointC(2, 3); // OK: public method is the only entry point
    pointClassOne.printPointC();    // needed because x/y can't be read directly

    return 0;
}