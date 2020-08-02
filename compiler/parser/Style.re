open Globals;

let rule =
  Reference.refr(Expression.expr)
  ==> no_ctx
  >>= (
    refr =>
      M.colon
      >> Reference.refr(Expression.expr)
      ==> no_ctx
      ==> (value => (refr, value))
  );

let class_ = M.period >> M.identifier ==> (s => ClassKey(s));

let rule_set =
  class_
  >>= (key => M.closure(rule |> M.terminated) ==> (rules => (key, rules)));

let decl = input =>
  (
    M.decl(M.style)
    >>= (
      name =>
        Property.list(Expression.expr)
        |= []
        >>= (
          params =>
            M.closure(rule_set)
            ==> (rules => (name, StyleDecl(params, rules)))
        )
    )
  )(
    input,
  );
