open Core;
open NestedHashtbl;

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
  | x when fst(x) == y => true
  | (x, Declared(_)) when x =?? y => true
  | _ => false
  }
and operands_match_type = (lhs, rhs, x) =>
  operand_matches_type(lhs, x) && operand_matches_type(rhs, x);

let operands_match = (x, y) =>
  switch (x, y) {
  | ((x, _), (y, _)) when x == y => true
  | ((x, _), (y, _)) when x =?? y && y =?? x => true
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
      let lhs_ref = opt_type_ref(lhs);
      let rhs_ref = opt_type_ref(rhs);

      (
        switch (lhs_ref^, rhs_ref^) {
        /* ~~~ Numeric Addition ~~~ */

        /* both of acceptable types */
        | (lhs, rhs) when operands_match_type(lhs, rhs, Number_t) => Number_t

        /* rhs type mutable */
        | (
            (Number_t, Declared(false)),
            (Generic_t(gen_t), Declared(true) as t_ctx | Expected as t_ctx),
          )
            when allows_numeric(gen_t) =>
          rhs_ref := (Number_t, t_ctx);

          Number_t;

        /* lhs type mutable */
        | (
            (Generic_t(gen_t), Declared(true) as t_ctx | Expected as t_ctx),
            (Number_t, Declared(false)),
          )
            when allows_numeric(gen_t) =>
          lhs_ref := (Number_t, t_ctx);

          Number_t;

        /* ~~~ String Addition ~~~ */

        /* both of acceptable types */
        | (lhs, rhs) when operands_match_type(lhs, rhs, String_t) => String_t

        /* rhs type mutable */
        | (
            (String_t, Declared(false)),
            (Generic_t(gen_t), Declared(true) as t_ctx | Expected as t_ctx),
          )
            when allows_string(gen_t) =>
          rhs_ref := (String_t, t_ctx);

          String_t;

        /* lhs type mutable */
        | (
            (Generic_t(gen_t), Declared(true) as t_ctx | Expected as t_ctx),
            (String_t, Declared(false)),
          )
            when allows_string(gen_t) =>
          lhs_ref := (String_t, t_ctx);

          String_t;

        | _ => raise(OperatorTypeMismatch)
        }
      )
      |> declared;

    /* (number, number) => number */
    | MulExpr(lhs, rhs)
    | DivExpr(lhs, rhs) =>
      let lhs_ref = opt_type_ref(lhs);
      let rhs_ref = opt_type_ref(rhs);

      switch (lhs_ref^, rhs_ref^) {
      /* both of acceptable types */
      | (lhs, rhs) when operands_match_type(lhs, rhs, Number_t) => ()

      /* rhs type mutable */
      | (
          (Number_t, Declared(false)),
          (Generic_t(gen_t), Declared(true) as t_ctx | Expected as t_ctx),
        )
          when allows_numeric(gen_t) =>
        rhs_ref := (Number_t, t_ctx)

      /* lhs type mutable */
      | (
          (Generic_t(gen_t), Declared(true) as t_ctx | Expected as t_ctx),
          (Number_t, Declared(false)),
        )
          when allows_numeric(gen_t) =>
        lhs_ref := (Number_t, t_ctx)

      | _ => raise(OperatorTypeMismatch)
      };

      declared(Number_t);

    /* (number, number) => boolean */
    | LTExpr(lhs, rhs)
    | GTExpr(lhs, rhs)
    | LTEExpr(lhs, rhs)
    | GTEExpr(lhs, rhs) =>
      let lhs_ref = opt_type_ref(lhs);
      let rhs_ref = opt_type_ref(rhs);

      switch (lhs_ref^, rhs_ref^) {
      /* both of acceptable types */
      | (lhs, rhs) when operands_match_type(lhs, rhs, Number_t) => ()

      /* rhs type mutable */
      | (
          (Number_t, Declared(false)),
          (Generic_t(gen_t), Declared(true) as t_ctx | Expected as t_ctx),
        )
          when allows_numeric(gen_t) =>
        rhs_ref := (Number_t, t_ctx)

      /* lhs type mutable */
      | (
          (Generic_t(gen_t), Declared(true) as t_ctx | Expected as t_ctx),
          (Number_t, Declared(false)),
        )
          when allows_numeric(gen_t) =>
        lhs_ref := (Number_t, t_ctx)

      | _ => raise(OperatorTypeMismatch)
      };

      declared(Boolean_t);

    /* (boolean, boolean) => boolean */
    | AndExpr(lhs, rhs)
    | OrExpr(lhs, rhs) =>
      let lhs_ref = opt_type_ref(lhs);
      let rhs_ref = opt_type_ref(rhs);

      switch (lhs_ref^, rhs_ref^) {
      /* both of acceptable types */
      | (lhs, rhs) when operands_match_type(lhs, rhs, Boolean_t) => ()

      /* rhs type mutable */
      | (
          (Boolean_t, Declared(false)),
          (Generic_t(gen_t), Declared(true) as t_ctx | Expected as t_ctx),
        )
          when allows_boolean(gen_t) =>
        rhs_ref := (Boolean_t, t_ctx)

      /* lhs type mutable */
      | (
          (Boolean_t, Declared(false)),
          (Generic_t(gen_t), Declared(true) as t_ctx | Expected as t_ctx),
        )
          when allows_boolean(gen_t) =>
        lhs_ref := (Boolean_t, t_ctx)

      | _ => raise(OperatorTypeMismatch)
      };

      declared(Boolean_t);

    | TernaryExpr(predicate, if_expression, else_expression) =>
      let pred_ref = opt_type_ref(predicate);
      let pred_type = pred_ref^;
      let if_expr_ref = opt_type_ref(if_expression);
      let if_type = if_expr_ref^;
      let else_type = (opt_type_ref(else_expression))^;
      switch (pred_type) {
      | _ when operand_matches_type(pred_type, Boolean_t) => ()

      /* predicate type mutable */
      | (Generic_t(gen_t), Declared(true) as t_ctx | Expected as t_ctx)
          when allows_boolean(gen_t) =>
        pred_ref := (Boolean_t, t_ctx)

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
