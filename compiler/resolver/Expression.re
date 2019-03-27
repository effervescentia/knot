open Core;
open NestedHashtbl;

let resolve = ((value, promise)) =>
  (
    switch (value) {
    | BooleanLit(_) => Boolean_t
    | NumericLit(_) => Number_t
    | StringLit(_) => String_t
    | JSX(_) => JSX_t

    /* (number, number) => number */
    /* (string, string) => string */
    | AddExpr(lhs, rhs)
    | SubExpr(lhs, rhs) =>
      switch (opt_type_ref(lhs), opt_type_ref(rhs)) {
      /* ~~~ Numeric Addition ~~~ */
      | (Number_t, Number_t) => Number_t

      /* ~~~ String Addition ~~~ */
      | (String_t, String_t) => String_t

      | _ => raise(OperatorTypeMismatch)
      }

    /* (number, number) => number */
    | MulExpr(lhs, rhs)
    | DivExpr(lhs, rhs) =>
      switch (opt_type_ref(lhs), opt_type_ref(rhs)) {
      | (Number_t, Number_t) => Number_t

      | _ => raise(OperatorTypeMismatch)
      }

    /* (number, number) => boolean */
    | LTExpr(lhs, rhs)
    | GTExpr(lhs, rhs)
    | LTEExpr(lhs, rhs)
    | GTEExpr(lhs, rhs) =>
      switch (opt_type_ref(lhs), opt_type_ref(rhs)) {
      | (Number_t, Number_t) => Boolean_t

      | _ => raise(OperatorTypeMismatch)
      }

    /* (boolean, boolean) => boolean */
    | AndExpr(lhs, rhs)
    | OrExpr(lhs, rhs) =>
      switch (opt_type_ref(lhs), opt_type_ref(rhs)) {
      | (Boolean_t, Boolean_t) => Boolean_t

      | _ => raise(OperatorTypeMismatch)
      }

    | TernaryExpr(predicate, if_expression, else_expression) =>
      let if_type = opt_type_ref(if_expression);
      let else_type = opt_type_ref(else_expression);

      if (opt_type_ref(predicate) != Boolean_t) {
        raise(OperatorTypeMismatch);
      };

      if (if_type != else_type) {
        raise(OperatorTypeMismatch);
      };

      if_type;

    | Reference(refr) => opt_type_ref(refr)
    }
  )
  <:= promise;
