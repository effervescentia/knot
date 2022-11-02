open Knot.Kore;
open Parse.Onyx;
open AST.ParserTypes;

module Keyword = Parse.Keyword;
module Matchers = Parse.Matchers;
module ParseContext = AST.ParseContext;
module Symbol = Parse.Symbol;
module SymbolTable = AST.SymbolTable;
module Type = AST.Type;
module Util = AST.Util;

let enumerated = (ctx: ParseContext.t, f): declaration_parser_t =>
  Keyword.enum
  >|= Node.get_range
  >>= (
    start =>
      KTypeStatement.Plugin.parse_type_variant_list(ctx)
      |> Matchers.assign(KIdentifier.Plugin.parse(ctx))
      |> Matchers.terminated
      >|= (
        ((id, raw_variants)) => {
          let variants =
            raw_variants
            |> List.map(
                 fst
                 % Tuple.map_snd2(
                     List.map(type_expr =>
                       type_expr
                       |> Node.add_type(
                            type_expr
                            |> fst
                            |> KTypeExpression.Plugin.analyze(
                                 SymbolTable.create(),
                               ),
                          )
                     ),
                   ),
               );
          let type_ =
            Type.Valid(
              `Enumerated(
                variants
                |> List.map(Tuple.map_each2(fst, List.map(Node.get_type))),
              ),
            );
          let range =
            Range.join(
              start,
              raw_variants |> List.last |?> Node.get_range |?: start,
            );
          let enum = Node.typed(AST.Result.of_enum(variants), type_, range);
          let export_id = f(id);

          ctx.symbols
          |> SymbolTable.declare_value(
               ~main=Util.is_main(export_id),
               fst(id),
               type_,
             );

          Node.untyped((export_id, enum), range);
        }
      )
      |> Matchers.terminated
  );
