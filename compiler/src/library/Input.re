/**
 container for a single unicode character and the point for its location
 in a source document
 */
type t = (Uchar.t, Point.t);

/* static */

let create = (uchar: Uchar.t, point: Point.t): t => (uchar, point);

/* getters */

let value = ((x, _): t): Uchar.t => x;
let point = ((_, x): t): Point.t => x;

/* methods */

/**
 combine two inputs into a node
 */
let join = (cs: list(t)): Node.Raw.t(string) => {
  let (start, end_) = List.ends(cs) |> Tuple.map2(point);
  let value = cs |> List.map(value) |> String.of_uchars;

  Node.Raw.create(value, Range.create(start, end_));
};

/**
 convert input into a node
 */
let to_node = (x: t): Node.Raw.t(Uchar.t) =>
  Node.Raw.create(value(x), Range.create(point(x), point(x)));
