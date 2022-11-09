open Knot.Kore;
open Parse.Kore;
open AST.ParserTypes;

module ParseContext = AST.ParseContext;
module SymbolTable = AST.SymbolTable;
module Type = AST.Type;
module Util = Parse.Util;

type type_module_statement_parser_t =
  ParseContext.t => Parse.Parser.t(AST.TypeDefinition.module_statement_t);

let type_variant = (ctx: ParseContext.t) =>
  KIdentifier.Plugin.parse(ctx)
  >>= (
    id =>
      KTypeExpression.Plugin.parse
      |> Matchers.comma_sep
      |> Matchers.between_parentheses
      |> option(id |> Node.map(_ => []))
      >|= Node.map(Tuple.with_fst2(id))
  );

let declaration: type_module_statement_parser_t =
  ctx =>
    Util.define_statement(
      Matchers.keyword(Constants.Keyword.declare),
      KTypeExpression.Plugin.parse
      >|= (expr => (expr, expr |> Node.get_range |> Option.some)),
      (((id, _), (raw_expr, _)) as res) => {
        let type_ = raw_expr |> KTypeExpression.Plugin.analyze(ctx.symbols);

        ctx.symbols |> SymbolTable.declare_value(id, type_);

        AST.TypeDefinition.of_declaration(res);
      },
    );

let enumerated: type_module_statement_parser_t =
  ctx =>
    Util.define_statement(
      Matchers.keyword(Constants.Keyword.enum),
      type_variant(ctx)
      |> Matchers.vertical_bar_sep
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
                 List.map(fst % KTypeExpression.Plugin.analyze(ctx.symbols)),
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

        AST.TypeDefinition.of_enum(res);
      },
    );

let type_: type_module_statement_parser_t =
  ctx =>
    Util.define_statement(
      Matchers.keyword(Constants.Keyword.type_),
      KTypeExpression.Plugin.parse
      >|= (expr => (expr, expr |> Node.get_range |> Option.some)),
      (((id, _), (raw_expr, _)) as res) => {
        let type_ = raw_expr |> KTypeExpression.Plugin.analyze(ctx.symbols);

        ctx.symbols |> SymbolTable.declare_type(id, type_);

        AST.TypeDefinition.of_type(res);
      },
    );

let statement: type_module_statement_parser_t =
  ctx =>
    choice([declaration(ctx), enumerated(ctx), type_(ctx)])
    |> Matchers.terminated;
