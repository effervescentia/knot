open Core;

let rule =
  Reference.refr(Expression.expr)
  >>= (
    refr =>
      M.colon >> Reference.refr(Expression.expr) ==> (value => (refr, value))
  );

let class_ = M.period >> M.identifier ==> (s => ClassKey(s));
let id = M.number_sign >> M.identifier ==> (s => IdKey(s));

let rule_set =
  class_
  <|> id
  >>= (key => M.closure(rule |> M.terminated) ==> (rules => (key, rules)));

let rec decl = input =>
  (
    M.decl(M.style)
    >>= (
      name =>
        Parameter.params
        |= []
        >>= (
          params =>
            M.closure(rule_set) ==> (rules => StyleDecl(name, params, rules))
        )
    )
  )(
    input,
  );
