open Error;
open Infix;

module Raw = {
  /**
   container for working on un-typed fragments of a source document
   */
  type t('a) = ('a, Range.t);

  /* static */

  let create = (value: 'a, range: Range.t): t('a) => (value, range);

  /* getters */

  let get_value = ((value, _): t('a)): 'a => value;
  let get_range = ((_, range): t('a)): Range.t => range;

  /* methods */

  let map_value = (map: 'a => 'b, (value, range): t('a)): t('b) =>
    create(map(value), range);

  let map_range = (map: Range.t => Range.t, (value, range): t('a)): t('b) =>
    create(value, map(range));

  let join_ranges = (lhs: t('a), rhs: t('b)) =>
    Range.join(get_range(lhs), get_range(rhs));

  let wrap = (map: t('a) => 'b, (_, range) as node: t('a)): t('b) =>
    create(map(node), range);

  /* pretty printing */

  let pp = (pp_value: Fmt.t('a)): Fmt.t(t('a)) =>
    (ppf, (value, range): t('a)) =>
      Fmt.pf(ppf, "%a %@ %a", pp_value, value, Range.pp, range);
};

/**
 container for working on fragments of a source document
 */
type t('a, 'b) = ('a, 'b, Range.t);

/* static */

let create = (value: 'a, type_: 'b, range: Range.t): t('a, 'b) => (
  value,
  type_,
  range,
);

let of_raw = (type_: 'a, raw: Raw.t('b)) =>
  create(Raw.get_value(raw), type_, Raw.get_range(raw));

/* getters */

let get_value = (node: t('a, 'b)): 'a => Tuple.fst3(node);
let get_type = (node: t('a, 'b)): 'b => Tuple.snd3(node);
let get_range = (node: t('a, 'b)): Range.t => Tuple.thd3(node);

/* methods */

let map_value =
    (map: 'a => 'c, (value, type_, range): t('a, 'b)): t('c, 'b) =>
  create(map(value), type_, range);

let map_range =
    (map: Range.t => Range.t, (value, type_, range): t('a, 'b)): t('a, 'b) =>
  create(value, type_, map(range));

let join_ranges = (lhs: t('a, 'b), rhs: t('c, 'd)) =>
  Range.join(get_range(lhs), get_range(rhs));

let wrap =
    (map: t('a, 'b) => 'c, (_, type_, range) as node: t('a, 'b))
    : t('c, 'b) =>
  create(map(node), type_, range);

/* pretty printing */

let pp = (pp_value: Fmt.t('a), pp_type: Fmt.t('b)): Fmt.t(t('a, 'b)) =>
  (ppf, (value, type_, range): t('a, 'b)) =>
    Fmt.pf(
      ppf,
      "%a (%a) %@ %a",
      pp_value,
      value,
      pp_type,
      type_,
      Range.pp,
      range,
    );
