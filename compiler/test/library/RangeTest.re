open Kore;

let __start_point = Point.create(3, 5);
let __end_point = Point.create(3, 5);
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
        ]
        |> Assert.(test_many(range))
    ),
  ];
