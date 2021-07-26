/**
 container for a single unicode character and the cursor for its location
 in a source document
 */
type t = Node.Raw.t(Uchar.t);

/* static */

let create: (Uchar.t, Cursor.t) => t = Node.Raw.create;

/* getters */

let value: t => Uchar.t = Node.Raw.value;
let cursor: t => Cursor.t = Node.Raw.cursor;

/* methods */

/**
 combine two inputs into a block
 */
let join = (cs: list(t)): Node.Raw.t(string) => {
  let (start, end_) = List.ends(cs) |> Tuple.map2(cursor);
  let value = cs |> List.map(value) |> String.of_uchars;

  Node.Raw.create(value, Cursor.join(start, end_));
};

/**
 convert input into a block
 */
let to_node = (x: t): Node.Raw.t(Uchar.t) =>
  Node.Raw.create(value(x), cursor(x));
