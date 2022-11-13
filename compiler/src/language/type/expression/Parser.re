open Knot.Kore;
open Parse.Kore;
open AST;

module Keyword = Constants.Keyword;

type type_expression_parser_t = Parse.Parser.t(TypeExpression.t);

let primitive: type_expression_parser_t =
  choice(
    TypeExpression.[
      Nil <$| Matchers.keyword(Keyword.nil),
      Boolean <$| Matchers.keyword(Keyword.boolean),
      Integer <$| Matchers.keyword(Keyword.integer),
      Float <$| Matchers.keyword(Keyword.float),
      String <$| Matchers.keyword(Keyword.string),
      Element <$| Matchers.keyword(Keyword.element),
      Style <$| Matchers.keyword(Keyword.style),
    ],
  );

let group = (parse_expr: type_expression_parser_t): type_expression_parser_t =>
  parse_expr
  |> Matchers.between_parentheses
  >|= Node.map(TypeExpression.of_group);

let list = (parse_expr: type_expression_parser_t): type_expression_parser_t =>
  parse_expr
  |> suffixed_by(
       Matchers.glyph("[]")
       >|= (
         (suffix, expr) =>
           Node.untyped(
             TypeExpression.of_list(expr),
             Node.join_ranges(expr, suffix),
           )
       ),
     );

let _struct_key = Matchers.identifier(~prefix=Matchers.alpha);
let _required_property = parse_expr =>
  Matchers.attribute(_struct_key, parse_expr >|= (x => (x, true)));
let _optional_property = parse_expr =>
  Matchers.binary_op(
    _struct_key,
    Matchers.glyph(Constants.Glyph.conditional),
    parse_expr >|= (x => (x, false)),
  );
let struct_ = (parse_expr: type_expression_parser_t): type_expression_parser_t =>
  _optional_property(parse_expr)
  <|> _required_property(parse_expr)
  |> Matchers.comma_sep
  |> Matchers.between_braces
  /* TODO: sort the props here by property name */
  >|= Node.map(props => TypeExpression.of_struct(props));

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
            TypeExpression.of_function((fst(args), res)),
            Node.join_ranges(args, res),
          )
      )
  );

let identifier: type_expression_parser_t =
  Matchers.identifier >|= Node.wrap(TypeExpression.of_id);

let dot_access = {
  let rec loop = expr =>
    Matchers.period
    >> Matchers.identifier
    >>= (
      prop =>
        loop(
          Node.untyped(
            (expr, prop) |> TypeExpression.of_dot_access,
            Node.get_range(prop),
          ),
        )
    )
    |> option(expr);

  loop;
};

let view = (parse_expr: type_expression_parser_t): type_expression_parser_t =>
  Matchers.keyword(Keyword.view)
  >>= (
    start =>
      parse_expr
      >>= (
        props =>
          Matchers.symbol(Constants.Character.comma)
          >> parse_expr
          >|= Tuple.with_fst2(props)
      )
      |> Matchers.between_parentheses
      >|= fst
      >|= (
        ((props, res)) =>
          Node.untyped(
            TypeExpression.of_view((props, res)),
            Node.join_ranges(start, res),
          )
      )
  );

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
        view(expr_0),
        function_(expr_0),
        group(expr_0),
        struct_(expr_0),
        primitive,
        identifier,
      ],
      input,
    );

let expression = expr_0;
