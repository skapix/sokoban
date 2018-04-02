#pragma once

namespace soko
{

enum class Cell : char
{
  Field = 0,
  Wall = 1,
  Unit = 2,
  Box = 4,
  Destination = 8,
  BoxDestination = Cell::Box | Cell::Destination,
  UnitDestination = Cell::Unit | Cell::Destination,
};
}
