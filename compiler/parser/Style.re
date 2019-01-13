open Core;

let protocol =
  Reference.expr
  >>= (
    refr =>
      Expression.expr
      |> M.comma_separated
      |> M.parentheses
      ==> (exprs => Protocol(refr, exprs))
  );
let preset = Reference.expr ==> (refr => Preset(refr));
let style_value = protocol <|> preset;

let rule =
  Reference.expr
  >>= (refr => M.colon >> style_value ==> (value => (refr, value)));

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
