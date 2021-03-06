/**
 * Container for working on fragments of an AST.
 */
open Error;
open Infix;

type t('a) = {
  value: 'a,
  type_: Type.t,
  errors: option(list(compiler_err)),
  cursor: Cursor.t,
};

let create =
    (~type_=Type.K_Invalid, ~errors=?, cursor: Cursor.t, value: 'a): t('a) => {
  value,
  type_,
  errors,
  cursor,
};

let value = (block: t('a)): 'a => block.value;
let cursor = (block: t('a)): Cursor.t => block.cursor;
let type_ = (block: t('a)): Type.t => block.type_;
let errors = (block: t('a)): option(list(compiler_err)) => block.errors;

let replace = (value: 'b, block: t('a)): t('b) => {...block, value};

let cast = (type_: Type.t, block: t('a)): t('a) => {...block, type_};

let map = (f: 'a => 'b, block: t('a)): t('b) =>
  replace(f(block.value), block);

let join =
    (~type_=Type.K_Invalid, ~combine=(++), lhs: t(string), rhs: t(string))
    : t(string) => {
  value: combine(lhs.value, rhs.value),
  type_,
  cursor: Cursor.range(lhs.cursor, rhs.cursor),
  errors: lhs.errors @? rhs.errors,
};

let wrap = (block: t('a)): t('b) => {...block, value: block};

let extend = (f: t('a) => t('b), block: t('a)): t('b) => f(block);

let compare = (lhs: t('a), rhs: t('a)): bool => lhs.value == rhs.value;
