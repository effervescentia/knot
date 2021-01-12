type t = (Uchar.t, Cursor.t);

let value = (x: t) => fst(x);
let context = (x: t) => snd(x);

let create = (v: Uchar.t, c: Cursor.t) => (v, c);

let join = (cs: list(t)): Block.t(string) => {
  let (start, end_) = List.ends(cs) |> Tuple.map2(context);
  let name = cs |> List.map(value) |> String.of_uchars;

  Block.create(Cursor.range(start, end_), name);
};

let to_block = (x: t): Block.t(Uchar.t) =>
  Block.create(context(x), value(x));
