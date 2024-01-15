# vort-ecs

# usage

```c++
#include <ecs.hpp>


struct Position {
   float x;
   float y;
   float z;
};

struct Scale {
   float x;
   float y;
   float z;
};
struct Orientation {
   float x;
   float y;
   float z;
   float w;
};

struct Time {
   float elapsed;
};


void startup(Command &command) {
   for (int i = 0; i < 10; i++) {

      // adds a new Entity with Position, Scale and Orientation component
      auto entityId = command.spawn(Position{0.0f, 0.0f, 0.0f}, Scale{1.0f, 1.0f, 1.0f}, Orientation{0.0f, 0.0f, 0.0f, 1.0f});
   }

   command.addGlobal(Time{0.0f});
}

void testSystem(Query<Columns<Position, Scale, Orientation>, Tag<>> &query, Global<Time> &global) {
   auto [time] = global;

   for (auto [position, scale, orientation]: query) {
      // do stuffs with it
   }
}

int main() {
   Entities().addSystem(SystemSchedule::Startup, startup).addSystem(SystemSchedule::Update, testSystem).run();

   return 0;
}
```
