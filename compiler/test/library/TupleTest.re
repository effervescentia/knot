open Kore;

let suite =
  "Library - Tuple"
  >::: [
    "map2()"
    >: (
      () =>
        [((1, 2), Tuple.map2(x => x + 1, (0, 1)))]
        |> Assert.(test_many(int_pair))
    ),
    "map_fst2()"
    >: (
      () =>
        [((1, 1), Tuple.map_fst2(x => x + 1, (0, 1)))]
        |> Assert.(test_many(int_pair))
    ),
    "map_snd2()"
    >: (
      () =>
        [((0, 2), Tuple.map_snd2(x => x + 1, (0, 1)))]
        |> Assert.(test_many(int_pair))
    ),
  ];
