open Kore;

module C = C.Character;
module M = Matchers;

let colon = M.symbol(C.colon);
let semicolon = M.symbol(C.semicolon);
let period = M.symbol(C.period);
let comma = M.symbol(C.comma);
let vertical_bar = M.symbol(C.vertical_bar);
let mixin = M.symbol(C.tilde);

let not = M.symbol(C.exclamation_mark);
let negative = M.symbol(C.minus_sign);
let positive = M.symbol(C.plus_sign);

let assign = M.symbol(C.equal_sign);

let multiply = M.symbol(C.asterisk);
let divide = M.symbol(C.forward_slash);
let add = M.symbol(C.plus_sign);
let subtract = M.symbol(C.minus_sign);

let exponent = M.symbol(C.caret);

let less_than = M.symbol(C.open_chevron);
let greater_than = M.symbol(C.close_chevron);

let open_inline_expr = M.symbol(C.open_brace);
let close_inline_expr = M.symbol(C.close_brace);

let open_closure = M.symbol(C.open_brace);
let close_closure = M.symbol(C.close_brace);

let open_group = M.symbol(C.open_paren);
let close_group = M.symbol(C.close_paren);
