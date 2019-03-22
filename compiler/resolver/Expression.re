open Core;
open NestedHashtbl;

let (=..=) = ((_, x), y) => {
  x =.= y;

  y;
};
let (=**=) = ((_, x), y) => {
  x =*= y;

  y;
};

let allows_type = (x, target) =>
  switch (x, target) {
  | (None, Number_t | String_t | Boolean_t | Nil_t) => true
  | _ => false
  };

let allows_boolean = x => allows_type(x, Boolean_t);
let allows_numeric = x => allows_type(x, Number_t);
let allows_string = x => allows_type(x, String_t);

let rec operand_matches_type = (x, y) =>
  switch (x) {
  | x when typeof(x) == y => true
  | Declared(x) when x =?? y => true
  | _ => false
  }
and operands_match_type = (lhs, rhs, x) =>
  operand_matches_type(lhs, x) && operand_matches_type(rhs, x);

let operands_match = (x, y) =>
  switch (x, y) {
  | (x, y) when typeof(x) == typeof(y) => true
  | (x, y) when typeof(x) =?? typeof(y) && typeof(y) =?? typeof(x) => true
  | _ => false
  };

let resolve = ((value, promise)) =>
  (
    switch (value) {
    | BooleanLit(_) => declared(Boolean_t)
    | NumericLit(_) => declared(Number_t)
    | StringLit(_) => declared(String_t)
    | JSX(jsx) => declared(JSX_t)

    /* (number, number) => number */
    /* (string, string) => string */
    | AddExpr(lhs, rhs)
    | SubExpr(lhs, rhs) =>
      (
        switch ((opt_type_ref(lhs))^, (opt_type_ref(rhs))^) {
        /* ~~~ Numeric Addition ~~~ */

        /* both of acceptable types */
        | (lhs, rhs) when operands_match_type(lhs, rhs, Number_t) => Number_t

        /* rhs type defined */
        | (Declared(Number_t), Defined(Generic_t(gen_t)))
            when allows_numeric(gen_t) =>
          rhs =**= Number_t
        /* rhs type inferred */
        | (Declared(Number_t), Inferred(Generic_t(gen_t)))
            when allows_numeric(gen_t) =>
          rhs =..= Number_t

        /* lhs type defined */
        | (Defined(Generic_t(gen_t)), Declared(Number_t))
            when allows_numeric(gen_t) =>
          lhs =**= Number_t
        /* lhs type inferred */
        | (Inferred(Generic_t(gen_t)), Declared(Number_t))
            when allows_numeric(gen_t) =>
          lhs =..= Number_t

        /* ~~~ String Addition ~~~ */

        /* both of acceptable types */
        | (lhs, rhs) when operands_match_type(lhs, rhs, String_t) => String_t

        /* rhs type defined */
        | (Declared(String_t), Defined(Generic_t(gen_t)))
            when allows_string(gen_t) =>
          rhs =**= String_t
        /* rhs type inferred */
        | (Declared(String_t), Inferred(Generic_t(gen_t)))
            when allows_string(gen_t) =>
          rhs =..= String_t

        /* lhs type defined */
        | (Defined(Generic_t(gen_t)), Declared(String_t))
            when allows_string(gen_t) =>
          lhs =**= String_t
        /* lhs type inferred */
        | (Inferred(Generic_t(gen_t)), Declared(String_t))
            when allows_string(gen_t) =>
          lhs =..= String_t

        | _ => raise(OperatorTypeMismatch)
        }
      )
      |> declared

    /* (number, number) => number */
    | MulExpr(lhs, rhs)
    | DivExpr(lhs, rhs) =>
      switch ((opt_type_ref(lhs))^, (opt_type_ref(rhs))^) {
      /* both of acceptable types */
      | (lhs, rhs) when operands_match_type(lhs, rhs, Number_t) => ()

      /* rhs type defined */
      | (Declared(Number_t), Defined(Generic_t(gen_t)))
          when allows_numeric(gen_t) =>
        snd(rhs) =*= Number_t
      /* rhs type inferred */
      | (Declared(Number_t), Inferred(Generic_t(gen_t)))
          when allows_numeric(gen_t) =>
        snd(rhs) =.= Number_t

      /* lhs type defined */
      | (Defined(Generic_t(gen_t)), Declared(Number_t))
          when allows_numeric(gen_t) =>
        snd(lhs) =*= Number_t
      /* lhs type inferred */
      | (Inferred(Generic_t(gen_t)), Declared(Number_t))
          when allows_numeric(gen_t) =>
        snd(lhs) =.= Number_t

      | _ => raise(OperatorTypeMismatch)
      };

      declared(Number_t);

    /* (number, number) => boolean */
    | LTExpr(lhs, rhs)
    | GTExpr(lhs, rhs)
    | LTEExpr(lhs, rhs)
    | GTEExpr(lhs, rhs) =>
      switch ((opt_type_ref(lhs))^, (opt_type_ref(rhs))^) {
      /* both of acceptable types */
      | (lhs, rhs) when operands_match_type(lhs, rhs, Number_t) => ()

      /* rhs type defined */
      | (Declared(Number_t), Defined(Generic_t(gen_t)))
          when allows_numeric(gen_t) =>
        snd(rhs) =*= Number_t
      /* rhs type inferred */
      | (Declared(Number_t), Inferred(Generic_t(gen_t)))
          when allows_numeric(gen_t) =>
        snd(rhs) =.= Number_t

      /* lhs type defined */
      | (Defined(Generic_t(gen_t)), Declared(Number_t))
          when allows_numeric(gen_t) =>
        snd(lhs) =*= Number_t
      /* lhs type inferred */
      | (Inferred(Generic_t(gen_t)), Declared(Number_t))
          when allows_numeric(gen_t) =>
        snd(lhs) =.= Number_t

      | _ => raise(OperatorTypeMismatch)
      };

      declared(Boolean_t);

    /* (boolean, boolean) => boolean */
    | AndExpr(lhs, rhs)
    | OrExpr(lhs, rhs) =>
      switch ((opt_type_ref(lhs))^, (opt_type_ref(rhs))^) {
      /* both of acceptable types */
      | (lhs, rhs) when operands_match_type(lhs, rhs, Boolean_t) => ()

      /* rhs type defined */
      | (Declared(Boolean_t), Defined(Generic_t(gen_t)))
          when allows_boolean(gen_t) =>
        snd(rhs) =*= Boolean_t
      /* rhs type inferred */
      | (Declared(Boolean_t), Inferred(Generic_t(gen_t)))
          when allows_boolean(gen_t) =>
        snd(rhs) =.= Boolean_t

      /* lhs type defined */
      | (Defined(Generic_t(gen_t)), Declared(Boolean_t))
          when allows_boolean(gen_t) =>
        snd(lhs) =*= Boolean_t
      /* lhs type inferred */
      | (Inferred(Generic_t(gen_t)), Declared(Boolean_t))
          when allows_boolean(gen_t) =>
        snd(lhs) =.= Boolean_t

      | _ => raise(OperatorTypeMismatch)
      };

      declared(Boolean_t);

    | TernaryExpr(predicate, if_expression, else_expression) =>
      let pred_type = (opt_type_ref(predicate))^;
      let if_expr_ref = opt_type_ref(if_expression);
      let if_type = if_expr_ref^;
      let else_type = (opt_type_ref(else_expression))^;
      switch (pred_type) {
      | _ when operand_matches_type(pred_type, Boolean_t) => ()

      /* predicate type defined */
      | Defined(Generic_t(gen_t)) when allows_boolean(gen_t) =>
        snd(predicate) =*= Boolean_t

      /* predicate type inferred */
      | Inferred(Generic_t(gen_t)) when allows_boolean(gen_t) =>
        snd(predicate) =.= Boolean_t

      | _ => raise(OperatorTypeMismatch)
      };

      switch (if_type, else_type) {
      | _ when operands_match(if_type, else_type) => ()

      /* TODO: maybe have the if_type infer the else_type when it can be inferred */

      | _ => raise(OperatorTypeMismatch)
      };

      if_expr_ref;

    | Reference(refr) => opt_type_ref(refr)
    }
  )
  <:= promise;
