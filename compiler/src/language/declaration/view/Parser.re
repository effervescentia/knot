open Knot.Kore;
open Parse.Kore;
open AST;

let __children_key = "children";
let __implicit_children_key = "$children";

let parse =
    ((ctx: ParseContext.t, export: Module.export_t))
    : Framework.declaration_parser_t =>
  Matchers.keyword(Constants.Keyword.view)
  >>= Node.get_range
  % (
    start =>
      KIdentifier.Parser.parse_raw(ctx)
      >>= (
        id =>
          KExpression.Plugin.parse
          |> KLambda.Parser.parse_lambda_with_mixins(ctx)
          >|= (
            ((props, mixins, res, range)) => {
              let scope = ctx |> Scope.of_parse_context(range);
              let props' =
                props
                |> List.map(
                     KLambda.Analyzer.analyze_argument(
                       scope,
                       KExpression.Plugin.analyze,
                     ),
                   );

              scope
              |> Scope.define(__implicit_children_key, Valid(`Element))
              |> ignore;

              props'
              |> List.iter(arg =>
                   scope
                   |> Scope.define(
                        arg |> fst |> Tuple.fst3 |> fst,
                        Node.get_type(arg),
                      )
                   |> Option.iter(
                        Scope.report_type_err(scope, Node.get_range(arg)),
                      )
                 );

              if (props'
                  |> List.exists((((name, _, _), _)) =>
                       fst(name) == __children_key
                     )) {
                let children_type =
                  scope
                  |> Scope.lookup(__children_key)
                  |> Option.value(~default=Ok(Type.Invalid(NotInferrable)))
                  |> Stdlib.Result.value(
                       ~default=Type.Invalid(NotInferrable),
                     );

                scope
                |> Scope.(
                     add_handler(__implicit_children_key, Lookup, () =>
                       Type.MustUseExplicitChildren(children_type)
                       |> Option.some
                     )
                   );
              };

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
                |> List.map(((((name, _), _, default), _) as node) =>
                     (
                       name,
                       node
                       |> Node.get_type
                       |> Tuple.with_snd2(Option.is_none(default)),
                     )
                   );
              let type_ =
                Type.Valid(`View((prop_types, Node.get_type(res'))));

              ctx.symbols
              |> SymbolTable.declare_value(
                   ~main=Util.is_main(export),
                   fst(id),
                   type_,
                 );

              let view =
                Node.typed(
                  (props', mixins', res') |> Result.of_view,
                  type_,
                  range,
                );

              Node.untyped((id, view), Range.join(start, range));
            }
          )
      )
      |> Matchers.terminated
  );
