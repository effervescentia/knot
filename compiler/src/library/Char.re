/**
 * Container for a single unicode character and the cursor state associated with its location
 * in the source document.
 */

type t = (Uchar.t, Cursor.t);

let value = (x: t) => fst(x);
let context = (x: t) => snd(x);

let create = (v: Uchar.t, c: Cursor.t) => (v, c);

let join = (cs: list(t)): Block.t(string) => {
  let (start, end_) = List.ends(cs) |> Tuple.map2(context);
  let value = cs |> List.map(value) |> String.of_uchars;

  Block.create(Cursor.join(start, end_), value);
};

let to_block = (x: t): Block.t(Uchar.t) =>
  Block.create(context(x), value(x));
