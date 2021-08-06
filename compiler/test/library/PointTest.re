open Kore;

let __point = Point.create(3, 5);

let suite =
  "Library.Point"
  >::: [
    "zero" >: (() => [((0, 0), Point.zero)] |> Assert.(test_many(point))),
    "get_line()"
    >: (() => [(3, Point.get_line(__point))] |> Assert.(test_many(int))),
    "get_column()"
    >: (() => [(5, Point.get_column(__point))] |> Assert.(test_many(int))),
    "to_string()"
    >: (
      () =>
        [("(3:5)", Point.to_string(__point))] |> Assert.(test_many(string))
    ),
  ];
