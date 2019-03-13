open Core;
open NestedHashtbl;

let resolve = promise =>
  (
    switch (fst(promise)) {
    | BooleanLit(_) => Some(Resolved(Boolean_t))
    | NumericLit(_) => Some(Resolved(Number_t))
    | StringLit(_) => Some(Resolved(String_t))
    | JSX(jsx) => Some(Resolved(JSX_t))

    | AddExpr(lhs, rhs)
    | SubExpr(lhs, rhs) =>
      switch (Util.typeof(lhs), Util.typeof(rhs)) {
      | (Some(Number_t), Some(Number_t)) => Some(Resolved(Number_t))
      | (Some(String_t), Some(String_t)) => Some(Resolved(String_t))
      | (Some(Number_t), Some(Any_t)) =>
        snd(rhs) := Synthetic(Number_t, []);
        Some(Resolved(Number_t));
      | (Some(Any_t), Some(Number_t)) =>
        snd(lhs) := Synthetic(Number_t, []);
        Some(Resolved(Number_t));
      | (Some(String_t), Some(Any_t)) =>
        snd(rhs) := Synthetic(String_t, []);
        Some(Resolved(String_t));
      | (Some(Any_t), Some(String_t)) =>
        snd(lhs) := Synthetic(String_t, []);
        Some(Resolved(String_t));
      | (None, _)
      | (_, None) => None
      | _ => raise(OperatorTypeMismatch)
      }

    | MulExpr(lhs, rhs)
    | DivExpr(lhs, rhs) =>
      switch (Util.typeof(lhs), Util.typeof(rhs)) {
      | (Some(Number_t), Some(Number_t)) => Some(Resolved(Number_t))
      | (Some(Number_t), Some(Any_t)) =>
        snd(rhs) := Synthetic(Number_t, []);
        Some(Resolved(Number_t));
      | (Some(Any_t), Some(Number_t)) =>
        snd(lhs) := Synthetic(Number_t, []);
        Some(Resolved(Number_t));
      | (None, _)
      | (_, None) => None
      | _ => raise(OperatorTypeMismatch)
      }

    | LTExpr(lhs, rhs)
    | GTExpr(lhs, rhs)
    | LTEExpr(lhs, rhs)
    | GTEExpr(lhs, rhs) =>
      switch (Util.typeof(lhs), Util.typeof(rhs)) {
      | (Some(Number_t), Some(Number_t)) => Some(Resolved(Boolean_t))
      | (Some(Number_t), Some(Any_t)) =>
        snd(rhs) := Synthetic(Number_t, []);
        Some(Resolved(Boolean_t));
      | (Some(Any_t), Some(Number_t)) =>
        snd(lhs) := Synthetic(Number_t, []);
        Some(Resolved(Boolean_t));
      | (None, _)
      | (_, None) => None
      | _ => raise(OperatorTypeMismatch)
      }

    | AndExpr(lhs, rhs)
    | OrExpr(lhs, rhs) =>
      switch (Util.typeof(lhs), Util.typeof(rhs)) {
      | (Some(Boolean_t), Some(Boolean_t)) => Some(Resolved(Boolean_t))
      | (Some(Boolean_t), Some(Any_t)) =>
        snd(rhs) := Synthetic(Boolean_t, []);
        Some(Resolved(Boolean_t));
      | (Some(Any_t), Some(Boolean_t)) =>
        snd(lhs) := Synthetic(Boolean_t, []);
        Some(Resolved(Boolean_t));
      | (None, _)
      | (_, None) => None
      | _ => raise(OperatorTypeMismatch)
      }

    | Reference((_, typ)) =>
      switch (typ^) {
      | Unanalyzed => None
      | res => Some(res)
      }
    }
  )
  |%> (x => snd(promise) := x);
