open Kore;

module Keyword = Constants.Keyword;

let import = Matchers.keyword(Keyword.import);
let from = Matchers.keyword(Keyword.from);
let main = Matchers.keyword(Keyword.main);
let const = Matchers.keyword(Keyword.const);
let enum = Matchers.keyword(Keyword.enum);
let func = Matchers.keyword(Keyword.func);
let view = Matchers.keyword(Keyword.view);
let style = Matchers.keyword(Keyword.style);
let let_ = Matchers.keyword(Keyword.let_);
let nil = Matchers.keyword(Keyword.nil);
let true_ = Matchers.keyword(Keyword.true_);
let false_ = Matchers.keyword(Keyword.false_);
let as_ = Matchers.keyword(Keyword.as_);
let on = Matchers.keyword(Keyword.on);
let declare = Matchers.keyword(Keyword.declare);
let decorator = Matchers.keyword(Keyword.decorator);
let type_ = Matchers.keyword(Keyword.type_);
let module_ = Matchers.keyword(Keyword.module_);

/* primitive types */
let boolean = Matchers.keyword(Keyword.boolean);
let integer = Matchers.keyword(Keyword.integer);
let float = Matchers.keyword(Keyword.float);
let string = Matchers.keyword(Keyword.string);
let element = Matchers.keyword(Keyword.element);
