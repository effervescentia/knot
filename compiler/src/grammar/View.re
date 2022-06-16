open Kore;

module SemanticAnalyzer = Analyze.Semantic;

let parser = (ctx: ModuleContext.t, f): declaration_parser_t =>
  Keyword.view
  >>= NR.get_range
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
              |> List.iter(((arg, arg_type, arg_range)) =>
                   scope
                   |> S.define(A.(arg.name) |> NR.get_value, arg_type)
                   |> Option.iter(S.report_type_err(scope, arg_range))
                 );

              let res_scope = scope |> S.create_child(N.get_range(raw_res));
              let res =
                raw_res |> SemanticAnalyzer.analyze_view_body(res_scope);

              let prop_types =
                props
                |> List.map(
                     Tuple.split2(
                       N.get_value
                       % A.(
                           prop =>
                             prop.name
                             |> NR.get_value
                             |> Reference.Identifier.to_string
                         ),
                       N.get_type,
                     ),
                   );
              let type_ = T.Valid(`View((prop_types, N.get_type(res))));
              let export_id = f(id);

              ctx
              |> ModuleContext.declare(
                   ~main=Util.is_main(export_id),
                   NR.get_value(id),
                   type_,
                 );

              let view = N.create((props, res) |> A.of_view, type_, range);

              NR.create((export_id, view), Range.join(start, range));
            }
          )
      )
      |> M.terminated
  );
