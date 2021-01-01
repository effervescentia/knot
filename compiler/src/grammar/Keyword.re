open Kore;

module K = C.Keyword;

let _kwd = x => () <$ M.token(x);

let import = _kwd(K.import);
let from = _kwd(K.from);
let const = _kwd(K.const);
let let_ = _kwd(K.let_);
