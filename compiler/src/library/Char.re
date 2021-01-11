include Input.Make({
  type t = (Uchar.t, Cursor.t);
  type f = Uchar.t;
  type s = Cursor.t;

  let fst = fst;
  let snd = snd;

  let create = (f, s) => (f, s);
});

let join = (cs: list(t)): Block.t(string) => {
  let (start, end_) = List.ends(cs) |> Tuple.map2(context);
  let name = cs |> List.map(value) |> String.of_uchars;

  Block.create(Cursor.range(start, end_), name);
};
