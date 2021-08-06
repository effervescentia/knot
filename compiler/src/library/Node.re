open Error;
open Infix;

/**
 container for working on fragments of an source document
 */
type t('a) = ('a, Type.t, Range.t);

/* static */

let create = (value: 'a, type_: Type.t, range: Range.t): t('a) => (
  value,
  type_,
  range,
);

/* getters */

let value = (node: t('a)): 'a => Tuple.fst3(node);
let type_ = (node: t('a)): Type.t => Tuple.snd3(node);
let range = (node: t('a)): Range.t => Tuple.thd3(node);

module Raw = {
  /**
 container for working on fragments of an source document
 */
  type t('a) = ('a, Range.t);

  /* static */

  let create = (value: 'a, range: Range.t): t('a) => (value, range);

  /* getters */

  let value = (node: t('a)): 'a => fst(node);
  let range = (node: t('a)): Range.t => snd(node);
};
