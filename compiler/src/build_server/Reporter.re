open Kore;

module Resolver = Resolve.Resolver;

let method_key = "compiler/error";

let __arrow_sep = Fmt.Sep.(of_sep(~trail=Trail.nop, " ->"));

let report = (server: JSONRPC.Server.t, errors: list(compile_err)) => {
  let errors' =
    errors
    |> List.filter_map(
         fun
         | FileNotFound(file) =>
           `Assoc([
             ("type", `String("file_not_found")),
             ("message", `String(file |> Fmt.str("cannot find file '%s'"))),
           ])
           |> Option.some

         | UnresolvedModule(module_) =>
           `Assoc([
             ("type", `String("unresolved_module")),
             (
               "message",
               `String(module_ |> Fmt.str("cannot resolve module '%s'")),
             ),
           ])
           |> Option.some

         | InvalidModule(namespace) =>
           `Assoc([
             ("type", `String("invalid_module")),
             (
               "message",
               `String(
                 namespace
                 |> Fmt.str(
                      "module '%a' did not contain valid source code",
                      Namespace.pp,
                    ),
               ),
             ),
           ])
           |> Option.some

         /* should never report an import cycle with fewer than 1 modules involved */
         | ImportCycle([]) => raise(SystemError)

         | ImportCycle([module_]) =>
           `Assoc([
             ("type", `String("import_cycle")),
             (
               "message",
               `String(module_ |> Fmt.str("module '%s' imports itself")),
             ),
           ])
           |> Option.some

         | ImportCycle([first, ..._] as cycle) =>
           `Assoc([
             ("type", `String("import_cycle")),
             (
               "message",
               `String(
                 cycle
                 @ [first]
                 |> Fmt.(
                      str(
                        "found module import cycle between %a",
                        list(~sep=__arrow_sep, string),
                      )
                    ),
               ),
             ),
           ])
           |> Option.some

         | _ => None,
       );

  if (!List.is_empty(errors')) {
    `Assoc([("errors", `List(errors'))]) |> server.notify(method_key);
  };
};
