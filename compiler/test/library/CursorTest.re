open Kore;

let _range = ((l, l'), (r, r')) =>
  Cursor.range(Cursor.point(l, l'), Cursor.point(r, r'));

let suite =
  "Library.Cursor"
  >::: [
    "zero"
    >: (
      () =>
        [(Cursor.Point({line: 0, column: 0}), Cursor.zero)]
        |> Assert.(test_many(cursor))
    ),
    "point()"
    >: (
      () =>
        [(Cursor.Point({line: 3, column: 5}), Cursor.point(3, 5))]
        |> Assert.(test_many(cursor))
    ),
    "range()"
    >: (
      () =>
        [
          (
            Cursor.Range({line: 3, column: 5}, {line: 4, column: 2}),
            _range((3, 5), (4, 2)),
          ),
          (
            Cursor.Range({line: 1, column: 8}, {line: 2, column: 1}),
            Cursor.range(
              _range((1, 8), (1, 10)),
              _range((2, 0), (2, 1)),
            ),
          ),
          (
            Cursor.Range({line: 1, column: 10}, {line: 2, column: 1}),
            Cursor.range(Cursor.point(1, 10), _range((2, 0), (2, 1))),
          ),
          (
            Cursor.Range({line: 1, column: 8}, {line: 2, column: 0}),
            Cursor.range(_range((1, 8), (1, 10)), Cursor.point(2, 0)),
          ),
        ]
        |> Assert.(test_many(cursor))
    ),
    "to_string()"
    >: (
      () =>
        [
          ("(1:2)", Cursor.point(1, 2) |> Cursor.to_string),
          (
            "(3:5 - 4:2)",
            Cursor.range(Cursor.point(3, 5), Cursor.point(4, 2))
            |> Cursor.to_string,
          ),
        ]
        |> Assert.(test_many(string))
    ),
  ];
