open Knot.Kore;
open AST.Common;

type variant_t = (identifier_t, list(KTypeExpression.Interface.node_t));

type t =
  | Declaration(identifier_t, KTypeExpression.Interface.node_t)
  | Type(identifier_t, KTypeExpression.Interface.node_t)
  | Enumerated(identifier_t, list(variant_t));

type node_t = raw_t(t);
