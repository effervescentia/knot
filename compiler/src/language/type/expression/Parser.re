open Knot.Kore;
open Parse.Onyx;

module Keyword = Grammar.Keyword;
module Matchers = Grammar.Matchers;
module Symbol = Grammar.Symbol;
module TE = AST.TypeExpression;

let primitive_types =
  TE.[
    (Keyword.nil, Nil),
    (Keyword.boolean, Boolean),
    (Keyword.integer, Integer),
    (Keyword.float, Float),
    (Keyword.string, String),
    (Keyword.element, Element),
    (Keyword.style, Style),
  ];

let primitive: Grammar.Kore.type_expression_parser_t =
  choice(
    primitive_types |> List.map(((kwd, prim)) => kwd >|= Node.map(_ => prim)),
  );

let group =
    (parse_expr: Grammar.Kore.type_expression_parser_t)
    : Grammar.Kore.type_expression_parser_t =>
  parse_expr
  |> Matchers.between(Symbol.open_group, Symbol.close_group)
  >|= Node.map(TE.of_group);

let list =
    (parse_expr: Grammar.Kore.type_expression_parser_t)
    : Grammar.Kore.type_expression_parser_t =>
  parse_expr
  |> suffixed_by(
       Matchers.glyph("[]")
       >|= (
         (suffix, expr) =>
           Node.untyped(TE.of_list(expr), Node.join_ranges(expr, suffix))
       ),
     );

let struct_ =
    (parse_expr: Grammar.Kore.type_expression_parser_t)
    : Grammar.Kore.type_expression_parser_t =>
  Matchers.identifier(~prefix=Matchers.alpha)
  >>= (id => Symbol.colon >> parse_expr >|= Tuple.with_fst2(id))
  |> Matchers.comma_sep
  |> Matchers.between(Symbol.open_closure, Symbol.close_closure)
  /* TODO: sort the props here by property name */
  >|= Node.map(props => TE.of_struct(props));

let function_ =
    (parse_expr: Grammar.Kore.type_expression_parser_t)
    : Grammar.Kore.type_expression_parser_t =>
  parse_expr
  |> Matchers.comma_sep
  |> Matchers.between(Symbol.open_group, Symbol.close_group)
  >>= (
    args =>
      Matchers.glyph("->")
      >> parse_expr
      >|= (
        res =>
          Node.untyped(
            TE.of_function((fst(args), res)),
            Node.join_ranges(args, res),
          )
      )
  );

let identifier: Grammar.Kore.type_expression_parser_t =
  Matchers.identifier >|= Node.wrap(TE.of_id);

let dot_access = {
  let rec loop = expr =>
    Symbol.period
    >> Matchers.identifier
    >>= (
      prop =>
        loop(
          Node.untyped(
            (expr, prop) |> TE.of_dot_access,
            Node.get_range(prop),
          ),
        )
    )
    |> option(expr);

  loop;
};

/*
  each type expression has a precedence denoted by its suffix

  the parser with the highest precedence should be matched first
 */

/* element[], float[][][] */
let rec expr_0: Grammar.Kore.type_expression_parser_t =
  input => (list(expr_1))(input)

/* foo.bar */
and expr_1: Grammar.Kore.type_expression_parser_t =
  input => (expr_2 >>= dot_access)(input)

/* nil, (string), (integer, float) -> boolean, { foo: string } */
and expr_2: Grammar.Kore.type_expression_parser_t =
  input =>
    choice(
      [
        function_(expr_0),
        group(expr_0),
        struct_(expr_0),
        primitive,
        identifier,
      ],
      input,
    );

let expression = expr_0;
