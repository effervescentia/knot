open Kore;

module SemanticAnalyzer = Analyze.Semantic;

let parser = (ctx: ModuleContext.t, f): declaration_parser_t =>
  Keyword.view
  >>= N2.get_range
  % (
    start =>
      Identifier.parser(ctx)
      >>= (
        id =>
          Lambda.parser(ctx)
          >|= (
            ((raw_props, raw_res, range)) => {
              let scope = ctx |> Util.create_scope(range);
              let props =
                raw_props
                |> List.map(SemanticAnalyzer.analyze_argument(scope));

              props
              |> List.iter(arg =>
                   scope
                   |> S.define(A.(fst(arg).name) |> fst, N2.get_type(arg))
                   |> Option.iter(
                        S.report_type_err(scope, N2.get_range(arg)),
                      )
                 );

              let res_scope = scope |> S.create_child(N2.get_range(raw_res));
              let res =
                raw_res |> SemanticAnalyzer.analyze_view_body(res_scope);

              let prop_types =
                props
                |> List.map(
                     Tuple.split2(
                       fst
                       % A.(
                           prop =>
                             prop.name |> fst |> Reference.Identifier.to_string
                         ),
                       N2.get_type,
                     ),
                   );
              let type_ = T.Valid(`View((prop_types, N2.get_type(res))));
              let export_id = f(id);

              ctx
              |> ModuleContext.declare(
                   ~main=Util.is_main(export_id),
                   fst(id),
                   type_,
                 );

              let view = N2.typed((props, res) |> A.of_view, type_, range);

              N2.untyped((export_id, view), Range.join(start, range));
            }
          )
      )
      |> M.terminated
  );
