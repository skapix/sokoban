#include "interface/util.h"

using soko::Cell;

namespace
{

void removeCarrete(std::string &str)
{
  while (!str.empty() && (str.back() == '\r' || str.back() == '\n'))
  {
    str.pop_back();
  }
}

std::string trimWhitespace(const std::string str)
{
  const char *spaces = "\r\n \t";
  auto end = str.find_last_not_of(spaces);
  if (end == std::string::npos)
  {
    return "";
  }
  auto start = str.find_first_not_of(spaces);
  return str.substr(start, end - start + 1);
}

std::string readName(std::istream &is)
{
  std::string result;
  while (std::getline(is, result))
  {
    // assume, name is obligatory
    result = trimWhitespace(result);
    if (!result.empty())
    {
      break;
    }
  }
  return result;
}


std::vector<std::string> readMap(std::istream &is)
{
  std::string tmp;
  while (std::getline(is, tmp))
  {
    removeCarrete(tmp);
    if (!tmp.empty())
    {
      break;
    }
  }
  if (tmp.empty())
  {
    return {};
  }
  std::vector<std::string> result;
  result.push_back(tmp);

  while (std::getline(is, tmp))
  {
    removeCarrete(tmp);
    if (tmp.empty())
    {
      break;
    }
    result.push_back(tmp);
  }
  return result;
}

soko::Map parseFromData(const std::vector<std::string> &possible_map)
{
  static const std::map<char, Cell> mapping = {{'0', Cell::Field},
                                               {'1', Cell::Unit},
                                               {'2', Cell::Box},
                                               {'3', Cell::Destination},
                                               {'4', Cell::BoxDestination},
                                               {'5', Cell::UnitDestination},
                                               {'6', Cell::Wall},
                                               {' ', Cell::Wall}};
  std::vector<std::vector<Cell>> result;
  size_t max_column = 0;

  for (auto row : possible_map)
  {
    size_t column = 0;
    result.push_back({});
    for (; column < row.size(); ++column)
    {
      char c = row[column];
      auto it = mapping.find(c);
      if (it == mapping.end())
      {
        throw std::logic_error(std::string("Unknown map symbol ") + std::string(1, c));
      }

      result.back().push_back(it->second);
    }
    max_column = std::max(max_column, column);
  }

  if (max_column == 0)
  {
    throw std::logic_error("Empty map");
  }

  for (auto &it : result)
  {
    it.resize(max_column, Cell::Wall);
  }

  return result;
}

} // namespace

std::vector<std::pair<std::string, soko::Map>> parseFromFile(std::istream &is)
{
  std::vector<std::pair<std::string, soko::Map>> result;
  while (true)
  {
    auto name = readName(is);
    auto map = readMap(is);
    if (name.empty() || map.empty())
    {
      return result;
    }

    result.push_back({name, parseFromData(map)});
  }
}
