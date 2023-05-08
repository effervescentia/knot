open Kore;
open Parse.Kore;
open AST;

type type_module_statement_parser_t('ast) =
  ParseContext.t('ast) => Parse.Parser.t(Interface.node_t);

let parse_type_variant = (ctx: ParseContext.t('ast)) =>
  KIdentifier.Plugin.parse_raw(ctx)
  >>= (
    id =>
      TypeExpression.parse
      |> Matchers.comma_sep
      |> Matchers.between_parentheses
      |> option(id |> Node.map(_ => []))
      >|= Node.map(Tuple.with_fst2(id))
  );

let parse_declaration: type_module_statement_parser_t('ast) =
  ctx =>
    Parse.Util.define_statement(
      Matchers.keyword(Constants.Keyword.declare),
      TypeExpression.parse
      >|= (expr => (expr, expr |> Node.get_range |> Option.some)),
      (((id, _), (raw_expr, _)) as res) => {
        let type_ = raw_expr |> TypeExpression.analyze(ctx.symbols);

        ctx.symbols |> SymbolTable.declare_value(id, type_);

        Interface.of_declaration(res);
      },
    );

let parse_enumerated: type_module_statement_parser_t('ast) =
  ctx =>
    Parse.Util.define_statement(
      Matchers.keyword(Constants.Keyword.enum),
      parse_type_variant(ctx)
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
                 List.map(fst % TypeExpression.analyze(ctx.symbols)),
               ),
             );
        let enum_type = Type.Valid(Enumerated(variants));
        let value_type =
          Type.Valid(
            Object(
              variants
              |> List.map(
                   Tuple.map_snd2(args =>
                     (Type.Valid(Function(args, enum_type)), true)
                   ),
                 ),
            ),
          );

        ctx.symbols |> SymbolTable.declare_type(id, enum_type);
        ctx.symbols |> SymbolTable.declare_value(id, value_type);

        Interface.of_enumerated(res);
      },
    );

let parse_type: type_module_statement_parser_t('ast) =
  ctx =>
    Parse.Util.define_statement(
      Matchers.keyword(Constants.Keyword.type_),
      TypeExpression.parse
      >|= (expr => (expr, expr |> Node.get_range |> Option.some)),
      (((id, _), (raw_expr, _)) as res) => {
        let type_ = raw_expr |> TypeExpression.analyze(ctx.symbols);

        ctx.symbols |> SymbolTable.declare_type(id, type_);

        Interface.of_type(res);
      },
    );

let parse: type_module_statement_parser_t('ast) =
  ctx =>
    choice([
      parse_declaration(ctx),
      parse_enumerated(ctx),
      parse_type(ctx),
    ])
    |> Matchers.terminated;
