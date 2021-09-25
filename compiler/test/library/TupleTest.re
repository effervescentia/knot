open Kore;

let __pair = (0, 1);
let __trio = (0, 1, 2);

let suite =
  "Library.Tuple"
  >::: [
    "fst2()"
    >: (() => [(0, Tuple.fst2(__pair))] |> Assert.(test_many(int))),
    "snd2()"
    >: (() => [(1, Tuple.snd2(__pair))] |> Assert.(test_many(int))),
    "fst3()"
    >: (() => [(0, Tuple.fst3(__trio))] |> Assert.(test_many(int))),
    "snd3()"
    >: (() => [(1, Tuple.snd3(__trio))] |> Assert.(test_many(int))),
    "thd3()"
    >: (() => [(2, Tuple.thd3(__trio))] |> Assert.(test_many(int))),
    "map2()"
    >: (
      () =>
        [((1, 2), Tuple.map2((+)(1), __pair))]
        |> Assert.(test_many(int_pair))
    ),
    "map3()"
    >: (
      () =>
        [((1, 2, 3), Tuple.map3((+)(1), __trio))]
        |> Assert.(test_many(int_trio))
    ),
    "map_fst2()"
    >: (
      () =>
        [((1, 1), Tuple.map_fst2((+)(1), __pair))]
        |> Assert.(test_many(int_pair))
    ),
    "map_snd2()"
    >: (
      () =>
        [((0, 2), Tuple.map_snd2((+)(1), __pair))]
        |> Assert.(test_many(int_pair))
    ),
    "map_fst3()"
    >: (
      () =>
        [((1, 1, 2), Tuple.map_fst3((+)(1), __trio))]
        |> Assert.(test_many(int_trio))
    ),
    "map_snd3()"
    >: (
      () =>
        [((0, 2, 2), Tuple.map_snd3((+)(1), __trio))]
        |> Assert.(test_many(int_trio))
    ),
    "map_thd3()"
    >: (
      () =>
        [((0, 1, 3), Tuple.map_thd3((+)(1), __trio))]
        |> Assert.(test_many(int_trio))
    ),
    "join2()"
    >: (() => [(1, Tuple.join2((+), __pair))] |> Assert.(test_many(int))),
    "join3()"
    >: (
      () =>
        [(3, Tuple.join3((a, b, c) => a + b + c, __trio))]
        |> Assert.(test_many(int))
    ),
    "split2()"
    >: (
      () =>
        [((3, 1), Tuple.split2((+)(2), (-)(2), 1))]
        |> Assert.(test_many(int_pair))
    ),
    "split3()"
    >: (
      () =>
        [((3, 1, 2), Tuple.split3((+)(2), (-)(2), ( * )(2), 1))]
        |> Assert.(test_many(int_trio))
    ),
  ];
