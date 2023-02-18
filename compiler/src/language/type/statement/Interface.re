open Knot.Kore;
open AST.Common;

type variant_t = (identifier_t, list(KTypeExpression.Interface.node_t));

type t =
  | Declaration((identifier_t, KTypeExpression.Interface.node_t))
  | Type((identifier_t, KTypeExpression.Interface.node_t))
  | Enumerated((identifier_t, list(variant_t)));

type node_t = raw_t(t);

/* static */

let of_declaration = x => Declaration(x);
let of_type = x => Type(x);
let of_enumerated = x => Enumerated(x);

/* methods */

let fold = (~declaration, ~type_, ~enumerated) =>
  fun
  | Declaration(x) => declaration(x)
  | Type(x) => type_(x)
  | Enumerated(x) => enumerated(x);
