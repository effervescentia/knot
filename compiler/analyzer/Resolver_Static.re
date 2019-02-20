open Core;

exception OperatorTypeMismatch;

let check_resolution = (resolver, promise, expr) =>
  switch (resolver(promise, expr)) {
  | Some(t) =>
    promise := Resolved(expr, t);
    true;
  | None => false
  };

let typeof =
  fun
  | {contents: Pending(_)} => None
  | {contents: Resolved(_, t)} => Some(t);

let rec resolve =
  fun
  | DeclarationScope({contents: Pending(decl)} as promise) =>
    check_resolution(resolve_decl, promise, decl)
  | ExpressionScope({contents: Pending(expr)} as promise) =>
    check_resolution(resolve_expr, promise, expr)
  | ReferenceScope({contents: Pending(refr)} as promise) =>
    check_resolution(resolve_ref, promise, refr)
  | JSXScope({contents: Pending(jsx)} as promise) =>
    check_resolution(resolve_jsx, promise, jsx)
  | _ => false
and resolve_decl = promise =>
  fun
  | A_ConstDecl(_, expr) => typeof(expr)
  | _ => None
and resolve_expr = promise =>
  fun
  | A_BooleanLit(_) => Some(Boolean_t)
  | A_NumericLit(_) => Some(Number_t)
  | A_StringLit(_) => Some(String_t)

  | A_AddExpr(lhs, rhs)
  | A_SubExpr(lhs, rhs) =>
    switch (typeof(lhs), typeof(rhs)) {
    | (Some(Number_t), Some(Number_t)) => Some(Number_t)
    | (Some(String_t), Some(String_t)) => Some(String_t)
    | (None, _)
    | (_, None) => None
    | _ => raise(OperatorTypeMismatch)
    }

  | A_MulExpr(lhs, rhs)
  | A_DivExpr(lhs, rhs) =>
    switch (typeof(lhs), typeof(rhs)) {
    | (Some(Number_t), Some(Number_t)) => Some(Number_t)
    | (None, _)
    | (_, None) => None
    | _ => raise(OperatorTypeMismatch)
    }

  | A_LTExpr(lhs, rhs)
  | A_GTExpr(lhs, rhs)
  | A_LTEExpr(lhs, rhs)
  | A_GTEExpr(lhs, rhs) =>
    switch (typeof(lhs), typeof(rhs)) {
    | (Some(Number_t), Some(Number_t)) => Some(Boolean_t)
    | (None, _)
    | (_, None) => None
    | _ => raise(OperatorTypeMismatch)
    }

  | A_AndExpr(lhs, rhs)
  | A_OrExpr(lhs, rhs) =>
    switch (typeof(lhs), typeof(rhs)) {
    | (Some(Boolean_t), Some(Boolean_t)) => Some(Boolean_t)
    | (None, _)
    | (_, None) => None
    | _ => raise(OperatorTypeMismatch)
    }

  | A_JSX(jsx) => typeof(jsx)
  | A_Reference(refr) => typeof(refr)
and resolve_ref = promise =>
  fun
  | A_Variable(_) => None
  | _ => None
and resolve_jsx = promise =>
  fun
  | _ => Some(JSX_t);
