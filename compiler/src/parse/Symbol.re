open Kore;

module Character = Constants.Character;
module M = Matchers;

let colon = Matchers.symbol(Character.colon);
let semicolon = Matchers.symbol(Character.semicolon);
let period = Matchers.symbol(Character.period);
let comma = Matchers.symbol(Character.comma);
let vertical_bar = Matchers.symbol(Character.vertical_bar);
let mixin = Matchers.symbol(Character.tilde);

let not = Matchers.symbol(Character.exclamation_mark);
let negative = Matchers.symbol(Character.minus_sign);
let positive = Matchers.symbol(Character.plus_sign);

let assign = Matchers.symbol(Character.equal_sign);

let multiply = Matchers.symbol(Character.asterisk);
let divide = Matchers.symbol(Character.forward_slash);
let add = Matchers.symbol(Character.plus_sign);
let subtract = Matchers.symbol(Character.minus_sign);

let exponent = Matchers.symbol(Character.caret);

let less_than = Matchers.symbol(Character.open_chevron);
let greater_than = Matchers.symbol(Character.close_chevron);

let open_inline_expr = Matchers.symbol(Character.open_brace);
let close_inline_expr = Matchers.symbol(Character.close_brace);

let open_closure = Matchers.symbol(Character.open_brace);
let close_closure = Matchers.symbol(Character.close_brace);

let open_group = Matchers.symbol(Character.open_paren);
let close_group = Matchers.symbol(Character.close_paren);
