open Knot.Kore;
open Parse.Kore;

let parse: Interface.Plugin.parse_t('ast, 'expr) =
  (f, parse_expression) =>
    parse_expression |> Matchers.between_parentheses >|= Node.wrap(fst % f);
