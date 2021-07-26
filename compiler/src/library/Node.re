open Error;
open Infix;

/**
 container for working on fragments of an source document
 */
type t('a) = ('a, Type.t, Cursor.t);

/* static */

let create = (value: 'a, type_: Type.t, cursor: Cursor.t): t('a) => (
  value,
  type_,
  cursor,
);

/* getters */

let value = (node: t('a)): 'a => Tuple.fst3(node);
let type_ = (node: t('a)): Type.t => Tuple.snd3(node);
let cursor = (node: t('a)): Cursor.t => Tuple.thd3(node);

module Raw = {
  /**
 container for working on fragments of an source document
 */
  type t('a) = ('a, Cursor.t);

  /* static */

  let create = (value: 'a, cursor: Cursor.t): t('a) => (value, cursor);

  /* getters */

  let value = (block: t('a)): 'a => fst(block);
  let cursor = (block: t('a)): Cursor.t => snd(block);
};
