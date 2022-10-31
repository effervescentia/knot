open Kore;

type severity =
  | Error
  | Warning
  | Information
  | Hint;

let severity =
  fun
  | Error => 1
  | Warning => 2
  | Information => 3
  | Hint => 4;

let method_key = "textDocument/publishDiagnostics";

let notification = (uri: string, errs) =>
  `Assoc([
    ("uri", `String(uri)),
    (
      "diagnostics",
      `List(
        errs
        |> List.map(((err, range)) =>
             `Assoc([
               ("range", Serialize.range(range)),
               ("severity", `Int(severity(Error))),
               ("source", `String(Target.knot)),
               ("message", `String(err |> ~@Knot.Error.pp_parse_err)),
             ])
           ),
      ),
    ),
  ]);

let send =
    ({server, _}: Runtime.t, source_dir: string, errs: list(compile_err)) => {
  let grouped_errs = Hashtbl.create(1);

  errs
  |> List.iter(
       fun
       | ParseError(err, namespace, range) =>
         Hashtbl.replace(
           grouped_errs,
           namespace,
           (Hashtbl.find_opt(grouped_errs, namespace) |?: [])
           @ [(err, range)],
         )
       | _ => (),
     );

  grouped_errs
  |> Hashtbl.to_seq
  |> List.of_seq
  |> List.iter(((namespace, errs)) =>
       errs
       |> notification(namespace |> Namespace.to_path(source_dir))
       |> server.notify(method_key)
     );
};
