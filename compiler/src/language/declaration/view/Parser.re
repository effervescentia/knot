open Knot.Kore;
open Parse.Onyx;

module Keyword = Grammar.Keyword;
module Matchers = Grammar.Matchers;
module SemanticAnalyzer = Analyze.Semantic;
module Util = Grammar.Util;

let view =
    (
      ctx: ParseContext.t,
      tag_export: AST.Raw.identifier_t => AST.export_t,
      parse_expression: Grammar.Kore.contextual_expression_parser_t,
    )
    : Grammar.Kore.declaration_parser_t =>
  Keyword.view
  >>= Node.get_range
  % (
    start =>
      KIdentifier.Plugin.parse(ctx)
      >>= (
        id =>
          parse_expression
          |> KLambda.Plugin.parse_with_mixins(ctx)
          >|= (
            ((raw_props, raw_mixins, raw_res, range)) => {
              let scope = ctx |> Scope.of_parse_context(range);
              let props =
                raw_props
                |> List.map(SemanticAnalyzer.analyze_argument(scope));

              props
              |> List.iter(arg =>
                   scope
                   |> Scope.define(
                        AST.(fst(arg).name) |> fst,
                        Node.get_type(arg),
                      )
                   |> Option.iter(
                        Scope.report_type_err(scope, Node.get_range(arg)),
                      )
                 );

              let mixins =
                raw_mixins
                |> List.map(mixin => {
                     let mixin_type =
                       ctx.symbols |> SymbolTable.resolve_value(fst(mixin));

                     switch (mixin_type) {
                     /* TODO: add state mixin support */

                     | Some(type_) =>
                       ctx
                       |> ParseContext.report(
                            TypeError(InvalidViewMixin(type_)),
                            Node.get_range(mixin),
                          )

                     | None =>
                       ctx
                       |> ParseContext.report(
                            TypeError(NotFound(fst(mixin))),
                            Node.get_range(mixin),
                          )
                     };

                     mixin
                     |> Node.add_type(mixin_type |?: Invalid(NotInferrable));
                   });

              let res_scope =
                scope |> Scope.create_child(Node.get_range(raw_res));
              let res =
                raw_res |> SemanticAnalyzer.analyze_view_body(res_scope);

              let prop_types =
                props
                |> List.map(
                     Tuple.split2(
                       fst % AST.(prop => fst(prop.name)),
                       Node.get_type,
                     ),
                   );
              let type_ =
                Type.Valid(`View((prop_types, Node.get_type(res))));
              let export_id = tag_export(id);

              ctx.symbols
              |> SymbolTable.declare_value(
                   ~main=Util.is_main(export_id),
                   fst(id),
                   type_,
                 );

              let view =
                Node.typed(
                  (props, mixins, res) |> AST.of_view,
                  type_,
                  range,
                );

              Node.untyped((export_id, view), Range.join(start, range));
            }
          )
      )
      |> Matchers.terminated
  );
