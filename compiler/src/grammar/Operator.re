open Kore;

let not = AST.of_not_op <$ Symbol.not;
let negative = AST.of_neg_op <$ Symbol.negative;
let positive = AST.of_pos_op <$ Symbol.positive;

let assign = (id, x) => M.binary_op(id, Symbol.assign, x);

let mult = AST.of_mult_op <$ Symbol.multiply;
let div = AST.of_div_op <$ Symbol.divide;
let add = AST.of_add_op <$ Symbol.add;
let sub = AST.of_sub_op <$ Symbol.subtract;

let logical_and = AST.of_and_op <$ Glyph.logical_and;
let logical_or = AST.of_or_op <$ Glyph.logical_or;

let less_or_eql = AST.of_lte_op <$ Glyph.less_or_eql;
let less_than = AST.of_lt_op <$ Symbol.less_than;
let greater_or_eql = AST.of_gte_op <$ Glyph.greater_or_eql;
let greater_than = AST.of_gt_op <$ Symbol.greater_than;

let equality = AST.of_eq_op <$ Glyph.equality;
let inequality = AST.of_ineq_op <$ Glyph.inequality;

let expo = ((l, r) => AST.of_expo_op((l, r))) <$ Symbol.exponent;
