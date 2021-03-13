open Error;
open Infix;

/**
 container for working on fragments of an AST
 */
type t('a) = {
  value: 'a,
  type_: Type.t,
  errors: option(list(compiler_err)),
  cursor: Cursor.t,
};

/* static */

let create =
    (~type_=Type.K_Invalid, ~errors=?, cursor: Cursor.t, value: 'a): t('a) => {
  value,
  type_,
  errors,
  cursor,
};

/* getters */

let value = (block: t('a)): 'a => block.value;
let cursor = (block: t('a)): Cursor.t => block.cursor;
let type_ = (block: t('a)): Type.t => block.type_;
let errors = (block: t('a)): option(list(compiler_err)) => block.errors;

/* methods */

/**
 replace the [value] of the block
 */
let replace = (value: 'b, block: t('a)): t('b) => {...block, value};

/**
 replace the [type_] of the block
 */
let cast = (type_: Type.t, block: t('a)): t('a) => {...block, type_};

/**
 transform the [value] of the block
 */
let map = (f: 'a => 'b, block: t('a)): t('b) =>
  replace(f(block.value), block);

/**
 combine two blocks
 */
let join =
    (
      ~type_=Type.K_Invalid,
      ~combine: ('a, 'a) => 'b,
      lhs: t('a),
      rhs: t('a),
    )
    : t('b) => {
  value: combine(lhs.value, rhs.value),
  type_,
  cursor: Cursor.join(lhs.cursor, rhs.cursor),
  errors: lhs.errors @? rhs.errors,
};

/**
 wrap a block in another block
 */
let wrap = (block: t('a)): t('b) => {...block, value: block};

/**
 transform the block and return the result
 */
let extend = (f: t('a) => t('b), block: t('a)): t('b) => f(block);

/**
 compare the blocks using their [value]
 */
let compare = (lhs: t('a), rhs: t('a)): bool => lhs.value == rhs.value;
