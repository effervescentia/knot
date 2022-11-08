open Knot.Kore;
open Parse.Onyx;
open AST.ParserTypes;

module Matchers = Parse.Matchers;
module TE = AST.TypeExpression;

type type_expression_parser_t = Parse.Parser.t(AST.TypeExpression.t);

let primitive_types =
  TE.[
    (Matchers.keyword(Constants.Keyword.nil), Nil),
    (Matchers.keyword(Constants.Keyword.boolean), Boolean),
    (Matchers.keyword(Constants.Keyword.integer), Integer),
    (Matchers.keyword(Constants.Keyword.float), Float),
    (Matchers.keyword(Constants.Keyword.string), String),
    (Matchers.keyword(Constants.Keyword.element), Element),
    (Matchers.keyword(Constants.Keyword.style), Style),
  ];

let primitive: type_expression_parser_t =
  choice(
    primitive_types |> List.map(((kwd, prim)) => kwd >|= Node.map(_ => prim)),
  );

let group = (parse_expr: type_expression_parser_t): type_expression_parser_t =>
  parse_expr |> Matchers.between_parentheses >|= Node.map(TE.of_group);

let list = (parse_expr: type_expression_parser_t): type_expression_parser_t =>
  parse_expr
  |> suffixed_by(
       Matchers.glyph("[]")
       >|= (
         (suffix, expr) =>
           Node.untyped(TE.of_list(expr), Node.join_ranges(expr, suffix))
       ),
     );

let struct_ = (parse_expr: type_expression_parser_t): type_expression_parser_t =>
  Matchers.attribute(Matchers.identifier(~prefix=Matchers.alpha), parse_expr)
  |> Matchers.comma_sep
  |> Matchers.between_braces
  /* TODO: sort the props here by property name */
  >|= Node.map(props => TE.of_struct(props));

let function_ =
    (parse_expr: type_expression_parser_t): type_expression_parser_t =>
  parse_expr
  |> Matchers.comma_sep
  |> Matchers.between_parentheses
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

let identifier: type_expression_parser_t =
  Matchers.identifier >|= Node.wrap(TE.of_id);

let dot_access = {
  let rec loop = expr =>
    Matchers.period
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
let rec expr_0: type_expression_parser_t = input => (list(expr_1))(input)

/* foo.bar */
and expr_1: type_expression_parser_t = input => (expr_2 >>= dot_access)(input)

/* nil, (string), (integer, float) -> boolean, { foo: string } */
and expr_2: type_expression_parser_t =
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
