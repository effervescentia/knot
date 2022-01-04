open Kore;

let __pair = (0, 1);
let __trio = (0, 1, 2);

let suite =
  "Library.Tuple"
  >::: [
    "fst2()" >: (() => Assert.int(0, Tuple.fst2(__pair))),
    "snd2()" >: (() => Assert.int(1, Tuple.snd2(__pair))),
    "fst3()" >: (() => Assert.int(0, Tuple.fst3(__trio))),
    "snd3()" >: (() => Assert.int(1, Tuple.snd3(__trio))),
    "thd3()" >: (() => Assert.int(2, Tuple.thd3(__trio))),
    "map2()" >: (() => Assert.int_pair((1, 2), Tuple.map2((+)(1), __pair))),
    "map3()"
    >: (() => Assert.int_trio((1, 2, 3), Tuple.map3((+)(1), __trio))),
    "map_fst2()"
    >: (() => Assert.int_pair((1, 1), Tuple.map_fst2((+)(1), __pair))),
    "map_snd2()"
    >: (() => Assert.int_pair((0, 2), Tuple.map_snd2((+)(1), __pair))),
    "map_fst3()"
    >: (() => Assert.int_trio((1, 1, 2), Tuple.map_fst3((+)(1), __trio))),
    "map_snd3()"
    >: (() => Assert.int_trio((0, 2, 2), Tuple.map_snd3((+)(1), __trio))),
    "map_thd3()"
    >: (() => Assert.int_trio((0, 1, 3), Tuple.map_thd3((+)(1), __trio))),
    "join2()" >: (() => Assert.int(1, Tuple.join2((+), __pair))),
    "join3()"
    >: (() => Assert.int(3, Tuple.join3((a, b, c) => a + b + c, __trio))),
    "split2()"
    >: (() => Assert.int_pair((3, 1), Tuple.split2((+)(2), (-)(2), 1))),
    "split3()"
    >: (
      () =>
        Assert.int_trio(
          (3, 1, 2),
          Tuple.split3((+)(2), (-)(2), ( * )(2), 1),
        )
    ),
    "pp2()"
    >: (
      () => Assert.string("(0, 1)", __pair |> ~@Tuple.pp2(Fmt.int, Fmt.int))
    ),
    "pp3()"
    >: (
      () =>
        Assert.string(
          "(0, 1, 2)",
          __trio |> ~@Tuple.pp3(Fmt.int, Fmt.int, Fmt.int),
        )
    ),
  ];
