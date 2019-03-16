open Core;
open NestedHashtbl;

let (=..=) = (x, y) => {
  x =.= y;

  y;
};

let allows_boolean = x => allows_type(x, Boolean_t);
let allows_numeric = x => allows_type(x, Number_t);
let allows_string = x => allows_type(x, String_t);

let operands_match = (lhs, rhs, x) =>
  switch (lhs, rhs) {
  | (Declared(lhs) | Inferred(lhs), Declared(rhs) | Inferred(rhs))
      when lhs == x && rhs == x =>
    true
  | (Declared(lhs), Declared(rhs)) when lhs =?? x && rhs =?? x => true
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
        switch ((t_ref(lhs))^, (t_ref(rhs))^) {
        /* ~~~ Numeric Addition ~~~ */

        /* both of acceptable types */
        | (lhs, rhs) when operands_match(lhs, rhs, Number_t) => Number_t

        /* rhs type inferred */
        | (Declared(Number_t), Inferred(Generic_t(gen_t)))
            when allows_numeric(gen_t) =>
          rhs =..= Number_t

        /* lhs type inferred */
        | (Inferred(Generic_t(gen_t)), Declared(Number_t))
            when allows_numeric(gen_t) =>
          lhs =..= Number_t

        /* ~~~ String Addition ~~~ */

        /* both of acceptable types */
        | (lhs, rhs) when operands_match(lhs, rhs, String_t) => String_t

        /* rhs type inferred */
        | (Declared(String_t), Inferred(Generic_t(gen_t)))
            when allows_string(gen_t) =>
          rhs =..= String_t

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
      switch ((t_ref(lhs))^, (t_ref(rhs))^) {
      /* both of acceptable types */
      | (lhs, rhs) when operands_match(lhs, rhs, Number_t) => ()

      /* rhs type inferred */
      | (Declared(Number_t), Inferred(Generic_t(gen_t)))
          when allows_numeric(gen_t) =>
        rhs =.= Number_t

      /* lhs type inferred */
      | (Inferred(Generic_t(gen_t)), Declared(Number_t))
          when allows_numeric(gen_t) =>
        lhs =.= Number_t

      | _ => raise(OperatorTypeMismatch)
      };

      declared(Number_t);

    /* (number, number) => boolean */
    | LTExpr(lhs, rhs)
    | GTExpr(lhs, rhs)
    | LTEExpr(lhs, rhs)
    | GTEExpr(lhs, rhs) =>
      switch ((t_ref(lhs))^, (t_ref(rhs))^) {
      /* both of acceptable types */
      | (lhs, rhs) when operands_match(lhs, rhs, Number_t) => ()

      /* rhs type inferred */
      | (Declared(Number_t), Inferred(Generic_t(gen_t)))
          when allows_numeric(gen_t) =>
        rhs =.= Number_t

      /* lhs type inferred */
      | (Inferred(Generic_t(gen_t)), Declared(Number_t))
          when allows_numeric(gen_t) =>
        lhs =.= Number_t

      | _ => raise(OperatorTypeMismatch)
      };

      declared(Boolean_t);

    /* (boolean, boolean) => boolean */
    | AndExpr(lhs, rhs)
    | OrExpr(lhs, rhs) =>
      switch ((t_ref(lhs))^, (t_ref(rhs))^) {
      /* both of acceptable types */
      | (lhs, rhs) when operands_match(lhs, rhs, Boolean_t) => ()

      /* rhs type inferred */
      | (Declared(Boolean_t), Inferred(Generic_t(gen_t)))
          when allows_boolean(gen_t) =>
        rhs =.= Boolean_t

      /* lhs type inferred */
      | (Inferred(Generic_t(gen_t)), Declared(Boolean_t))
          when allows_boolean(gen_t) =>
        rhs =.= Boolean_t

      | _ => raise(OperatorTypeMismatch)
      };

      declared(Boolean_t);

    | Reference(refr) =>
      let refr_ref = t_ref(refr);

      switch (refr_ref^) {
      | Unanalyzed => raise(InvalidTypeReference)
      | res => refr_ref
      };
    }
  )
  |:> promise;
