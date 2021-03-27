open Kore;

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
            Cursor.range((3, 5), (4, 2)),
          ),
        ]
        |> Assert.(test_many(cursor))
    ),
    "join()"
    >: (
      () =>
        [
          (
            Cursor.Range({line: 3, column: 5}, {line: 4, column: 2}),
            Cursor.join(Cursor.point(3, 5), Cursor.point(4, 2)),
          ),
          (
            Cursor.Range({line: 1, column: 8}, {line: 2, column: 1}),
            Cursor.join(
              Cursor.range((1, 8), (1, 10)),
              Cursor.range((2, 0), (2, 1)),
            ),
          ),
          (
            Cursor.Range({line: 1, column: 10}, {line: 2, column: 1}),
            Cursor.join(
              Cursor.point(1, 10),
              Cursor.range((2, 0), (2, 1)),
            ),
          ),
          (
            Cursor.Range({line: 1, column: 8}, {line: 2, column: 0}),
            Cursor.join(
              Cursor.range((1, 8), (1, 10)),
              Cursor.point(2, 0),
            ),
          ),
        ]
        |> Assert.(test_many(cursor))
    ),
  ];
