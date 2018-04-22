#include "interface/util.h"
#include "soko/heuristic.h"
#include "soko/util.h"
#include <functional>
#include <variant>
#include <regex>

using soko::Cell;

namespace
{

std::regex::flag_type g_regexFlag = std::regex::ECMAScript | std::regex::icase;

void removeCarrete(std::string &str)
{
  while (!str.empty() && (str.back() == '\r' || str.back() == '\n'))
  {
    str.pop_back();
  }
}

std::string trimWhitespace(const std::string &str)
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

std::string getToken(std::string &str)
{
  // example of tokens:
  // abc
  // ab'c
  // ' abc def'
  // ' abc \' def' -> " abc ' def"
  str = trimWhitespace(str);
  if (str.empty())
  {
    return {};
  }
  if (str.front() == '\'')
  {
    auto startSearch = str.begin() + 1;
    while (true)
    {
      auto it = std::find(startSearch, str.end(), '\'');
      if (it == str.end())
      {
        throw std::logic_error("Unable to find end of literal: " + str);
      }
      if (*std::prev(it) == '\'')
      {
        it = std::next(str.erase(std::prev(it)));
      }
      else
      {
        std::string result = std::string(std::next(str.begin()), it);
        str.erase(str.begin(), std::next(it));
        return result;
      }
    }
  }

  auto it =
      std::find_if(str.begin(), str.end(), [](char c) { return std::isspace(c, std::locale()); });
  auto result = std::string(str.begin(), it);
  it = it == str.end() ? it : std::next(it);
  str.erase(str.begin(), it);
  return result;
}

int caseCompare(const std::string &s1, const std::string &s2)
{
  auto fn = [](auto a, auto b) { return a < b ? -1 : 1; };
  if (s1.size() != s2.size())
  {
    return fn(s1.size(), s2.size());
  }
  for (size_t i = 0; i < s1.size(); ++i)
  {
    char c1 = std::toupper(s1[i], std::locale());
    char c2 = std::toupper(s2[i], std::locale());
    if (c1 != c2)
    {
      return fn(c1, c2);
    }
  }
  return 0;
}

std::string toValue(const std::string &val)
{
  std::string result = trimWhitespace(val);
  if (result.empty() || (result.size() == 2 && result.front() == '\'' && result.back() == '\''))
  {
    throw std::logic_error("Empty value met");
  }
  if (result.size() > 2 && result.front() == '\'' && result.back() == '\'')
  {
    result = std::string(std::next(result.begin()), std::prev(result.end()));
  }
  return result;
}

enum class MapInfoPosition
{
  Before,
  After,
  None,
};

class MapInfoDelim {
public:
  MapInfoDelim() = default;
  virtual bool isMapEnd(const std::string &) noexcept = 0;
  virtual void reinit() {}
  virtual MapInfoPosition infoPos() const noexcept { return MapInfoPosition::None; }
};

class DelimFalse : public MapInfoDelim {
public:
  DelimFalse() = default;
  virtual bool isMapEnd(const std::string &str) noexcept override { return false; }
};

class DelimStartsWith : public MapInfoDelim {
public:
  DelimStartsWith(const std::string &s)
    : m_start(s)
  {}

  virtual bool isMapEnd(const std::string &str) noexcept override
  {
    if (str.size() < m_start.size())
    {
      return false;
    }
    return caseCompare(m_start, str.substr(0, m_start.size())) == 0;
  }
  virtual MapInfoPosition infoPos() const noexcept override { return MapInfoPosition::After; }

private:
  std::string m_start;
};

class DelimCounter : public MapInfoDelim {
public:
  DelimCounter(uint32_t cnt)
    : m_total(cnt)
  {}
  virtual bool isMapEnd(const std::string &) noexcept override
  {
    if (m_cnt == m_total)
    {
      return true;
    }
    ++m_cnt;
    return false;
  }
  virtual void reinit() noexcept override { m_cnt = 0; }
  virtual MapInfoPosition infoPos() const noexcept override { return MapInfoPosition::Before; }

private:
  uint32_t m_cnt = 0;
  const uint32_t m_total;
};

struct ReaderSettings
{
  std::map<char, soko::Cell> mapping;

