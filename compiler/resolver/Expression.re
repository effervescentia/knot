open Core;
open NestedHashtbl;

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
      switch (lhs^, rhs^) {
      | (Resolved(Number_t), Resolved(Number_t)) => resolved(Number_t)
      | (Resolved(String_t), Resolved(String_t)) => resolved(String_t)
      | (Resolved(Number_t), Synthetic(rules))
          when V.allows_type(rules, Number_t) =>
        rhs := Resolved(Number_t);

        resolved(Number_t);
      | (Synthetic(rules), Resolved(Number_t))
          when V.allows_type(rules, Number_t) =>
        lhs := Resolved(Number_t);

        resolved(Number_t);
      | (Resolved(String_t), Synthetic(rules))
          when V.allows_type(rules, String_t) =>
        rhs := Resolved(String_t);

        resolved(String_t);
      | (Synthetic(rules), Resolved(String_t))
          when V.allows_type(rules, String_t) =>
        lhs := Resolved(String_t);

        resolved(Number_t);
      | _ => raise(OperatorTypeMismatch)
      }

    /* (number, number) => number */
    | MulExpr((_, {contents: lhs}), (_, {contents: rhs}))
    | DivExpr((_, {contents: lhs}), (_, {contents: rhs})) =>
      switch (lhs^, rhs^) {
      | (Resolved(Number_t), Resolved(Number_t)) => resolved(Number_t)
      | (Resolved(Number_t), Synthetic(rules))
          when V.allows_type(rules, Number_t) =>
        rhs := Resolved(Number_t);

        resolved(Number_t);
      | (Synthetic(rules), Resolved(Number_t))
          when V.allows_type(rules, Number_t) =>
        lhs := Resolved(Number_t);

        resolved(Number_t);
      | _ => raise(OperatorTypeMismatch)
      }

    /* (number, number) => boolean */
    | LTExpr((_, {contents: lhs}), (_, {contents: rhs}))
    | GTExpr((_, {contents: lhs}), (_, {contents: rhs}))
    | LTEExpr((_, {contents: lhs}), (_, {contents: rhs}))
    | GTEExpr((_, {contents: lhs}), (_, {contents: rhs})) =>
      switch (lhs^, rhs^) {
      | (Resolved(Number_t), Resolved(Number_t)) => resolved(Boolean_t)
      | (Resolved(Number_t), Synthetic(rules))
          when V.allows_type(rules, Number_t) =>
        rhs := Resolved(Number_t);

        resolved(Boolean_t);
      | (Synthetic(rules), Resolved(Number_t))
          when V.allows_type(rules, Number_t) =>
        lhs := Resolved(Number_t);

        resolved(Boolean_t);
      | _ => raise(OperatorTypeMismatch)
      }

    /* (boolean, boolean) => boolean */
    | AndExpr((_, {contents: lhs}), (_, {contents: rhs}))
    | OrExpr((_, {contents: lhs}), (_, {contents: rhs})) =>
      switch (lhs^, rhs^) {
      | (Resolved(Boolean_t), Resolved(Boolean_t)) => resolved(Boolean_t)
      | (Resolved(Boolean_t), Synthetic(rules))
          when V.allows_type(rules, Boolean_t) =>
        rhs := Resolved(Boolean_t);

        resolved(Boolean_t);
      | (Synthetic(rules), Resolved(Boolean_t))
          when V.allows_type(rules, Boolean_t) =>
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
