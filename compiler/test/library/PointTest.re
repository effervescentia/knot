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
    "compare()"
    >: (
      () =>
        [
          (0, Point.compare(__point, __point)),
          (1, Point.compare(Point.create(3, 6), __point)),
          ((-1), Point.compare(__point, Point.create(3, 6))),
          (1, Point.compare(__point, Point.create(2, 10))),
          ((-1), Point.compare(Point.create(2, 10), __point)),
        ]
        |> Assert.(test_many(int))
    ),
    "max()"
    >: (
      () =>
        [
          (__point, Point.max(__point, __point)),
          (__point, Point.max(Point.create(2, 0), __point)),
          (__point, Point.max(__point, Point.create(2, 0))),
          (Point.create(4, 0), Point.max(Point.create(4, 0), __point)),
          (Point.create(4, 0), Point.max(__point, Point.create(4, 0))),
        ]
        |> Assert.(test_many(point))
    ),
    "min()"
    >: (
      () =>
        [
          (__point, Point.min(__point, __point)),
          (__point, Point.min(Point.create(4, 0), __point)),
          (__point, Point.min(__point, Point.create(4, 0))),
          (Point.create(2, 0), Point.min(Point.create(2, 0), __point)),
          (Point.create(2, 0), Point.min(__point, Point.create(2, 0))),
        ]
        |> Assert.(test_many(point))
    ),
    "pp()"
    >: (
      () => [("3.5", __point |> ~@Point.pp)] |> Assert.(test_many(string))
    ),
  ];