  // template to extract name from info
  std::regex regexNameTemplate = std::regex(".*", g_regexFlag);
  std::unique_ptr<MapInfoDelim> mapInfoDelim = std::make_unique<DelimFalse>();
};

void setMapInfo(const std::string &value, ReaderSettings &result)
{
  std::vector<std::string> tokens;
  std::string tmp = value;
  while (!tmp.empty())
  {
    tokens.push_back(getToken(tmp));
  }

  assert(!tokens.empty());
  if (caseCompare(tokens[0], "Before") == 0)
  {
    if (tokens.size() < 3)
    {
      throw std::logic_error("Before: Expected argument");
    }
    if (caseCompare(tokens[1], "amount") != 0)
    {
      throw std::logic_error("Unable to parse MapInfo before condition: " + tokens[1]);
    }

    result.mapInfoDelim = std::make_unique<DelimCounter>(std::stoul(tokens[2]));
  }
  else if (caseCompare(tokens[0], "After") == 0)
  {
    if (tokens.size() < 3)
    {
      throw std::logic_error("After: Expected argument");
    }
    if (caseCompare(tokens[1], "starts_with") != 0)
    {
      throw std::logic_error("Unable to parse MapInfo before condition: " + tokens[1]);
    }

    result.mapInfoDelim = std::make_unique<DelimStartsWith>(getToken(tokens[2]));
  }
  else if (caseCompare(tokens[0], "None") == 0)
  {
    result.mapInfoDelim = std::make_unique<DelimFalse>();
  }
  else
  {
    throw std::logic_error("Unknown LevelNamePosition value: " + value);
  }
}

std::vector<std::string> readMapChunk(std::istream &is)
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

std::pair<std::string, soko::Map> parseFromData(const std::vector<std::string> &mapChunk,
                                                ReaderSettings &settings)
{
  const std::map<char, soko::Cell> &mapping = settings.mapping;

  settings.mapInfoDelim->reinit();
  auto it = std::find_if(mapChunk.begin(), mapChunk.end(),
                         [&settings](auto s) { return settings.mapInfoDelim->isMapEnd(s); });
  std::vector<std::string> map(mapChunk.begin(), it);
  std::vector<std::string> info(it, mapChunk.end());
  if (settings.mapInfoDelim->infoPos() == MapInfoPosition::Before)
  {
    std::swap(map, info);
  }

  std::string name;
  for (auto &str : info)
  {
    for (std::sregex_iterator i =
             std::sregex_iterator(str.begin(), str.end(), settings.regexNameTemplate);
         i != std::sregex_iterator(); ++i)
    {
      std::smatch m = *i;
      assert(m.size());
      // TODO: fix names
      name = m[0].str();
      break;
    }
    if (!name.empty())
    {
      break;
    }
  }

  std::vector<std::vector<Cell>> result;
  size_t max_column = 0;
  for (auto row : map)
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

  return {name, result};
}

struct CaseComparator
{
  bool operator()(const std::string &a, const std::string &b) const
  {
    return caseCompare(a, b) < 0;
  }
};

void readSettingValue(const std::string &key, const std::string &value, ReaderSettings &result)
{
  if (caseCompare(key, "MapInfo") == 0)
  {
    setMapInfo(value, result);
    return;
  }

  if (caseCompare(key, "levelname") == 0)
  {
    result.regexNameTemplate = std::regex(value, g_regexFlag);
    return;
  }

  static const std::map<std::string, soko::Cell, CaseComparator> exprs = {
      {"wall", Cell::Wall},
      {"field", Cell::Field},
      {"destination", Cell::Destination},
      {"unit", Cell::Unit},
      {"box", Cell::Box},
      {"destinationbox", Cell::BoxDestination},
      {"destinationunit", Cell::UnitDestination},
  };

  auto keyIt = exprs.find(key);
  if (keyIt == exprs.end())
  {
    throw std::logic_error("Unknown key: " + key);
  }
  for (auto ch : value)
  {
    auto it = result.mapping.insert({ch, keyIt->second});
    if (!it.second)
    {
      throw std::logic_error("Error parsing map settings: symbol '" + std::string(1, ch) +
                             "' occures several times");
    }
  }
}

ReaderSettings readSettings(std::istream &is)
{
  std::string s;
  ReaderSettings result;
  while (std::getline(is, s))
  {
    s = trimWhitespace(s);
    if (s.empty() || s.front() == '#')
    {
      continue;
    }
    if (caseCompare(s, "start levels") == 0)
    {
      return result;
    }
    auto equalSign = std::find(s.begin(), s.end(), '=');
    if (equalSign == s.end())
    {
      throw std::logic_error("Unparsed line: " + s);
    }
    std::string key = std::string(s.begin(), equalSign);
    std::string value = std::string(std::next(equalSign), s.end());
    readSettingValue(trimWhitespace(key), toValue(value), result);
  }
  throw std::logic_error("Can't read map settings");
}

} // namespace

std::vector<std::pair<std::string, soko::Map>> parseFromFile(std::istream &is)
{
  std::vector<std::pair<std::string, soko::Map>> result;
  std::vector<std::string> mapChunk;
  auto settings = readSettings(is);
  while (true)
  {
    if ((mapChunk = readMapChunk(is)).empty())
    {
      break;
    }
    result.push_back(parseFromData(mapChunk, settings));
  }
  // fill name fields, if they are not presented in file
  if (settings.mapInfoDelim->infoPos() == MapInfoPosition::None)
  {
    size_t i = 1;
    for (auto &it : result)
    {
      it.first = "Level " + std::to_string(i);
    }
  }
  return result;
}

size_t calculateHeuristic(const soko::Map &original, const soko::Heuristic &initedHeuristic)
{
  soko::MapState state;
  state.boxes = soko::getBoxes(original);
  state.unit = soko::getUnit(original);
  return initedHeuristic(state);
}
