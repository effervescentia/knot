/**
 container for a single unicode character and the cursor for its location
 in a source document
 */
type t = (Uchar.t, context_t)
and context_t = {cursor: Cursor.t};

/* static */

let create = (v: Uchar.t, c: context_t) => (v, c);

/* getters */

let value = (x: t) => fst(x);
let context = (x: t) => snd(x);
let cursor = (x: t) => context(x).cursor;

/* methods */

/**
 combine two inputs into a block
 */
let join = (cs: list(t)): Block.t(string) => {
  let (start, end_) = List.ends(cs) |> Tuple.map2(cursor);
  let value = cs |> List.map(value) |> String.of_uchars;

  Block.create(value, Cursor.join(start, end_));
};

/**
 convert input into a block
 */
let to_block = (x: t): Block.t(Uchar.t) =>
  Block.create(value(x), cursor(x));
