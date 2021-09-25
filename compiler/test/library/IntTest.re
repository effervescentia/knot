open Kore;

let suite =
  "Library.Int"
  >::: [
    "max_of()"
    >: (() => [(3, Int.max_of([2, 3, 1]))] |> Assert.(test_many(int))),
    "contains()"
    >: (
      () =>
        [
          (false, Int.contains((2, 4), 1)),
          (true, Int.contains((2, 4), 2)),
          (true, Int.contains((2, 4), 3)),
          (true, Int.contains((2, 4), 4)),
          (false, Int.contains((2, 4), 5)),
        ]
        |> Assert.(test_many(bool))
    ),
  ];
