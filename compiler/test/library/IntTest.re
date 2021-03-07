open Kore;

let suite =
  "Library.Int"
  >::: [
    "max_of()"
    >: (() => [(3, Int.max_of([2, 3, 1]))] |> Assert.(test_many(int))),
  ];
