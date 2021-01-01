open Kore;

module K = C.Keyword;

let _kwd = x => () <$ M.token(x);

let import = _kwd(K.import);
let from = _kwd(K.from);
let const = _kwd(K.const);
let let_ = _kwd(K.let_);
let nil = _kwd(K.nil);
let true_ = _kwd(K.true_);
let false_ = _kwd(K.false_);
