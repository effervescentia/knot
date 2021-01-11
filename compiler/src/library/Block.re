open Error;
open Infix;

type t('a) = {
  value: 'a,
  type_: option(Type.t),
  errors: option(list(compiler_err)),
  cursor: Cursor.t,
};

let create = (~type_=?, ~errors=?, cursor: Cursor.t, value: 'a): t('a) => {
  value,
  type_,
  errors,
  cursor,
};

let value = (block: t('a)): 'a => block.value;
let cursor = (block: t('a)): Cursor.t => block.cursor;
let type_ = (block: t('a)): option(Type.t) => block.type_;

let replace = (x: 'b, block: t('a)): t('b) => {...block, value: x};

let map = (f: 'a => 'b, block: t('a)): t('b) =>
  replace(f(block.value), block);

let join = (~combine=(++), lhs: t(string), rhs: t(string)): t(string) => {
  value: combine(lhs.value, rhs.value),
  type_: None,
  cursor: Cursor.range(lhs.cursor, rhs.cursor),
  errors: lhs.errors @? rhs.errors,
};

let compare = (lhs: t('a), rhs: t('a)): bool => lhs.value == rhs.value;
