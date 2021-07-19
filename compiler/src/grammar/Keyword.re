open Kore;

module K = C.Keyword;

/* declarations */
let const = M.keyword(K.const);
let type_ = M.keyword(K.type_);
let func = M.keyword(K.func);
let let_ = M.keyword(K.let_);

/* utility */
let main = M.keyword(K.main);
let as_ = M.keyword(K.as_);

/* import */
let import = M.keyword(K.import);
let from = M.keyword(K.from);

/* primitives */
let nil = M.keyword(K.nil);
let true_ = M.keyword(K.true_);
let false_ = M.keyword(K.false_);

/* type primitives */
let bool = M.keyword(K.bool);
let int = M.keyword(K.int);
let float = M.keyword(K.float);
let number = M.keyword(K.number);
let string = M.keyword(K.string);
let element = M.keyword(K.element);
