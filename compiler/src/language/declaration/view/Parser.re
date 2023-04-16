open Kore;
open Parse.Kore;
open AST;

module ExportKind = KModuleStatement.Interface.ExportKind;

let __children_key = "children";
let __implicit_children_key = "$children";

let parse: Interface.Plugin.parse_t('ast) =
  (is_main, ctx) =>
    Matchers.keyword(Constants.Keyword.view)
    >>= Node.get_range
    % (
      start =>
        KIdentifier.Plugin.parse_raw(ctx)
        >>= (
          name =>
            Expression.parse
            |> Lambda.parse_lambda_with_mixins(ctx)
            >|= (
              ((parameters, mixins, body, lambda_range)) => {
                let scope = ctx |> Scope.of_parse_context(lambda_range);
                let parameters' =
                  parameters
                  |> Lambda.analyze_parameter_list(Expression.analyze, scope);

                scope
                |> Scope.define(__implicit_children_key, Valid(Element))
                |> ignore;

                parameters'
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

                if (parameters'
                    |> List.exists((((name, _, _), _)) =>
                         fst(name) == __children_key
                       )) {
                  let children_type =
                    scope
                    |> Scope.lookup(__children_key)
                    |> Option.value(
                         ~default=Ok(Type.Invalid(NotInferrable)),
                       )
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
                       |> Node.add_type(
                            mixin_type |?: Invalid(NotInferrable),
                          );
                     });

                let body_scope =
                  scope |> Scope.create_child(Node.get_range(body));
                let (body', body_type) =
                  body |> Analyzer.analyze_view_body(body_scope);

                let parameter_types =
                  parameters'
                  |> List.map(((((name, _), _, default), _) as node) =>
                       (
                         name,
                         node
                         |> Node.get_type
                         |> Tuple.with_snd2(Option.is_none(default)),
                       )
                     );
                let type_ =
                  Type.Valid(View(parameter_types, Node.get_type(body')));

                ctx.symbols
                |> SymbolTable.declare_value(~main=is_main, fst(name), type_);

                let result =
                  Node.typed(
                    (parameters', mixins', body'),
                    type_,
                    lambda_range,
                  );
                let range = Range.join(start, lambda_range);

                Node.raw((name, result), range);
              }
            )
        )
        |> Matchers.terminated
    );
