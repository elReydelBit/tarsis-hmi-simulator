#include <iostream>
#include <memory>   // for shared_ptr, weak_ptr, make_shared

// A Waypoint represents one stop in a UAV mission route.
// Using "struct" here because there is no invariant to protect:
// no rule would be broken by accessing label/next/previous directly.
//
// Ownership model:
// - "next" is a strong owner: the route keeps the next waypoint alive
//   as long as someone holds a shared_ptr to this one.
// - "previous" is a weak observer: it can check the previous waypoint
//   still exists, but it never keeps it alive by itself.
struct Waypoint
{
    std::string label;                 // default-initialized to an empty string
    std::shared_ptr<Waypoint> next;    // default-initialized to null (no target)
    std::weak_ptr<Waypoint> previous;  // default-initialized to empty (no target)

    Waypoint(std::string lbl);
    ~Waypoint();
};

// Constructor: runs automatically when a Waypoint is created.
// Stores the label and prints a trace so we can see object
// lifetime in the console output.
Waypoint::Waypoint(std::string lbl) : label(lbl)
{
    std::cout << "Waypoint created: " << label << "\n";
}

// Destructor: runs automatically when the object is destroyed
// (e.g. when the last shared_ptr owning it goes out of scope,
// or after reset()). This is RAII in action: cleanup is guaranteed,
// no manual call needed.
Waypoint::~Waypoint()
{
    std::cout << "Waypoint destroyed: " << label << "\n";
}


int main (){

    // make_shared constructs a Waypoint on the heap (calling the
    // constructor we wrote above) and wraps it in a shared_ptr.
    //
    // auto: the compiler infers wp1's type from the right-hand side.
    // make_shared<Waypoint>(...) returns a std::shared_ptr<Waypoint>,
    // so auto becomes exactly that type. Writing it manually would be:
    //   std::shared_ptr<Waypoint> wp1 = std::make_shared<Waypoint>("WP1");
    // auto just avoids typing the type twice -- resolved at COMPILE time,
    // nothing dynamic happens at runtime.
    auto wp1 = std::make_shared<Waypoint>("WP1");
    auto wp2 = std::make_shared<Waypoint>("WP2");

    // STRONG link forward: wp1->next now ALSO owns wp2.
    // Before: wp2 had ONE owner (itself) -> use_count = 1
    // After:  wp2 has TWO owners (itself + wp1->next) -> use_count = 2
    wp1->next = wp2;

    // WEAK link backward: wp2->previous OBSERVES wp1 but does not own it.
    // wp1's use_count stays at 1 -- this line does not touch it.
    wp2->previous = wp1;

    std::cout << "wp1 use_count: " << wp1.use_count() << "\n"; // 1
    std::cout << "wp2 use_count: " << wp2.use_count() << "\n"; // 2

    // .lock() is the ONLY safe way to use a weak_ptr's target.
    // weak_ptr has no "->" operator on purpose -- the target might
    // already be dead, and using it directly would be undefined
    // behavior (use-after-free).
    //
    // What .lock() does:
    // 1) checks if the object is still alive (some shared_ptr owns it)
    // 2) if YES -> returns a valid, temporary shared_ptr to it
    // 3) if NO  -> returns an EMPTY shared_ptr (holds nullptr)
    //
    // "if (auto temp = ...)" works because a shared_ptr converts to
    // false when empty, true when it holds something -- so this if
    // is really asking "did lock() succeed?"
    if (auto temp = wp2->previous.lock())
    {
        std::cout << "wp2's previous is still alive: " << temp->label << "\n";
    }

    // wp1.reset() destroys what wp1 owns and drops use_count by 1.
    // wp1 was the ONLY strong (shared_ptr) owner of WP1, so use_count
    // hits 0 right here -> the destructor fires IMMEDIATELY, printing
    // "Waypoint destroyed: WP1".
    //
    // wp2->previous (weak_ptr) was NEVER counted as an owner, so it
    // has no power to stop this -- that's the entire purpose of weak_ptr.
    wp1.reset();

    // Same .lock() call, but the target is gone now.
    if (auto temp = wp2->previous.lock())
    {
        std::cout << "wp2's previous is still alive: " << temp->label << "\n";
    }
    else
    {
        // This branch runs now: lock() safely detected the object is
        // dead and returned an empty shared_ptr instead of crashing.
        std::cout << "wp2's previous no longer exists (safely detected, no crash)\n";
    }

    return 0;
}