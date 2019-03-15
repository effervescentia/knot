open Core;
open NestedHashtbl;

let resolve = ((value, promise)) =>
  (
    switch (value) {
    | BooleanLit(_) => Some(resolved(Boolean_t))
    | NumericLit(_) => Some(resolved(Number_t))
    | StringLit(_) => Some(resolved(String_t))
    | JSX(jsx) => Some(resolved(JSX_t))

    /* (number, number) => number */
    /* (string, string) => string */
    | AddExpr(lhs, rhs)
    | SubExpr(lhs, rhs) =>
      (
        switch (t_of(lhs), t_of(rhs)) {
        | (Number_t, Number_t) => Number_t
        | (String_t, String_t) => String_t
        | (Number_t, Generic_t(gen_t)) when allows_type(gen_t, Number_t) =>
          /* TODO: should only be able to set reference if Generic_t is also Synthetic */
          rhs =:= Number_t;

          Number_t;
        | (Number_t, Generic_t(gen_t)) when allows_type(gen_t, Number_t) =>
          rhs =:= Number_t;

          Number_t;
        | (Generic_t(gen_t), Number_t) when allows_type(gen_t, Number_t) =>
          lhs =:= Number_t;

          Number_t;
        | (String_t, Generic_t(gen_t)) when allows_type(gen_t, String_t) =>
          rhs =:= String_t;

          String_t;
        | (Generic_t(gen_t), String_t) when allows_type(gen_t, String_t) =>
          lhs =:= String_t;

          Number_t;
        | _ => raise(OperatorTypeMismatch)
        }
      )
      |> resolve_ref

    /* (number, number) => number */
    | MulExpr(lhs, rhs)
    | DivExpr(lhs, rhs) =>
      (
        switch (t_of(lhs), t_of(rhs)) {
        | (Number_t, Number_t) => Number_t
        | (Number_t, Generic_t(gen_t)) when allows_type(gen_t, Number_t) =>
          rhs =:= Number_t;

          Number_t;
        | (Generic_t(gen_t), Number_t) when allows_type(gen_t, Number_t) =>
          lhs =:= Number_t;

          Number_t;
        | _ => raise(OperatorTypeMismatch)
        }
      )
      |> resolve_ref

    /* (number, number) => boolean */
    | LTExpr(lhs, rhs)
    | GTExpr(lhs, rhs)
    | LTEExpr(lhs, rhs)
    | GTEExpr(lhs, rhs) =>
      (
        switch (t_of(lhs), t_of(rhs)) {
        | (Number_t, Number_t) => Boolean_t
        | (Number_t, Generic_t(gen_t)) when allows_type(gen_t, Number_t) =>
          rhs =:= Number_t;

          Boolean_t;
        | (Generic_t(gen_t), Number_t) when allows_type(gen_t, Number_t) =>
          lhs =:= Number_t;

          Boolean_t;
        | _ => raise(OperatorTypeMismatch)
        }
      )
      |> resolve_ref

    /* (boolean, boolean) => boolean */
    | AndExpr(lhs, rhs)
    | OrExpr(lhs, rhs) =>
      (
        switch (t_of(lhs), t_of(rhs)) {
        | (Boolean_t, Boolean_t) => Boolean_t
        | (Boolean_t, Generic_t(gen_t)) when allows_type(gen_t, Boolean_t) =>
          rhs =:= Boolean_t;

          Boolean_t;
        | (Generic_t(gen_t), Boolean_t) when allows_type(gen_t, Boolean_t) =>
          rhs =:= Boolean_t;

          Boolean_t;
        | _ => raise(OperatorTypeMismatch)
        }
      )
      |> resolve_ref

    | Reference((_, typ)) =>
      switch (typ^ ^) {
      | Unanalyzed => raise(InvalidTypeReference)
      | res => Some(typ^)
      }
    }
  )
  |::> promise;
