open Kore;

module Compiler = Compile.Compiler;

let __file_schema = "file://";

let _reporter = errs =>
  Protocol.send(
    Response.show_message(
      errs
      |> Knot.Error.print_errs
      |> Print.fmt("compilation failed with errors:\n%s"),
      Error,
    ),
  );

let start = (find_config: string => Config.t) => {
  let compilers = Hashtbl.create(1);

  Protocol.subscribe(
    stdin,
    Event.(
      fun
      | Initialize(x) => {
          /* TODO: handle the case where workspace folders are nested? */
          x.params.workspace_folders
          |?: []
          |> List.iter(({uri, name}) => {
               let path = String.drop_prefix(__file_schema, uri);
               Log.info(
                 "creating compiler for '%s' project (%s)",
                 name,
                 path,
               );

               let config = find_config(path);
               let root_dir =
                 (
                   Filename.is_relative(config.root_dir)
                     ? Filename.concat(path, config.root_dir)
                     : config.root_dir
                 )
                 |> Filename.resolve;

               Log.info("root dir: %s", root_dir);

               let compiler =
                 Compiler.create(
                   ~report=_ => _reporter,
                   {
                     root_dir,
                     source_dir: config.source_dir,
                     name,
                     fail_fast: false,
                   },
                 );

               Hashtbl.add(compilers, name, compiler);
             });
        }

      | Hover(_) => Log.info("hover")
      | FileOpen({params: {text_document: {uri}}}) => {
          let path = String.drop_prefix(__file_schema, uri);
          Log.info("opened file: %s", path);

          compilers
          |> Hashtbl.to_seq_values
          |> List.of_seq
          |> List.find_opt(compiler =>
               String.starts_with(Compiler.(compiler.config.root_dir), path)
             )
          |> (
            fun
            | Some(compiler) => {
                let source_dir =
                  Filename.concat(
                    compiler.config.root_dir,
                    compiler.config.source_dir,
                  );
                let relative =
                  path
                  |> Filename.relative_to(source_dir)
                  |> String.drop_suffix(Constants.file_extension)
                  |> (++)(Constants.root_dir);

                Log.info("relative path: %s", relative);

                compiler
                |> Compiler.upsert_module(
                     relative |> Reference.Namespace.of_string,
                   );
              }
            | None =>
              Log.warn(
                "event for file %s ignored as it doesn't belong to any active projects",
                uri,
              )
          );
        }
      | FileClose(x) => Log.info("closed file %s", x.params.text_document.uri)
      | GoToDefinition(x) =>
        Log.info("go to definition %s", x.params.text_document.uri)
    ),
  );

  Lwt.return();
};
