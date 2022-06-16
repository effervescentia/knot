module Fmt = Pretty.Formatters;

/**
 container for a single unicode character and the point for its location
 in a source document
 */
type t = (Uchar.t, Point.t);

/* static */

let create = (uchar: Uchar.t, point: Point.t): t => (uchar, point);

/* getters */

let get_value = ((x, _): t): Uchar.t => x;
let get_point = ((_, x): t): Point.t => x;

/* methods */

/**
 combine a list of inputs into a raw node
 */
let join = (cs: list(t)): Node2.t(string, unit) => {
  let (start, end_) = List.ends(cs) |> Tuple.map2(get_point);
  let value = cs |> List.map(get_value) |> String.of_uchars;

  Node2.untyped(value, Range.create(start, end_));
};

/**
 convert input into a node
 */
let to_node = (x: t): Node2.t(Uchar.t, unit) =>
  Node2.untyped(get_value(x), Range.create(get_point(x), get_point(x)));

/* pretty printing */

let pp: Fmt.t(t) =
  (ppf, (uchar, point): t) =>
    Fmt.pf(ppf, "%a%@%a", Fmt.uchar, uchar, Point.pp, point);
