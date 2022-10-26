open Knot.Kore;
open Parse.Onyx;

module Character = Grammar.Character;
module Matchers = Grammar.Matchers;
module Symbol = Grammar.Symbol;

module Tag = {
  let open_ = Matchers.symbol(Constants.Character.open_chevron);
  let close = Matchers.symbol(Constants.Character.close_chevron);

  let self_close = Matchers.glyph(Constants.Glyph.self_close_tag);
  let open_end = Matchers.glyph(Constants.Glyph.open_end_tag);
};

module Fragment = {
  let open_ = Matchers.glyph(Constants.Glyph.open_fragment);
  let close = Matchers.glyph(Constants.Glyph.close_fragment);
};

let _attribute =
    (
      ~prefix=Matchers.alpha <|> Character.underscore,
      ctx: ParseContext.t,
      (parse_term, parse_expr): Grammar.Kore.expression_parsers_arg_t,
    ) =>
  Matchers.assign(
    Matchers.identifier(~prefix),
    parse_expr(ctx)
    |> Matchers.between(Symbol.open_group, Symbol.close_group)
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

let _self_closing = Tag.self_close >|= Node.map(() => []);

let rec _inner_ksx =
        (ctx: ParseContext.t, parsers: Grammar.Kore.expression_parsers_arg_t)
        : Grammar.Kore.jsx_parser_t =>
  /* do not attempt to simplify this `input` argument away or JSX parsing will loop forever */
  input =>
    (choice([fragment(ctx, parsers), tag(ctx, parsers)]) |> Matchers.lexeme)(
      input,
    )

and fragment =
    (ctx: ParseContext.t, parsers: Grammar.Kore.expression_parsers_arg_t)
    : Grammar.Kore.jsx_parser_t =>
  children(ctx, parsers)
  |> Matchers.between(Fragment.open_, Fragment.close)
  >|= Node.map(AST.Raw.of_frag)

and tag =
    (ctx: ParseContext.t, parsers: Grammar.Kore.expression_parsers_arg_t)
    : Grammar.Kore.jsx_parser_t =>
  Tag.open_
  >> Matchers.identifier
  >>= (
    id =>
      attributes(ctx, parsers)
      >>= (
        attrs =>
          Tag.close
          >> children(ctx, parsers)
          >>= (
            cs =>
              id
              |> fst
              |> Matchers.keyword
              |> Matchers.between(Tag.open_end, Tag.close)
              >|= Node.map(_ => cs)
          )
          <|> _self_closing
          >|= (
            cs =>
              Node.untyped(
                (id, attrs, fst(cs)) |> AST.Raw.of_tag,
                Node.join_ranges(id, cs),
              )
          )
      )
  )

and property_attribute =
    (ctx: ParseContext.t, parsers: Grammar.Kore.expression_parsers_arg_t)
    : Grammar.Kore.jsx_attribute_parser_t =>
  _attribute(ctx, parsers)
  >|= (
    ((name, value, range)) =>
      Node.untyped((name, value) |> AST.Raw.of_prop, range)
  )

and class_attribute =
    (ctx: ParseContext.t, parsers: Grammar.Kore.expression_parsers_arg_t)
    : Grammar.Kore.jsx_attribute_parser_t =>
  _attribute(~prefix=Character.period, ctx, parsers)
  >|= (
    ((name, value, range)) =>
      Node.untyped(
        (name |> Node.map(String.drop_left(1)), value)
        |> AST.Raw.of_jsx_class,
        range,
      )
  )

and id_attribute: Grammar.Kore.jsx_attribute_parser_t =
  Matchers.identifier(~prefix=Character.octothorpe)
  >|= Node.map(String.drop_left(1))
  >|= Node.wrap(AST.Raw.of_jsx_id)

and attributes =
    (ctx: ParseContext.t, parsers: Grammar.Kore.expression_parsers_arg_t)
    : Grammar.Kore.jsx_attribute_list_parser_t =>
  choice([
    property_attribute(ctx, parsers),
    class_attribute(ctx, parsers),
    id_attribute,
  ])
  |> many

and children =
    (ctx: ParseContext.t, parsers: Grammar.Kore.expression_parsers_arg_t)
    : Grammar.Kore.jsx_child_list_parser_t =>
  choice([node(ctx, parsers), inline_expr(ctx, parsers), text])
  |> Matchers.lexeme
  |> many

and text: Grammar.Kore.jsx_child_parser_t =
  none_of(Constants.Character.[open_brace, open_chevron])
  <~> (
    none_of(Constants.Character.[open_brace, open_chevron, close_chevron])
    |> many
  )
  >|= Input.join
  >|= Node.map(String.trim % AST.Raw.of_text)

and node =
    (ctx: ParseContext.t, parsers: Grammar.Kore.expression_parsers_arg_t)
    : Grammar.Kore.jsx_child_parser_t =>
  _inner_ksx(ctx, parsers) >|= Node.map(AST.Raw.of_node)

and inline_expr =
    (
      ctx: ParseContext.t,
      (_, parse_expr): Grammar.Kore.expression_parsers_arg_t,
    )
    : Grammar.Kore.jsx_child_parser_t =>
  parse_expr(ctx)
  |> Matchers.between(Symbol.open_inline_expr, Symbol.close_inline_expr)
  >|= Node.map(AST.Raw.of_inline_expr);

let ksx =
    (ctx: ParseContext.t, parsers: Grammar.Kore.expression_parsers_arg_t)
    : Grammar.Kore.expression_parser_t =>
  _inner_ksx(ctx, parsers) >|= Node.map(AST.Raw.of_jsx);
