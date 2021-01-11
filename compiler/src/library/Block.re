open Error;

type t('a) = {
  value: 'a,
  type_: option(Type.t),
  errors: option(list(compiler_err)),
  cursor: Cursor.t,
};

let create = (~type_=?, ~errors=?, value: 'a, cursor: Cursor.t): t('a) => {
  value,
  type_,
  errors,
  cursor,
};

let value = (block: t('a)): 'a => block.value;
let cursor = (block: t('a)): Cursor.t => block.cursor;

let map = (f: 'a => 'b, block: t('a)): t('b) => {
  ...block,
  value: f(block.value),
};
