module FileStream = Knot.FileStream;
module TokenStream = KnotLex.TokenStream;
module Parser = KnotParse.Parser;
module Analyzer = KnotAnalyze.Analyzer;
module Scope = KnotAnalyze.Scope;

exception InvalidProgram;
exception ParsingFailed;

type loaded_module = {
  ast: Knot.AST.module_,
  deps: list(string),
};

let buffer_size = 1000;

let cache_as_tmp = file =>
  Filename.open_temp_file(
    ~mode=[Open_wronly, Open_binary],
    "knot-cached-module-",
    "-maybe-put-a-build-hash-here",
  )
  |> (
    ((tmp_file, tmp_channel)) => {
      let in_channel = open_in_bin(file);
      let channel_length = in_channel_length(in_channel);

      let rec write = () => {
        let remaining = channel_length - pos_in(in_channel);
        let buf_length = min(buffer_size, remaining);
        let buf = Bytes.create(buf_length);

        let read = input(in_channel, buf, 0, buf_length);
        output(tmp_channel, buf, 0, read);

        if (remaining !== 0) {
          write();
        };
      };

      write();

      flush(tmp_channel);

      close_out(tmp_channel);
      close_in(in_channel);

      open_in(tmp_file);
    }
  );

let load = (~module_tbl=Hashtbl.create(24), file) => {
  Printf.printf("loading %s\n", file);

  let in_channel = cache_as_tmp(file);

  FileStream.of_channel(in_channel)
  |> TokenStream.of_file_stream(~filter=TokenStream.filter_comments)
  |> Parser.parse(Parser.prog)
  |> (
    fun
    | Some(_) as res => res
    | None => raise(ParsingFailed)
  )
  |> Analyzer.analyze(
       ~scope=
         Scope.create(
           ~label=Printf.sprintf("module(%s)", file),
           ~module_tbl,
           (),
         ),
       (),
     )
  |> (
    fun
    | Some(ast) => {
        close_in(in_channel);
        {ast, deps: Analyzer.analyze_dependencies(ast)};
      }
    | None => raise(InvalidProgram)
  );
};
