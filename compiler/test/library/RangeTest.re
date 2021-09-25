open Kore;

let __start_point = Point.create(3, 5);
let __end_point = Point.create(4, 8);
let __range = Range.create(__start_point, __end_point);

let suite =
  "Library.Range"
  >::: [
    "zero"
    >: (
      () => [(((0, 0), (0, 0)), Range.zero)] |> Assert.(test_many(range))
    ),
    "get_start()"
    >: (
      () =>
        [(__start_point, Range.get_start(__range))]
        |> Assert.(test_many(point))
    ),
    "get_end()"
    >: (
      () =>
        [(__end_point, Range.get_end(__range))]
        |> Assert.(test_many(point))
    ),
    "join()"
    >: (
      () =>
        [
          (
            Range.create(__start_point, (8, 11)),
            Range.join(__range, ((5, 1), (8, 11))),
          ),
          (
            Range.create(__start_point, (8, 11)),
            Range.join(((5, 1), (8, 11)), __range),
          ),
        ]
        |> Assert.(test_many(range))
    ),
    "contains_point()"
    >: (
      () =>
        [
          (true, Range.contains_point(Point.create(3, 10), __range)),
          (true, Range.contains_point(__start_point, __range)),
          (true, Range.contains_point(__end_point, __range)),
          (false, Range.contains_point(Point.create(2, 0), __range)),
          (false, Range.contains_point(Point.create(5, 0), __range)),
        ]
        |> Assert.(test_many(bool))
    ),
    "contains()"
    >: (
      () =>
        [
          (true, Range.contains(__range, __range)),
          (false, Range.contains(((1, 1), (2, 11)), __range)),
          (false, Range.contains(((1, 1), (4, 0)), __range)),
          (false, Range.contains(((1, 1), (8, 11)), __range)),
          (false, Range.contains(((2, 11), (8, 11)), __range)),
          (false, Range.contains(((5, 1), (8, 11)), __range)),
          (false, Range.contains((__start_point, (8, 11)), __range)),
          (false, Range.contains(((5, 1), __end_point), __range)),
        ]
        |> Assert.(test_many(bool))
    ),
  ];
