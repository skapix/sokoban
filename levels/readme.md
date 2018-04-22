## Levels structure


Level file can be logically splitted into 2 separate parts: `LevelSettings` and `Maps` themselves.

#### LevelSetting

This section contains information about map parsing.

One character represents a single cell in map.
In order to set a cell representation you should fit the Cell Table, that looks in the following way:
```
Key=Value
```

Where `key` is one of following values: `Field`, `Unit`, `Box`, `Destination`, `DestinationBox`, `DestinationUnit`, `Wall`. All key values are case-insensetive.
`Value` can be a single or multiple characters. Space characters should be placed inside quotes.

Comments starts from `#` (only in this section).

Maps can have an oprional information. If information exists, it can take place either before map or after.
Level names can be parsed from the information. For more info, take a look into examples.


#### Maps

Level map and info should be grouped together and no empty lines between them are allowed.


##### Other information
 * Level structure is parsed inside `interface/util.cpp`.
 * The examples are places in this directory.
