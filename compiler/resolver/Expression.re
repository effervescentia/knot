open Core;
open NestedHashtbl;

let allows_type = (x, target) =>
  switch (x, target) {
  | (None, Number_t | String_t | Boolean_t | Nil_t) => true
  | _ => false
  };

let resolve = ((value, promise)) =>
  (
    switch (value) {
    | BooleanLit(_) => resolved(Boolean_t)
    | NumericLit(_) => resolved(Number_t)
    | StringLit(_) => resolved(String_t)
    | JSX(jsx) => resolved(JSX_t)

    /* (number, number) => number */
    /* (string, string) => string */
    | AddExpr((_, {contents: lhs}), (_, {contents: rhs}))
    | SubExpr((_, {contents: lhs}), (_, {contents: rhs})) =>
      switch (typeof(lhs^), typeof(rhs^)) {
      | (Number_t, Number_t) => resolved(Number_t)
      | (String_t, String_t) => resolved(String_t)
      | (Number_t, Generic_t(gen_t)) when allows_type(gen_t, Number_t) =>
        /* TODO: should only be able to set reference if Generic_t is also Synthetic */
        rhs := Resolved(Number_t);

        resolved(Number_t);
      | (Number_t, Generic_t(gen_t)) when allows_type(gen_t, Number_t) =>
        rhs := Resolved(Number_t);

        resolved(Number_t);
      | (Generic_t(gen_t), Number_t) when allows_type(gen_t, Number_t) =>
        lhs := Resolved(Number_t);

        resolved(Number_t);
      | (String_t, Generic_t(gen_t)) when allows_type(gen_t, String_t) =>
        rhs := Resolved(String_t);

        resolved(String_t);
      | (Generic_t(gen_t), String_t) when allows_type(gen_t, String_t) =>
        lhs := Resolved(String_t);

        resolved(Number_t);
      | _ => raise(OperatorTypeMismatch)
      }

    /* (number, number) => number */
    | MulExpr((_, {contents: lhs}), (_, {contents: rhs}))
    | DivExpr((_, {contents: lhs}), (_, {contents: rhs})) =>
      switch (typeof(lhs^), typeof(rhs^)) {
      | (Number_t, Number_t) => resolved(Number_t)
      | (Number_t, Generic_t(gen_t)) when allows_type(gen_t, Number_t) =>
        rhs := Resolved(Number_t);

        resolved(Number_t);
      | (Generic_t(gen_t), Number_t) when allows_type(gen_t, Number_t) =>
        lhs := Resolved(Number_t);

        resolved(Number_t);
      | _ => raise(OperatorTypeMismatch)
      }

    /* (number, number) => boolean */
    | LTExpr((_, {contents: lhs}), (_, {contents: rhs}))
    | GTExpr((_, {contents: lhs}), (_, {contents: rhs}))
    | LTEExpr((_, {contents: lhs}), (_, {contents: rhs}))
    | GTEExpr((_, {contents: lhs}), (_, {contents: rhs})) =>
      switch (typeof(lhs^), typeof(rhs^)) {
      | (Number_t, Number_t) => resolved(Boolean_t)
      | (Number_t, Generic_t(gen_t)) when allows_type(gen_t, Number_t) =>
        rhs := Resolved(Number_t);

        resolved(Boolean_t);
      | (Generic_t(gen_t), Number_t) when allows_type(gen_t, Number_t) =>
        lhs := Resolved(Number_t);

        resolved(Boolean_t);
      | _ => raise(OperatorTypeMismatch)
      }

    /* (boolean, boolean) => boolean */
    | AndExpr((_, {contents: lhs}), (_, {contents: rhs}))
    | OrExpr((_, {contents: lhs}), (_, {contents: rhs})) =>
      switch (typeof(lhs^), typeof(rhs^)) {
      | (Boolean_t, Boolean_t) => resolved(Boolean_t)
      | (Boolean_t, Generic_t(gen_t)) when allows_type(gen_t, Boolean_t) =>
        rhs := Resolved(Boolean_t);

        resolved(Boolean_t);
      | (Generic_t(gen_t), Boolean_t) when allows_type(gen_t, Boolean_t) =>
        rhs := Resolved(Boolean_t);

        resolved(Boolean_t);
      | _ => raise(OperatorTypeMismatch)
      }

    | Reference((_, {contents: typ})) =>
      switch (typ^) {
      | Unanalyzed => raise(InvalidTypeReference)
      | res => Some(typ)
      }
    }
  )
  |::> promise;
