open Kore;

module C = C.Character;
module M = Matchers;

let not = M.symbol(C.exclamation_mark);
let exponent = M.symbol(C.caret);

let open_inline_expr = M.symbol(C.open_brace);
let close_inline_expr = M.symbol(C.close_brace);

let open_closure = M.symbol(C.open_brace);
let close_closure = M.symbol(C.close_brace);

let open_group = M.symbol(C.open_paren);
let close_group = M.symbol(C.close_paren);
