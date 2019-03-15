open Core;
open NestedHashtbl;

let resolved = x => Some(ref(Resolved(x)));
let synthetic = x => ref(Synthetic(x, []));

let (=:=) = (x, y) => x^ := Synthetic(y, []);
let (=?>) = (x, y) =>
  switch (x, y) {
  | (_, _) when x == y => true
  | (Any_t, _)
  | (_, Any_t) => true
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
    | AddExpr((_, lhs), (_, rhs))
    | SubExpr((_, lhs), (_, rhs)) =>
      switch (lhs^ ^, rhs^ ^) {
      | (Resolved(Number_t), Resolved(Number_t)) => resolved(Number_t)
      | (Resolved(String_t), Resolved(String_t)) => resolved(String_t)
      | (Resolved(Number_t), Synthetic(typ, _)) when typ =?> Number_t =>
        rhs =:= Number_t;

        resolved(Number_t);
      | (Synthetic(typ, _), Resolved(Number_t)) when typ =?> Number_t =>
        lhs =:= Number_t;

        resolved(Number_t);
      | (Resolved(String_t), Synthetic(typ, _)) when typ =?> String_t =>
        rhs =:= String_t;

        resolved(String_t);
      | (Synthetic(typ, _), Resolved(String_t)) when typ =?> String_t =>
        lhs =:= String_t;

        resolved(Number_t);
      | _ => raise(OperatorTypeMismatch)
      }

    /* (number, number) => number */
    | MulExpr((_, lhs), (_, rhs))
    | DivExpr((_, lhs), (_, rhs)) =>
      switch (lhs^ ^, rhs^ ^) {
      | (Resolved(Number_t), Resolved(Number_t)) => resolved(Number_t)
      | (Resolved(Number_t), Synthetic(typ, [])) when typ =?> Number_t =>
        rhs =:= Number_t;

        resolved(Number_t);
      | (Synthetic(typ, []), Resolved(Number_t)) when typ =?> Number_t =>
        lhs =:= Number_t;

        resolved(Number_t);
      | _ => raise(OperatorTypeMismatch)
      }

    /* (number, number) => boolean */
    | LTExpr((_, lhs), (_, rhs))
    | GTExpr((_, lhs), (_, rhs))
    | LTEExpr((_, lhs), (_, rhs))
    | GTEExpr((_, lhs), (_, rhs)) =>
      switch (lhs^ ^, rhs^ ^) {
      | (Resolved(Number_t), Resolved(Number_t)) => resolved(Boolean_t)
      | (Resolved(Number_t), Synthetic(typ, [])) when typ =?> Number_t =>
        rhs =:= Number_t;

        resolved(Boolean_t);
      | (Synthetic(typ, []), Resolved(Number_t)) when typ =?> Number_t =>
        lhs =:= Number_t;

        resolved(Boolean_t);
      | _ => raise(OperatorTypeMismatch)
      }

    /* (boolean, boolean) => boolean */
    | AndExpr((_, lhs), (_, rhs))
    | OrExpr((_, lhs), (_, rhs)) =>
      switch (lhs^ ^, rhs^ ^) {
      | (Resolved(Boolean_t), Resolved(Boolean_t)) => resolved(Boolean_t)
      | (Resolved(Boolean_t), Synthetic(typ, [])) when typ =?> Boolean_t =>
        rhs =:= Boolean_t;

        resolved(Boolean_t);
      | (Synthetic(typ, []), Resolved(Boolean_t)) when typ =?> Boolean_t =>
        rhs =:= Boolean_t;

        resolved(Boolean_t);
      | _ => raise(OperatorTypeMismatch)
      }

    | Reference((_, {contents: typ})) =>
      switch (typ^) {
      | Unanalyzed => None
      | res => Some(typ)
      }
    }
  )
  |::> promise;
