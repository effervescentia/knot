open Kore;

let (<=>) = (x, f) => M.spaces >> x << M.spaces >> return(f);

let assign = Character.equal_sign |> M.lexeme;

let mult = Character.asterisk <=> AST.of_mult_op;
let div = Character.forward_slash <=> AST.of_div_op;
let add = Character.plus_sign <=> AST.of_add_op;
let sub = Character.minus_sign <=> AST.of_sub_op;

let logical_and = Glyph.logical_and <=> AST.of_and_op;
let logical_or = Glyph.logical_or <=> AST.of_or_op;

let less_or_eql = Glyph.less_or_eql <=> AST.of_lte_op;
let less_than = Glyph.less_than <=> AST.of_lt_op;
let greater_or_eql = Glyph.greater_or_eql <=> AST.of_gte_op;
let greater_than = Glyph.greater_than <=> AST.of_gt_op;

let equality = Glyph.equality <=> AST.of_eq_op;
let inequality = Glyph.inequality <=> AST.of_ineq_op;
