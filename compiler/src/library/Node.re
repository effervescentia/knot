open Error;
open Infix;

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

/* getters */

let get_value = (node: t('a, 'b)): 'a => Tuple.fst3(node);
let get_type = (node: t('a, 'b)): 'b => Tuple.snd3(node);
let get_range = (node: t('a, 'b)): Range.t => Tuple.thd3(node);

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

module Raw = {
  /**
   container for working on un-typed fragments of a source document
   */
  type t('a) = ('a, Range.t);

  /* static */

  let create = (value: 'a, range: Range.t): t('a) => (value, range);

  /* getters */

  let get_value = (node: t('a)): 'a => fst(node);
  let get_range = (node: t('a)): Range.t => snd(node);

  /* pretty printing */

  let pp = (pp_value: Fmt.t('a)): Fmt.t(t('a)) =>
    (ppf, (value, range): t('a)) =>
      Fmt.pf(ppf, "%a %@ %a", pp_value, value, Range.pp, range);
};
