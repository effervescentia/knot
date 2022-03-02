open Kore;

module K = C.Keyword;

let import = M.keyword(K.import);
let from = M.keyword(K.from);
let main = M.keyword(K.main);
let const = M.keyword(K.const);
let func = M.keyword(K.func);
let view = M.keyword(K.view);
let let_ = M.keyword(K.let_);
let nil = M.keyword(K.nil);
let true_ = M.keyword(K.true_);
let false_ = M.keyword(K.false_);
let as_ = M.keyword(K.as_);

/* primitive types */
let boolean = M.keyword(K.boolean);
let integer = M.keyword(K.integer);
let float = M.keyword(K.float);
let string = M.keyword(K.string);
let element = M.keyword(K.element);
