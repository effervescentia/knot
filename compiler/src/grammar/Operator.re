open Kore;

/**
 * binary operator
 */
let (<=>) = (x, f) => f <$ (M.lexeme(x) >> M.spaces);

let not_ = AST.of_not_op <$ Character.exclamation_mark;
let negative = AST.of_neg_op <$ Character.minus_sign;
let positive = AST.of_pos_op <$ Character.plus_sign;

let assign = (id, x) => M.binary_op(id, Character.equal_sign, x);

let mult = Character.asterisk <=> AST.of_mult_op;
let div = Character.forward_slash <=> AST.of_div_op;
let add = Character.plus_sign <=> AST.of_add_op;
let sub = Character.minus_sign <=> AST.of_sub_op;

let logical_and = Glyph.logical_and <=> AST.of_and_op;
let logical_or = Glyph.logical_or <=> AST.of_or_op;

let less_or_eql = Glyph.less_or_eql <=> AST.of_lte_op;
let less_than = Character.open_chevron <=> AST.of_lt_op;
let greater_or_eql = Glyph.greater_or_eql <=> AST.of_gte_op;
let greater_than = Character.close_chevron <=> AST.of_gt_op;

let equality = Glyph.equality <=> AST.of_eq_op;
let inequality = Glyph.inequality <=> AST.of_ineq_op;

let expo = Character.caret <=> ((l, r) => AST.of_expo_op((l, r)));
