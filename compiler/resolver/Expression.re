open Core;
open NestedHashtbl;

let resolve = promise =>
  fun
  | BooleanLit(_) => Some(Boolean_t)
  | NumericLit(_) => Some(Number_t)
  | StringLit(_) => Some(String_t)
  | JSX(jsx) => Some(JSX_t)

  | AddExpr(lhs, rhs)
  | SubExpr(lhs, rhs) =>
    switch (Util.typeof(lhs), Util.typeof(rhs)) {
    | (Some(Number_t), Some(Number_t)) => Some(Number_t)
    | (Some(String_t), Some(String_t)) => Some(String_t)
    | (None, _)
    | (_, None) => None
    | _ => raise(OperatorTypeMismatch)
    }

  | MulExpr(lhs, rhs)
  | DivExpr(lhs, rhs) =>
    switch (Util.typeof(lhs), Util.typeof(rhs)) {
    | (Some(Number_t), Some(Number_t)) => Some(Number_t)
    | (None, _)
    | (_, None) => None
    | _ => raise(OperatorTypeMismatch)
    }

  | LTExpr(lhs, rhs)
  | GTExpr(lhs, rhs)
  | LTEExpr(lhs, rhs)
  | GTEExpr(lhs, rhs) =>
    switch (Util.typeof(lhs), Util.typeof(rhs)) {
    | (Some(Number_t), Some(Number_t)) => Some(Boolean_t)
    | (None, _)
    | (_, None) => None
    | _ => raise(OperatorTypeMismatch)
    }

  | AndExpr(lhs, rhs)
  | OrExpr(lhs, rhs) =>
    switch (Util.typeof(lhs), Util.typeof(rhs)) {
    | (Some(Boolean_t), Some(Boolean_t)) => Some(Boolean_t)
    | (None, _)
    | (_, None) => None
    | _ => raise(OperatorTypeMismatch)
    }

  | Reference(refr) => Util.typeof(refr);
