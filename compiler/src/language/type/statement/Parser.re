open Knot.Kore;
open Parse.Kore;
open AST;

type type_module_statement_parser_t =
  ParseContext.t => Parse.Parser.t(TypeDefinition.module_statement_t);

let parse_type_variant = (ctx: ParseContext.t) =>
  KIdentifier.Parser.parse_raw(ctx)
  >>= (
    id =>
      KTypeExpression.Plugin.parse
      |> Matchers.comma_sep
      |> Matchers.between_parentheses
      |> option(id |> Node.map(_ => []))
      >|= Node.map(Tuple.with_fst2(id))
  );

let parse_declaration: type_module_statement_parser_t =
  ctx =>
    Parse.Util.define_statement(
      Matchers.keyword(Constants.Keyword.declare),
      KTypeExpression.Plugin.parse
      >|= (expr => (expr, expr |> Node.get_range |> Option.some)),
      (((id, _), (raw_expr, _)) as res) => {
        let type_ = raw_expr |> KTypeExpression.Plugin.analyze(ctx.symbols);

        ctx.symbols |> SymbolTable.declare_value(id, type_);

        TypeDefinition.of_exportaration(res);
      },
    );

let parse_enumerated: type_module_statement_parser_t =
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
                     (Type.Valid(`Function((args, enum_type))), true)
                   ),
                 ),
            ),
          );

        ctx.symbols |> SymbolTable.declare_type(id, enum_type);
        ctx.symbols |> SymbolTable.declare_value(id, value_type);

        TypeDefinition.of_enum(res);
      },
    );

let parse_type: type_module_statement_parser_t =
  ctx =>
    Parse.Util.define_statement(
      Matchers.keyword(Constants.Keyword.type_),
      KTypeExpression.Plugin.parse
      >|= (expr => (expr, expr |> Node.get_range |> Option.some)),
      (((id, _), (raw_expr, _)) as res) => {
        let type_ = raw_expr |> KTypeExpression.Plugin.analyze(ctx.symbols);

        ctx.symbols |> SymbolTable.declare_type(id, type_);

        TypeDefinition.of_type(res);
      },
    );

let parse: type_module_statement_parser_t =
  ctx =>
    choice([
      parse_declaration(ctx),
      parse_enumerated(ctx),
      parse_type(ctx),
    ])
    |> Matchers.terminated;
