open Knot.Kore;
open Parse.Kore;
open AST.ParserTypes;

module Keyword = Constants.Keyword;

type type_expression_parser_t = Parse.Parser.t(AST.TypeExpression.t);

let primitive_types =
  AST.TypeExpression.[
    (Matchers.keyword(Keyword.nil), Nil),
    (Matchers.keyword(Keyword.boolean), Boolean),
    (Matchers.keyword(Keyword.integer), Integer),
    (Matchers.keyword(Keyword.float), Float),
    (Matchers.keyword(Keyword.string), String),
    (Matchers.keyword(Keyword.element), Element),
    (Matchers.keyword(Keyword.style), Style),
  ];

let primitive: type_expression_parser_t =
  choice(
    primitive_types |> List.map(((kwd, prim)) => kwd >|= Node.map(_ => prim)),
  );

let group = (parse_expr: type_expression_parser_t): type_expression_parser_t =>
  parse_expr
  |> Matchers.between_parentheses
  >|= Node.map(AST.TypeExpression.of_group);

let list = (parse_expr: type_expression_parser_t): type_expression_parser_t =>
  parse_expr
  |> suffixed_by(
       Matchers.glyph("[]")
       >|= (
         (suffix, expr) =>
           Node.untyped(
             AST.TypeExpression.of_list(expr),
             Node.join_ranges(expr, suffix),
           )
       ),
     );

let struct_ = (parse_expr: type_expression_parser_t): type_expression_parser_t =>
  Matchers.attribute(Matchers.identifier(~prefix=Matchers.alpha), parse_expr)
  |> Matchers.comma_sep
  |> Matchers.between_braces
  /* TODO: sort the props here by property name */
  >|= Node.map(props => AST.TypeExpression.of_struct(props));

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
            AST.TypeExpression.of_function((fst(args), res)),
            Node.join_ranges(args, res),
          )
      )
  );

let identifier: type_expression_parser_t =
  Matchers.identifier >|= Node.wrap(AST.TypeExpression.of_id);

let dot_access = {
  let rec loop = expr =>
    Matchers.period
    >> Matchers.identifier
    >>= (
      prop =>
        loop(
          Node.untyped(
            (expr, prop) |> AST.TypeExpression.of_dot_access,
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
