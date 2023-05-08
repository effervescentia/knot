open Knot.Kore;
open Parse.Kore;

let parse: Interface.Plugin.parse_t('ast, 'expr) =
  (f, (parse_term, parse_expression)) => {
    let rec loop = expression =>
      parse_expression
      |> Matchers.comma_sep
      |> Matchers.between_parentheses
      >>= (
        arguments =>
          loop(
            Node.raw(
              (expression, fst(arguments)) |> f,
              Node.join_ranges(expression, arguments),
            ),
          )
      )
      |> option(expression);

    parse_term >>= loop;
  };
