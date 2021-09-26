open Kore;
open Deserialize;
open Yojson.Basic.Util;

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

let notification = (uri: string, errs) =>
  `Assoc([
    ("uri", `String(uri)),
    (
      "diagnostics",
      `List(
        errs
        |> List.map(((err, range)) =>
             `Assoc([
               ("range", Response.range(range)),
               ("severity", `Int(severity(Error))),
               ("source", `String("knot")),
               ("message", `String(err |> ~@Knot.Error.pp_parse_err)),
             ])
           ),
      ),
    ),
  ])
  |> Response.wrap_notification("textDocument/publishDiagnostics");

let report = (source_dir: string, errs) => {
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
  |> Seq.iter(((namespace, errs)) =>
       errs
       |> notification(namespace |> Namespace.to_path(source_dir))
       |> Protocol.notify
     );
};
