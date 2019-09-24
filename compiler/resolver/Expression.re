open Core;

let resolve = ((value, promise)) =>
  (
    switch (value) {
    | BooleanLit(_) => Boolean_t
    | NumericLit(_) => Number_t
    | StringLit(_) => String_t
    | JSX(_) => JSX_t

    | FunctionLit(params, exprs) =>
      let (param_types, return_type) =
        Function.resolve_callable(params, exprs);

      Function_t(param_types, return_type);

    /* (boolean) => boolean */
    | NegatedExpr(expr) =>
      switch (opt_type_ref(expr)) {
      | Boolean_t => Boolean_t
      | _ => raise(OperatorTypeMismatch)
      }

    /* (number, number) => number */
    /* (string, string) => string */
    | EqualsExpr(lhs, rhs) =>
      switch (opt_type_ref(lhs), opt_type_ref(rhs)) {
      /* ~~~ Numeric Equality ~~~ */
      | (Number_t, Number_t)
      /* ~~~ String Equality ~~~ */
      | (String_t, String_t)
      /* ~~~ Boolean Equality ~~~ */
      | (Boolean_t, Boolean_t) => Boolean_t
      /* ~~~ Other Equality ~~~ */
      | (lhs, rhs) when lhs == rhs => raise(NotImplemented)

      | _ => raise(OperatorTypeMismatch)
      }

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
