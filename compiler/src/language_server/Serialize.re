open Kore;

let symbol =
  Capabilities.(
    fun
    | File => 1
    | Module => 2
    | Namespace => 3
    | Package => 4
    | Class => 5
    | Method => 6
    | Property => 7
    | Field => 8
    | Constructor => 9
    | Enum => 10
    | Interface => 11
    | Function => 12
    | Variable => 13
    | Constant => 14
    | String => 15
    | Number => 16
    | Boolean => 17
    | Array => 18
    | Object => 19
    | Key => 20
    | Null => 21
    | EnumMember => 22
    | Object => 23
    | Event => 24
    | Operator => 25
    | TypeParameter => 26
  );

let range = ((start, end_): Range.t) =>
  `Assoc([
    (
      "start",
      `Assoc([
        ("line", `Int(Point.get_line(start) - 1)),
        ("character", `Int(Point.get_column(start) - 1)),
      ]),
    ),
    (
      "end",
      `Assoc([
        ("line", `Int(Point.get_line(end_) - 1)),
        ("character", `Int(Point.get_column(end_))),
      ]),
    ),
  ]);
