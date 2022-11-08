open Knot.Kore;
open Parse.Onyx;
open AST.ParserTypes;

module Matchers = Parse.Matchers;
module ParseContext = AST.ParseContext;
module Scope = AST.Scope;
module SymbolTable = AST.SymbolTable;
module Type = AST.Type;
module Util = AST.Util;

let view =
    (
      ctx: ParseContext.t,
      tag_export: AST.Raw.identifier_t => AST.Result.export_t,
    )
    : declaration_parser_t =>
  Matchers.keyword(Constants.Keyword.view)
  >>= Node.get_range
  % (
    start =>
      KIdentifier.Plugin.parse(ctx)
      >>= (
        id =>
          KExpression.Plugin.parse
          |> KLambda.Plugin.parse_with_mixins(ctx)
          >|= (
            ((props, mixins, res, range)) => {
              let scope = ctx |> Scope.of_parse_context(range);
              let props' =
                props
                |> List.map(
                     KLambda.Plugin.analyze_argument(
                       scope,
                       KExpression.Plugin.analyze,
                     ),
                   );

              props'
              |> List.iter(arg =>
                   scope
                   |> Scope.define(
                        AST.Expression.(fst(arg).name) |> fst,
                        Node.get_type(arg),
                      )
                   |> Option.iter(
                        Scope.report_type_err(scope, Node.get_range(arg)),
                      )
                 );

              let mixins' =
                mixins
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
                scope |> Scope.create_child(Node.get_range(res));
              let res' =
                res
                |> Analyzer.analyze_view_body(
                     res_scope,
                     KExpression.Plugin.analyze,
                   );

              let prop_types =
                props'
                |> List.map(
                     Tuple.split2(
                       fst % AST.Expression.(prop => fst(prop.name)),
                       Node.get_type,
                     ),
                   );
              let type_ =
                Type.Valid(`View((prop_types, Node.get_type(res'))));
              let export_id = tag_export(id);

              ctx.symbols
              |> SymbolTable.declare_value(
                   ~main=Util.is_main(export_id),
                   fst(id),
                   type_,
                 );

              let view =
                Node.typed(
                  (props', mixins', res') |> AST.Result.of_view,
                  type_,
                  range,
                );

              Node.untyped((export_id, view), Range.join(start, range));
            }
          )
      )
      |> Matchers.terminated
  );
