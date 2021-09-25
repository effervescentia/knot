/**
 container representing a selection of a source document
 */
type t = (Point.t, Point.t);

/* static */

let create = (start: Point.t, end_: Point.t) => (start, end_);

let zero = create(Point.zero, Point.zero);

/* getters */

let get_start = ((x, _): t): Point.t => x;
let get_end = ((_, x): t): Point.t => x;

/* methods */

/**
 combine two ranges
 */
let join = ((l_start, l_end): t, (r_start, r_end): t): t => (
  Point.min(l_start, r_start),
  Point.max(l_end, r_end),
);

/**
 check if a point exists withing a range
 */
let contains_point = (point: Point.t, (start, end_): t) =>
  Point.(
    Int.contains((start, end_) |> Tuple.map2(get_line), get_line(point))
    && (
      if (get_line(start) == get_line(end_)) {
        Int.contains(
          (start, end_) |> Tuple.map2(get_line),
          get_column(point),
        );
      } else if (get_line(start) == get_line(point)) {
        get_column(point) >= get_column(start);
      } else if (get_line(end_) == get_line(point)) {
        get_column(point) <= get_column(end_);
      } else {
        true;
      }
    )
  );

/**
 check if one range is contained within the other
 */
let contains = ((start, end_): t, target: t) =>
  contains_point(start, target) && contains_point(end_, target);

let to_string = ((start, end_): t) =>
  Point.(
    Print.fmt(
      "(%d:%d - %d:%d)",
      get_line(start),
      get_column(start),
      get_line(end_),
      get_column(end_),
    )
  );
