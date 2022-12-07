open Knot.Kore;
open Parse.Kore;
open AST;

module Character = Constants.Character;
module Glyph = Constants.Glyph;

module Tag = {
  let open_ = Matchers.symbol(Character.open_chevron);
  let close = Matchers.symbol(Character.close_chevron);

  let self_close = Matchers.glyph(Glyph.self_close_tag);
  let open_end = Matchers.glyph(Glyph.open_end_tag);
};

module Fragment = {
  let open_ = Matchers.glyph(Glyph.open_fragment);
  let close = Matchers.glyph(Glyph.close_fragment);
};

type expression_parsers_arg_t = (
  /* parses a "term" */
  Framework.contextual_expression_parser_t,
  /* parses an "expression" */
  Framework.contextual_expression_parser_t,
);

type jsx_parser_t = Parse.Parser.t(Node.t(Raw.jsx_t, unit));

type jsx_attribute_parser_t = Parse.Parser.t(Raw.jsx_attribute_t);
type jsx_attribute_list_parser_t = Parse.Parser.t(list(Raw.jsx_attribute_t));

type jsx_child_parser_t = Parse.Parser.t(Raw.jsx_child_t);
type jsx_child_list_parser_t = Parse.Parser.t(list(Raw.jsx_child_t));

let _parse_attribute =
    (
      ~prefix=Matchers.alpha <|> Matchers.underscore,
      ctx: ParseContext.t,
      (parse_term, parse_expr): expression_parsers_arg_t,
    ) =>
  Matchers.assign(
    Matchers.identifier(~prefix),
    parse_expr(ctx)
    |> Matchers.between_parentheses
    >|= (
      ((expr, _) as expr_group) =>
        Node.map_range(_ => Node.get_range(expr_group), expr)
    )
    <|> parse_term(ctx),
  )
  >|= (
    ((name, value)) => (name, Some(value), Node.join_ranges(name, value))
  )
  <|> (
    Matchers.identifier(~prefix) >|= (id => (id, None, Node.get_range(id)))
  );

let _parse_self_closing = Tag.self_close >|= Node.map(() => []);

let rec _parse_inner_ksx =
        (ctx: ParseContext.t, parsers: expression_parsers_arg_t): jsx_parser_t =>
  /* do not attempt to simplify this `input` argument away or JSX parsing will loop forever */
  input =>
    (
      choice([parse_fragment(ctx, parsers), parse_tag(ctx, parsers)])
      |> Matchers.lexeme
    )(
      input,
    )

and parse_fragment =
    (ctx: ParseContext.t, parsers: expression_parsers_arg_t): jsx_parser_t =>
  parse_children(ctx, parsers)
  |> Matchers.between(Fragment.open_, Fragment.close)
  >|= Node.map(Raw.of_frag)

and parse_tag =
    (ctx: ParseContext.t, parsers: expression_parsers_arg_t): jsx_parser_t =>
  Tag.open_
  >> Matchers.identifier
  >>= (
    id =>
      parse_property_attribute(ctx, parsers)
      |> many
      >>= (
        attrs =>
          Tag.close
          >> parse_children(ctx, parsers)
          >>= (
            cs =>
              cs
              <$| (
                id
                |> fst
                |> Matchers.keyword
                |> Matchers.between(Tag.open_end, Tag.close)
              )
          )
          <|> _parse_self_closing
          >|= (
            cs =>
              Node.untyped(
                (id, [], attrs, fst(cs)) |> Raw.of_tag,
                Node.join_ranges(id, cs),
              )
          )
      )
  )

and parse_property_attribute =
    (ctx: ParseContext.t, parsers: expression_parsers_arg_t)
    : jsx_attribute_parser_t =>
  _parse_attribute(ctx, parsers)
  >|= (((name, value, range)) => Node.untyped((name, value), range))

and parse_children =
    (ctx: ParseContext.t, parsers: expression_parsers_arg_t)
    : jsx_child_list_parser_t =>
  choice([
    parse_node(ctx, parsers),
    parse_inline_expr(ctx, parsers),
    parse_text,
  ])
  |> Matchers.lexeme
  |> many

and parse_text: jsx_child_parser_t =
  none_of(. Character.[open_brace, open_chevron])
  <~> (
    none_of(. Character.[open_brace, open_chevron, close_chevron]) |> many
  )
  >|= Input.join
  >|= Node.map(String.trim % Raw.of_text)

and parse_node =
    (ctx: ParseContext.t, parsers: expression_parsers_arg_t)
    : jsx_child_parser_t =>
  _parse_inner_ksx(ctx, parsers) >|= Node.map(Raw.of_node)

and parse_inline_expr =
    (ctx: ParseContext.t, (_, parse_expr): expression_parsers_arg_t)
    : jsx_child_parser_t =>
  parse_expr(ctx) |> Matchers.between_braces >|= Node.map(Raw.of_inline_expr);

let parse =
    ((ctx: ParseContext.t, parsers: expression_parsers_arg_t))
    : Framework.expression_parser_t =>
  _parse_inner_ksx(ctx, parsers) >|= Node.map(Raw.of_jsx);
