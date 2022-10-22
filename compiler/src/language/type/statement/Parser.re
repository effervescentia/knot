open Knot.Kore;
open Parse.Onyx;

module Glyph = Grammar.Glyph;
module Keyword = Grammar.Keyword;
module Matchers = Grammar.Matchers;
module Symbol = Grammar.Symbol;
module TE = AST.TypeExpression;
module TD = AST.TypeDefinition;
module Util = Grammar.Util;

let _type_variant = (ctx: ParseContext.t) =>
  KIdentifier.Plugin.parse(ctx)
  >>= (
    id =>
      KTypeExpression.Plugin.parse
      |> Matchers.comma_sep
      |> Matchers.between(Symbol.open_group, Symbol.close_group)
      |> option(id |> Node.map(_ => []))
      >|= Node.map(Tuple.with_fst2(id))
  );

let type_variant_list = (ctx: ParseContext.t) =>
  optional(Symbol.vertical_bar)
  >> (_type_variant(ctx) |> sep_by(Symbol.vertical_bar));

let declaration: Grammar.Kore.type_module_statement_parser_t =
  ctx =>
    Util.define_statement(
      Keyword.declare,
      KTypeExpression.Plugin.parse
      >|= (expr => (expr, expr |> Node.get_range |> Option.some)),
      (((id, _), (raw_expr, _)) as res) => {
        let type_ =
          raw_expr |> Analyze.Typing.eval_type_expression(ctx.symbols);

        ctx.symbols |> SymbolTable.declare_value(id, type_);

        TD.of_declaration(res);
      },
    );

let enumerated: Grammar.Kore.type_module_statement_parser_t =
  ctx =>
    Util.define_statement(
      Keyword.enum,
      type_variant_list(ctx)
      >|= (
        variants => {
          let variant_range = variants |> List.last |?> Node.get_range;

          (variants |> List.map(fst), variant_range);
        }
      ),
      (((id, _), raw_variants) as res) => {
        let variants =
          raw_variants
          |> List.map(
               Tuple.map_each2(
                 fst,
                 List.map(
                   fst % Analyze.Typing.eval_type_expression(ctx.symbols),
                 ),
               ),
             );
        let enum_type = Type.Valid(`Enumerated(variants));
        let value_type =
          Type.Valid(
            `Struct(
              variants
              |> List.map(
                   Tuple.map_snd2(args =>
                     Type.Valid(`Function((args, enum_type)))
                   ),
                 ),
            ),
          );

        ctx.symbols |> SymbolTable.declare_type(id, enum_type);
        ctx.symbols |> SymbolTable.declare_value(id, value_type);

        TD.of_enum(res);
      },
    );

let type_: Grammar.Kore.type_module_statement_parser_t =
  ctx =>
    Util.define_statement(
      Keyword.type_,
      KTypeExpression.Plugin.parse
      >|= (expr => (expr, expr |> Node.get_range |> Option.some)),
      (((id, _), (raw_expr, _)) as res) => {
        let type_ =
          raw_expr |> Analyze.Typing.eval_type_expression(ctx.symbols);

        ctx.symbols |> SymbolTable.declare_type(id, type_);

        TD.of_type(res);
      },
    );

let statement: Grammar.Kore.type_module_statement_parser_t =
  ctx =>
    choice([declaration(ctx), enumerated(ctx), type_(ctx)])
    |> Matchers.terminated;
